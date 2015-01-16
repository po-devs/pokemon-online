#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtOpenGL/QGLWidget>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"
#include "moveinfoaccessor.h"
#include "themeaccessor.h"
#include "proxydatacontainer.h"
#include "defaulttheme.h"
#include <Utilities/functions.h>

BattleScene::BattleScene(battledata_ptr dat, BattleDefaultTheme *theme, QVariantMap options)
    : mData(dat), mOwnProxy(new BattleSceneProxy(this)), peeking(false), inmove(false), pauseCount(0), mOptions(options)
{
    activelyReplaying = false;

    QSettings s;
    mNewSprites = s.value("Battle/NewSprites", true).toBool();

    qmlRegisterType<ProxyDataContainer>("pokemononline.battlemanager.proxies", 1, 0, "BattleData");
    qmlRegisterType<TeamProxy>("pokemononline.battlemanager.proxies", 1, 0, "TeamData");
    qmlRegisterType<PokeProxy>("pokemononline.battlemanager.proxies", 1, 0, "PokeData");
    qmlRegisterType<MoveProxy>("pokemononline.battlemanager.proxies", 1, 0, "MoveData");
    qmlRegisterType<PokemonInfoAccessor>();
    qmlRegisterType<BattleSceneProxy>();
    qmlRegisterType<MoveInfoAccessor>("pokemononline.battlemanager.proxies", 1, 0, "MoveInfo");
    qmlRegisterType<BattleDefaultTheme>("pokemononline.battlemanager.proxies", 1, 0, "Theme");
    qmlRegisterType<AuxPokeDataProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldPokeData");
    qmlRegisterType<FieldProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldData");
    qmlRegisterType<ZoneProxy>("pokemononline.battlemanager.proxies", 1, 0, "ZoneData");
    qmlRegisterType<BattleScene>("pokemononline.battlemanager.proxies", 1, 0, "BattleScene");

    /* Tells QML not to delete our pokeproxy and teamproxy objects...

      See http://apidocs.meego.com/1.1/core/html/qt4/qdeclarativeengine.html#objectOwnership */
    ProxyDataContainer *data_ptr = getDataProxy();
    for (int i = 0; i < 2; i++) {
        QDeclarativeEngine::setObjectOwnership(data_ptr->team(i), QDeclarativeEngine::CppOwnership);
        for (int j = 0; j < 6; j++) {
            QDeclarativeEngine::setObjectOwnership(data_ptr->team(i)->poke(j), QDeclarativeEngine::CppOwnership);
        }
        for (int j = 0; j < dat->numberOfSlots()/2; j++) {
            QDeclarativeEngine::setObjectOwnership(data_ptr->field()->poke(j*2+i), QDeclarativeEngine::CppOwnership);
        }
    }
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);

    // Set optimizations not already done in QDeclarativeView
    mWidget->setAttribute(Qt::WA_OpaquePaintEvent);
    mWidget->setAttribute(Qt::WA_NoSystemBackground);

    // Using OpenGL for the viewport causes horrible lag on Mac
    
#ifndef Q_OS_MACX
    // Make QDeclarativeView use OpenGL backend
    QGLWidget *glWidget = new QGLWidget(mWidget);
    mWidget->setViewport(glWidget);
    mWidget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#endif

    mWidget->engine()->rootContext()->setContextProperty("battle", mOwnProxy);
    mWidget->engine()->addImageProvider("pokeinfo", new PokemonInfoAccessor());
    mWidget->engine()->addImageProvider("themeinfo", new ThemeAccessor(theme));
    mWidget->engine()->rootContext()->setContextProperty("moveInfo", new MoveInfoAccessor(this, data()->gen()));
    mWidget->engine()->rootContext()->setContextProperty("theme", theme);
    mWidget->setSource(QString("qml/initial.qml"));
}

QVariant BattleScene::option(const QString &opt, const QVariant &def) const
{
    return mOptions.value(opt,def);
}

void BattleScene::log(const QString &mess)
{
    /* Bolded messages are clause / turn announcements, ... */
    if (mess.contains(QRegExp("<span class=\"[A-Za-z]+\"><b><span")) || mess.contains("class=\"PlayerChat\"") || mess.contains("class=\"SpectatorChat\"")
             || mess.contains("class=\"Spectating\"")) {
        return;
    }
    /* The battle window log text is white, no exceptions */
    QRegExp r("color:#[0-9a-f]{6}");

    QString rep = mess;
    rep.remove(r);

    if (mess.contains("class=\"Space\"")) {
        emit battleLog("");
    } else {
        emit battleLog(rep);
    }
}

void BattleScene::launch() {
    emit launched();
}

bool BattleScene::newSprites()
{
    // return value set at the beginning to prevent switching between the two mid-battle, as the scene doesn't handle that well //
    return mNewSprites;
}

BattleScene::~BattleScene()
{
    delete mOwnProxy;
}

bool BattleScene::reversed()
{
    return data()->role(1) == BattleConfiguration::Player;
}

int BattleScene::width() const
{
    return mWidget->width();
}

int BattleScene::height() const
{
    return mWidget->height();
}

BattleScene::battledata_ptr BattleScene::data()
{
    return mData;
}

QDeclarativeView *BattleScene::getWidget()
{
    mWidget->setFixedSize(mWidget->sizeHint());
    return mWidget;
}

ProxyDataContainer * BattleScene::getDataProxy()
{
    return data()->exposedData();
}

void BattleScene::debug(const QString &m)
{
    qDebug() << m;
    emit printMessage(m+"\n");
}

void BattleScene::pause(int ticks)
{
    pauseCount += ticks;
    baseClass::pause(ticks);
}

void BattleScene::unpause(int ticks)
{
    pauseCount -= ticks;

    useCommands();

    if (playingCommands()) {
        baseClass::pause(-ticks);
    } else {
        baseClass::unpause(ticks);
    }
}

void BattleScene::useCommands()
{
    if (!playingCommands()) {
        activelyReplaying = true;
        while (!isPaused() && commands.size() > 0) {
            useCommand();
        }
        activelyReplaying = false;
    }
}

void BattleScene::replayCommands()
{
    misReplayingCommands = true;

    useCommands();
    baseClass::unpause(0);
}

void BattleScene::useCommand()
{
    if (commands.size() > 0) {
        AbstractCommand *command = *commands.begin();

        int val = command->val();

        if (!onPeek(val)) {
            return;
        }

        commands.pop_front();
        command->apply();
        delete command;

        if (replayCount > 0 && misReplayingCommands) {
            replayCount --;

            if (replayCount == 0) {
                misReplayingCommands = false;
            }
        }
    }
}

bool BattleScene::shouldStartPeeking(param<BattleEnum::StatChange> p, int spot, int stat, int boost, bool silent)
{
    return shouldContinuePeeking(p, spot, stat, boost, silent);
}

bool BattleScene::shouldContinuePeeking(param<BattleEnum::StatChange>, int, int, int boost, bool)
{
    /* Stacks all consecutive changes of the same sign into one boost, for the purpose of animations */
    QLinkedListIterator<int> it(info.statChanges);
    it.toBack();

    if (sign(info.statChanges.back()) == sign(boost)) {
        boost += info.statChanges.back();
        info.statChanges.back() = 0;
    }

    info.statChanges.push_back(boost);

    return true;
}

int BattleScene::statboostlevel()
{
    return abs(info.statChanges.front());
}

bool BattleScene::isPlayer(int spot)
{
    return data()->isPlayer(spot);
}

void BattleScene::onStatBoost(int, int, int, bool)
{
    info.statChanges.pop_front();
}

bool BattleScene::shouldStartPeeking(param<BattleEnum::UseAttack>, int, int attack, bool)
{
    /* Those three attacks require a choice from the player and so would hang the battle window
      until the player made the choice - and it's annoying. */
    if (attack == Move::U_turn || attack == Move::BatonPass || attack == Move::VoltSwitch) {
        return false;
    } else {
        info.reset();
        inmove = true;
        return true;
    }
}

void BattleScene::onUseAttack(int spot, int attack, bool)
{
    info.attack = attack;
    info.spot = spot;

    if (info.hits > 0) {
        info.moveData["currentHit"] = 0;
    }
    emit attackUsed(spot, attack, info.moveData);
}

void BattleScene::startPeeking()
{
    replayCount = 0;
    peeking = true;
}

void BattleScene::stopPeeking()
{
    inmove = false;
    peeking = false;
    replayCount = commands.size();
}

bool BattleScene::onPeek(int val)
{
    if (val == BattleEnum::Damaged && info.hits > 0) {
        if (info.blocked) {
            if (info.currentHit < info.hits) {
                info.currentHit++;
                info.moveData["currentHit"] = info.currentHit;
                emit hit(info.spot, info.attack, info.moveData);
            }
            info.blocked = false;
            return false;
        } else {
            info.blocked = true;
            return true;
        }
    }

    return true;
}

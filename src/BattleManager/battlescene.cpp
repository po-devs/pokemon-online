#include <QtDeclarative>
#include <QtOpenGL/QGLWidget>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"
#include "../Utilities/functions.h"

BattleScene::BattleScene(battledata_ptr dat) : mData(dat), mOwnProxy(new BattleSceneProxy(this)), peeking(false), inmove(false),
    pauseCount(0)
{
    qmlRegisterType<ProxyDataContainer>("pokemononline.battlemanager.proxies", 1, 0, "BattleData");
    qmlRegisterType<TeamProxy>("pokemononline.battlemanager.proxies", 1, 0, "TeamData");
    qmlRegisterType<PokeProxy>("pokemononline.battlemanager.proxies", 1, 0, "PokeData");
    qmlRegisterType<PokemonInfoAccessor>();
    qmlRegisterType<BattleSceneProxy>();
    qmlRegisterType<AuxPokeDataProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldPokeData");
    qmlRegisterType<FieldProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldData");
    qmlRegisterType<BattleScene>("pokemononline.battlemanager.proxies", 1, 0, "BattleScene");

    /* Tells QML not to delete our pokeproxy and teamproxy objects...

      See http://apidocs.meego.com/1.1/core/html/qt4/qdeclarativeengine.html#objectOwnership */
    ProxyDataContainer *data_ptr = getDataProxy();
    for (int i = 0; i < 2; i++) {
        QDeclarativeEngine::setObjectOwnership(data_ptr->team(i), QDeclarativeEngine::CppOwnership);
        for (int j = 0; j < 6; j++) {
            QDeclarativeEngine::setObjectOwnership(data_ptr->team(i)->poke(j), QDeclarativeEngine::CppOwnership);
        }
        for (int j = 0; j < 3; j++) {
            QDeclarativeEngine::setObjectOwnership(data_ptr->field()->poke(j*2+i), QDeclarativeEngine::CppOwnership);
        }
    }
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);

    // Set optimizations not already done in QDeclarativeView
    mWidget->setAttribute(Qt::WA_OpaquePaintEvent);
    mWidget->setAttribute(Qt::WA_NoSystemBackground);
    // Make QDeclarativeView use OpenGL backend
    QGLWidget *glWidget = new QGLWidget(mWidget);
    mWidget->setViewport(glWidget);
    mWidget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    mWidget->engine()->rootContext()->setContextProperty("battle", mOwnProxy);
    mWidget->engine()->addImageProvider("pokeinfo", new PokemonInfoAccessor());
    mWidget->setSource(QString("qml/initial.qml"));
}

void BattleScene::launch() {
    emit launched();
}

BattleScene::~BattleScene()
{
    delete mOwnProxy;
}

bool BattleScene::reversed()
{
    return data()->role(1) == BattleConfiguration::Player;
}

BattleScene::battledata_ptr BattleScene::data()
{
    return mData;
}

QDeclarativeView *BattleScene::getWidget()
{
    return mWidget;
}

ProxyDataContainer * BattleScene::getDataProxy()
{
    return data()->exposedData();
}

void BattleScene::debug(const QString &m)
{
    qDebug() << m;
    emit printMessage(m);
}

void BattleScene::pause()
{
    pauseCount =+ 1;
    baseClass::pause();
}

void BattleScene::unpause()
{
    pauseCount -= 1;

    while (!isPaused() && commands.size() > 0) {
        useCommand();
    }

    baseClass::unpause();
}

void BattleScene::useCommand()
{
    if (commands.size() > 0) {
        AbstractCommand *command = *commands.begin();
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

void BattleScene::onStatBoost(int, int, int, bool)
{
    info.statChanges.pop_front();
}

bool BattleScene::shouldStartPeeking(param<BattleEnum::UseAttack>, int, int)
{
    inmove = true;
    return true;
}

void BattleScene::onUseAttack(int spot, int attack)
{
    qDebug() << "move data length: " << info.moveData.count();
    emit attackUsed(spot, attack, info.moveData);
    info.moveData.clear();
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

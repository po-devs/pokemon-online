#include <QtDeclarative>
#include <QtOpenGL/QGLWidget>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"

BattleScene::BattleScene(battledata_ptr dat) : mData(dat), mOwnProxy(new BattleSceneProxy(this)), peeking(false),
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

    if (pauseCount == 0) {
        if (commands.size() > 0) {
            commands[0]->apply();
            delete commands[0];
            commands.erase(commands.begin(), commands.begin()+1);
        }
    }

    baseClass::unpause();
}

bool BattleScene::shouldContinuePeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent)  {
    (void) stat;
    (void) silent;

    if (info.lastSlot == spot && ((info.lastStatChange == StatUp) == (boost > 0) )) {
        return true;
    }
    return false;
}

bool BattleScene::shouldStartPeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent)  {
    (void) stat;
    (void) silent;

    info.lastStatChange = boost > 0 ? StatUp : StatDown;
    info.lastSlot = spot;

    return true;
}

void BattleScene::onUseAttack(int spot, int attack) {
    emit attackUsed(spot, attack);
}

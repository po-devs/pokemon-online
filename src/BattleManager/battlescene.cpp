#include <QtDeclarative>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"

BattleScene::BattleScene(battledata_ptr dat) : mData(dat), mOwnProxy(new BattleSceneProxy(this))
{
    qmlRegisterType<ProxyDataContainer>("pokemononline.battlemanager.proxies", 1, 0, "BattleData");
    qmlRegisterType<TeamProxy>("pokemononline.battlemanager.proxies", 1, 0, "TeamData");
    qmlRegisterType<PokeProxy>("pokemononline.battlemanager.proxies", 1, 0, "PokeData");
    qmlRegisterType<PokemonInfoAccessor>();
    qmlRegisterType<BattleSceneProxy>();
    qmlRegisterType<AuxPokeDataProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldPokeData");
    qmlRegisterType<FieldProxy>("pokemononline.battlemanager.proxies", 1, 0, "FieldData");
    qmlRegisterType<BattleScene>();

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
    mWidget->engine()->rootContext()->setContextProperty("battle", mOwnProxy);
    mWidget->engine()->addImageProvider("pokeinfo", new PokemonInfoAccessor());
    mWidget->setSource(QString("qml/battlescene.qml"));
}

BattleScene::~BattleScene()
{
    delete mOwnProxy;
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

void BattleScene::pause() {
    BattleCommandManager<BattleScene>::pause();
}

void BattleScene::unpause() {
    BattleCommandManager<BattleScene>::unpause();
}

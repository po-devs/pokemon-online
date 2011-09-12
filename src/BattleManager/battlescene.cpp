#include <QtDeclarative>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"

BattleScene::BattleScene(BattleData *dat) : mData(dat), proxy(new BattleDataProxy(dat)), mOwnProxy(new BattleSceneProxy(this))
{
    qmlRegisterType<BattleDataProxy>("pokemononline.battlemanager.proxies", 1, 0, "BattleData");
    qmlRegisterType<TeamProxy>("pokemononline.battlemanager.proxies", 1, 0, "TeamData");
    qmlRegisterType<PokeProxy>("pokemononline.battlemanager.proxies", 1, 0, "PokeData");

    /* Tells QML not to delete our pokeproxy and teamproxy objects...

      The dataproxy is a property of the CPP object, not something returned by
      a CPP function inside QML, so it's safe, but we never know for the future.

      The calls on team() and team()->poke() are absolutely needed though.

      See http://apidocs.meego.com/1.1/core/html/qt4/qdeclarativeengine.html#objectOwnership */
    QDeclarativeEngine::setObjectOwnership(proxy, QDeclarativeEngine::CppOwnership);
    for (int i = 0; i < 2; i++) {
        QDeclarativeEngine::setObjectOwnership(proxy->team(i), QDeclarativeEngine::CppOwnership);
        for (int j = 0; j < 6; j++) {
            QDeclarativeEngine::setObjectOwnership(proxy->team(i)->poke(j), QDeclarativeEngine::CppOwnership);
        }
    }

    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);
    mWidget->engine()->rootContext()->setContextProperty("battle", mOwnProxy);
    //mWidget->engine()->rootContext()->setContextProperty("info", PokemonInfoAccessor::getInstance());
    //mWidget->engine()->rootContext()->setContextProperty("data", proxy);
    mWidget->engine()->addImageProvider("pokeinfo", PokemonInfoAccessor::getInstance());
    mWidget->setSource(QString("qrc:battlescene.qml"));
}

BattleScene::~BattleScene()
{
    delete mOwnProxy;
    delete proxy;
}

BattleData * BattleScene::data()
{
    return mData;
}

QDeclarativeView *BattleScene::getWidget()
{
    return mWidget;
}

BattleDataProxy * BattleScene::getDataProxy()
{
    return proxy;
}

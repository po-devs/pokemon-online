#include <QtDeclarative>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"
#include "pokemoninfoaccessor.h"

BattleScene::BattleScene(BattleData *dat) : mData(dat), proxy(new BattleDataProxy(dat)), mOwnProxy(new BattleSceneProxy(this))
{
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);
    mWidget->engine()->rootContext()->setContextObject(mOwnProxy);
    mWidget->engine()->rootContext()->setContextProperty("info", PokemonInfoAccessor::getInstance());
    mWidget->engine()->rootContext()->setContextProperty("data", proxy);
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

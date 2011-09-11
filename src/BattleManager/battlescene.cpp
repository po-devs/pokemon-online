#include <QtDeclarative>

#include "battlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "battlesceneproxy.h"

BattleScene::BattleScene(BattleData *dat) : mData(dat), proxy(new BattleDataProxy(dat)), mOwnProxy(new BattleSceneProxy(this))
{
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);
    mWidget->engine()->rootContext()->setContextObject(proxy);
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

BattleDataProxy *BattleScene::getProxy()
{
    return proxy;
}

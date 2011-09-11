#include <QtDeclarative/QDeclarativeView>

#include "battlescene.h"
#include "battledata.h"

BattleScene::BattleScene(BattleData *dat) : mData(dat)
{
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);
    mWidget->setSource(QString("qrc:battlescene.qml"));
}

BattleScene::~BattleScene()
{
}

BattleData * BattleScene::data()
{
    return mData;
}

QDeclarativeView *BattleScene::getWidget()
{
    return mWidget;
}

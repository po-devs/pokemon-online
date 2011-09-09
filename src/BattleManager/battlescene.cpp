#include <QtDeclarative/QDeclarativeView>

#include "battlescene.h"
#include "battledata.h"

BattleScene::BattleScene(BattleData *dat) : mData(dat)
{
    mWidget = new QDeclarativeView();
    mWidget->setAttribute(Qt::WA_DeleteOnClose);
}

BattleScene::~BattleScene()
{
    /* Normally, somebody should have taken ownership of the widget,
      but if not, closing & deleting it */
    if (mWidget->parent() == NULL) {
        mWidget->close();
    }
}

BattleData * BattleScene::data()
{
    return mData;
}

QDeclarativeView *BattleScene::getWidget()
{
    return mWidget;
}

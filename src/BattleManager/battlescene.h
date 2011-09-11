#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include <QObject>
#include "battlecommandmanager.h"

class BattleData;
class QDeclarativeView;
class BattleDataProxy;
class BattleSceneProxy;

class BattleScene: public BattleCommandManager<BattleScene>
{
public:
    BattleScene(BattleData *data);
    ~BattleScene();

    QDeclarativeView *getWidget();
    BattleDataProxy *getProxy();
private:
    BattleData *mData;
    BattleData *data();

    BattleDataProxy *proxy;
    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;
};


#endif // BATTLESCENE_H

#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battlecommandmanager.h"

class BattleData;
class QDeclarativeView;

class BattleScene: public BattleCommandManager<BattleScene>
{
public:
    BattleScene(BattleData *data);
    ~BattleScene();

    QDeclarativeView *getWidget();
private:
    BattleData *mData;
    BattleData *data();

    QDeclarativeView *mWidget;
};

#endif // BATTLESCENE_H

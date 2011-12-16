#ifndef REGULARBATTLESCENE_H
#define REGULARBATTLESCENE_H

/*
 * Non-QML battle scene
 */

#include "battlesceneflow.h"
#include "battlecommandmanager.h"
#include "advancedbattledata.h"

class BattleSceneProxy;
class ProxyDataContainer;

class RegularBattleScene: public QObject, public BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> >
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;
    typedef BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> > baseClass;

    RegularBattleScene(battledata_ptr data=0);
    ~RegularBattleScene();

    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)

    /* Should the players be reversed positions in the visual scene? */
    bool reversed();
    void launch();

    template <enumClass val, typename... Params>
    bool shouldStartPeeking(param<val>, Params...) {
        return false;
    }

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return false;
    }
    void onUseAttack(int spot, int attack);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking() { peeking = true; }
    void stopPeeking() { peeking = false; }
signals:
    void printMessage(const QString&);
    void launched();
    void attackUsed(int spot, int attack);
private:
    battledata_ptr mData;
    battledata_ptr data();

    bool peeking;
    int pauseCount;
};

#endif // REGULARBATTLESCENE_H

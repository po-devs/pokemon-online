#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battlesceneflow.h"
#include "battlecommandmanager.h"
#include "advancedbattledata.h"

class QDeclarativeView;
class BattleSceneProxy;
class ProxyDataContainer;

class BattleScene: public QObject, public BattleCommandManager<BattleScene, BattleSceneFlow<BattleEnum, BattleScene> >
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;
    typedef BattleCommandManager<BattleScene, BattleSceneFlow<BattleEnum, BattleScene> > baseClass;

    BattleScene(battledata_ptr data=0);
    ~BattleScene();

    enum StatDirection {
        StatDown = -1,
        NoStat = 0,
        StatUp = 1
    };

    QDeclarativeView *getWidget();
    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

    /* Prints debug text in the console and battle log */
    Q_INVOKABLE void debug(const QString&m);

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)

    /* Should the players be reversed positions in the visual scene? */
    bool reversed();
    void launch();

    template <enumClass val, typename... Params>
    bool shouldStartPeeking(param<val>, Params...) {
        return false;
    }

    bool shouldStartPeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent);

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return false;
    }

    bool shouldContinuePeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent);

    void onUseAttack(int spot, int attack);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking() { peeking = true; }
    void stopPeeking() { peeking = false; }
signals:
    void printMessage(const QString&);
    void launched();
    void playCry(int);
    void attackUsed(int spot, int attack);
private:
    battledata_ptr mData;
    battledata_ptr data();

    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;

    struct BattleSceneInfo {
        StatDirection lastStatChange;
        int lastSlot;

        BattleSceneInfo() {
            lastSlot = -1;
            lastStatChange = NoStat;
        }
    };

    BattleSceneInfo info;
    bool peeking;
    int pauseCount;
};


#endif // BATTLESCENE_H

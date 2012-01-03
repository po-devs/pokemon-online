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

    QDeclarativeView *getWidget();
    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause(int ticks=1);
    Q_INVOKABLE void unpause(int ticks=1);

    /* Prints debug text in the console and battle log */
    Q_INVOKABLE void debug(const QString&m);

    Q_INVOKABLE int statboostlevel();

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)

    /* Should the players be reversed positions in the visual scene? */
    bool reversed();
    void launch();

    template <enumClass val, typename... Params>
    bool shouldStartPeeking(param<val>, Params...) {
        return false;
    }

    bool shouldStartPeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent);
    bool shouldStartPeeking(param<BattleEnum::UseAttack>, int, int, bool);

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return true;
    }

    bool shouldContinuePeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent);
    bool shouldContinuePeeking(param<BattleEnum::BlankMessage>) {return false;}
    /* When using u-turn or baton pass, it stops at the middle. Dynamic Stats is always sent to both players though, so it unhangs both */
    bool shouldContinuePeeking(param<BattleEnum::DynamicStats>, int , std::shared_ptr<BattleStats>*) { return false;}
    /* When sending a backup with intimidate, this happens before any blank message */
    bool shouldContinuePeeking(param<BattleEnum::Turn>, int) {return false;}
    bool shouldContinuePeeking(param<BattleEnum::Hits>, int, int hits) {info.hits = hits; info.blocked = true; info.moveData.insert("hits", hits); return true;}

    void onUseAttack(int spot, int attack, bool silent);
    void onStatBoost(int spot, int stat, int boost, bool silent);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking();
    void stopPeeking();
    void useCommand();
    void useCommands();

    bool onPeek(int val);
    bool playingCommands() const { return activelyReplaying;}

    /* Replays the commands stored and delete them. */
    void replayCommands() ;
signals:
    void printMessage(const QString&);
    void launched();
    void playCry(int);
    void attackUsed(int spot, int attack, QVariantMap params);
    void hit(int spot, int attack, QVariantMap params);
private:
    battledata_ptr mData;
    battledata_ptr data();

    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;

    struct BattleSceneInfo {
        QLinkedList<int> statChanges;
        QVector<AbstractCommand*> attacks;
        QVariantMap moveData;
        int hits;
        int currentHit;
        bool blocked;

        int attack;
        int spot;

        void reset() {attacks.clear(); moveData.clear(); hits = 0; blocked = false; currentHit = 0;}

        BattleSceneInfo() {statChanges.push_back(0); reset();}
    };

    BattleSceneInfo info;
    bool peeking;
    bool inmove;
    int pauseCount;
    int replayCount;
    bool activelyReplaying;
};


#endif // BATTLESCENE_H

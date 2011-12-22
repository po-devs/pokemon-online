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

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

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
    bool shouldStartPeeking(param<BattleEnum::UseAttack>, int, int);

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return true;
    }

    bool shouldContinuePeeking(param<BattleEnum::StatChange>, int spot, int stat, int boost, bool silent);
    bool shouldContinuePeeking(param<BattleEnum::BlankMessage>) {return false;}
    bool shouldContinuePeeking(param<BattleEnum::Hits>, int, int hits) {info.moveData.insert("hits", hits); return true;}

    void onUseAttack(int spot, int attack);
    void onStatBoost(int spot, int stat, int boost, bool silent);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking();
    void stopPeeking();
    void useCommand();

    /* Replays the commands stored and delete them. */
    void replayCommands() {
        misReplayingCommands = true;

        while (!isPaused() && commands.size() > 0) {
            useCommand();
        }
    }
signals:
    void printMessage(const QString&);
    void launched();
    void playCry(int);
    void attackUsed(int spot, int attack, QVariantMap params);
private:
    battledata_ptr mData;
    battledata_ptr data();

    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;

    struct BattleSceneInfo {
        QLinkedList<int> statChanges;
        QVariantMap moveData;

        BattleSceneInfo() {statChanges.push_back(0);}
    };

    BattleSceneInfo info;
    bool peeking;
    bool inmove;
    int pauseCount;
    int replayCount;
};


#endif // BATTLESCENE_H

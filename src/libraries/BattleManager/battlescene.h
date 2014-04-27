#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battlesceneflow.h"
#include "battlecommandmanager.h"
#include "advancedbattledata.h"

class QDeclarativeView;
class BattleSceneProxy;
class ProxyDataContainer;
class BattleDefaultTheme;

class BattleScene: public QObject, public BattleCommandManager<BattleScene, BattleSceneFlow<BattleEnum, BattleScene> >
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;
    typedef BattleCommandManager<BattleScene, BattleSceneFlow<BattleEnum, BattleScene> > baseClass;

    BattleScene(battledata_ptr data=0, BattleDefaultTheme *theme=0, QVariantMap options = QVariantMap());
    ~BattleScene();

    QDeclarativeView *getWidget();
    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause(int ticks=1);
    Q_INVOKABLE void unpause(int ticks=1);

    /* Prints debug text in the console and battle log */
    Q_INVOKABLE void debug(const QString&m);

    Q_INVOKABLE int statboostlevel();
    Q_INVOKABLE bool isPlayer(int spot);

    Q_INVOKABLE QVariant option(const QString &opt, const QVariant &def=QVariant()) const;

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)
    Q_PROPERTY(int width READ width() CONSTANT)
    Q_PROPERTY(int height READ height() CONSTANT)
    Q_PROPERTY(bool newSprites READ newSprites() CONSTANT)

    /* Should the players be reversed positions in the visual scene? */
    bool reversed();
    int width() const;
    int height() const;
    void launch();
    bool newSprites();

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
    bool shouldContinuePeeking(param<BattleEnum::BlankMessage>) {
        return false;
    }
    bool shouldContinuePeeking(param<BattleEnum::OfferChoice>, int __attribute__((unused)) player, std::shared_ptr<BattleChoices>__attribute__((unused)) *  choice) {
        return false;
    }
    bool shouldContinuePeeking(param<BattleEnum::ClockStart>, int __attribute__((unused)) player, int __attribute__((unused)) time) {
        return false;
    }
    bool shouldContinuePeeking(param<BattleEnum::ClockStop>, int __attribute__((unused)) player, int __attribute__((unused)) time) {
        return false;
    }
    bool shouldContinuePeeking(param<BattleEnum::ChoiceSelection>, int __attribute__((unused)) player) {
        return false;
    }
    bool shouldContinuePeeking(param<BattleEnum::ChoiceCancelled>, int __attribute__((unused)) player) {
        return false;
    }

    /* When using u-turn or baton pass, it stops at the middle. Dynamic Stats is always sent to both players though, so it unhangs both */
    bool shouldContinuePeeking(param<BattleEnum::DynamicStats>, int , std::shared_ptr<BattleStats>*) { return false;}
    /* When sending a backup with intimidate, this happens before any blank message */
    bool shouldContinuePeeking(param<BattleEnum::Turn>, int) {return false;}
    bool shouldContinuePeeking(param<BattleEnum::Hits>, int, int hits) {info.hits = hits; info.blocked = true; info.moveData.insert("hits", hits); return true;}

    void onUseAttack(int spot, int attack, bool silent);
    void onStatBoost(int spot, int stat, int boost, bool silent);
    void onContinueWeather(int __attribute__((unused)) weather) {
        emit weatherContinue();
    }

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking();
    void stopPeeking();
    void useCommand();
    void useCommands();

    bool onPeek(int val);
    bool playingCommands() const { return activelyReplaying;}

    /* Replays the commands stored and delete them. */
    void replayCommands();
public slots:
    void log(const QString&);
signals:
    void battleLog(const QString &logMessage);
    void printMessage(const QString&);
    void launched();
    void playCry(int);
    void attackUsed(int spot, int attack, QVariantMap params);
    void weatherContinue();
    void hit(int spot, int attack, QVariantMap params);

//    /** Necessary because Qt crashes if a loader element containing a ShaderEffectItem is
//      still active when the declarative view is deleted, so we need to warn the qml that
//      the view is going to be deleted
//      */
//    void appearing();
//    void disappearing();
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

    QVariantMap mOptions;
};


#endif // BATTLESCENE_H

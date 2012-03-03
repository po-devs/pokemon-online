#ifndef BATTLEBASE_H
#define BATTLEBASE_H

#include "battleinterface.h"

//#include <QtCore>
#include "../PokemonInfo/battlestructs.h"
//#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/mtrand.h"
#include "../Utilities/contextswitch.h"
//#include "../Utilities/coreclasses.h"
//#include "battleinterface.h"
#include "battlepluginstruct.h"
//#include "battlecounters.h"


class BattleBase : public ContextCallee, public BattleInterface
{
    Q_OBJECT

    PROPERTY(int, turn);
    PROPERTY(int , publicId);
    PROPERTY(bool, rated);
    PROPERTY(QString, tier);
    PROPERTY(int, attacker);
    PROPERTY(int, attacked);
    PROPERTY(int, numberOfSlots);
    PROPERTY(bool, blocked);
    PROPERTY(int, attackCount);
    PROPERTY(bool, rearrangeTime);
    PROPERTY(int, selfKoer);
    PROPERTY(int, repeatCount);
    PROPERTY(bool, heatOfAttack);
    PROPERTY(int, drawer);
    PROPERTY(int, forfeiter);

public:
    BattleBase();
    ~BattleBase();

    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();
protected:
    void onDestroy(); //call in the sub class destructor

    virtual void engageBattle();
    virtual void beginTurn() = 0;
    virtual void endTurn() = 0;
    virtual void initializeEndTurnFunctions() = 0;
public:
    const TeamBattle &pubteam(int id) const;
    /* returns 0 or 1, or -1 if that player is not involved */
    int spot(int id) const;
    /* The other player */
    int opponent(int player) const;
    int partner(int spot) const;
    QList<int> revs(int slot) const;
    QList<int> allRevs(int slot) const; //returns even koed opponents
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    int id(int spot) const;
    /* Return the configuration of the players (1 refer to that player, 0 to that one... */
    const BattleConfiguration &configuration() const;
    /* Returns the rating of the beginning of a battle, of a player */
    int rating(int spot) const;

    Pokemon::gen gen() const {return conf.gen;}
    int mode() const {return conf.mode;}
    quint32 clauses() const {return conf.clauses;}
    /*
    Below Player is either 1 or 0, aka the spot of the id.
    Use the functions above to make conversions
    */
    TeamBattle &team(int player);
    const TeamBattle &team(int player) const;
    PokeBattle &poke(int player);
    const PokeBattle &poke(int player) const;
    PokeBattle &poke(int player, int poke);
    const PokeBattle &poke(int player, int poke) const;
    bool koed(int player) const;
    bool isOut(int player, int poke);

    int player(int slot) const;
    /* Returns -1 if none */
    int randomOpponent(int slot) const;
    /* Returns a koed one if none */
    int randomValidOpponent(int slot) const;
    int slot(int player, int poke = 0) const;
    int slotNum(int slot) const;
    int countAlive(int player) const;
    int countBackUp(int player) const;
    bool canTarget(int attack, int attacker, int defender) const;
    bool areAdjacent(int attacker, int defender) const;
    /* Returns true or false if an attack is going on or not */
    bool attacking();
    bool multiples() const {
        return mode() != ChallengeInfo::Singles && mode() != ChallengeInfo::Rotation;
    }
    bool arePartners(int p1, int p2) const {
        return player(p1) == player(p2);
    }
    int numberPerSide() const {
        return numberOfSlots()/2;
    }

    bool sleepClause() const {
        return clauses() & ChallengeInfo::SleepClause;
    }

    virtual std::vector<int> &&sortedBySpeed();

    void notifyClause(int clause);
    void notifyMiss(bool multitar, int player, int target);
    void notifyKO(int player);

    void rearrangeTeams();

    int currentInternalId(int slot) const;

    virtual void changeStatus(int player, int status, bool tell = true, int turns = 0) = 0;

    /* Sends data to players */
    template <typename ...Params>
    void notify(int player, int command, int who, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), qint8(who), std::forward<Params>(params)...);

        emitCommand(who, player, tosend);
    }

    void emitCommand(int player, int players, const QByteArray &data);

    void battleChoiceReceived(int id, const BattleChoice &b);
    void battleChat(int id, const QString &str);
    void spectatingChat(int id, const QString &str);

    virtual int getStat(int poke, int stat) = 0;
    virtual void sendPoke(int player, int poke, bool silent = false) = 0;
protected slots:
    void clearSpectatorQueue();
signals:
    /* Due to threading issue, and the signal not being direct,
       The battle might already be deleted when the signal is received.

       So the parameter "publicId" is for the server to not to have to use
       sender(); */
    void battleInfo(int publicId, int id, const QByteArray &info);
    void battleFinished(int battleid, int result, int winner, int loser);
protected:
    QHash<int,QPair<int, QString> > spectators;
    mutable QMutex spectatorMutex;
    QList<QPointer<Player> > pendingSpectators;

    int spectatorKey(int id) const {
        return 10000 + id;
    }

    virtual void notifySituation(int dest) = 0;

    int ratings[2];
    /*.*/
    int myid[2];
    QString winMessage[2];
    QString loseMessage[2];

    /* timers */
    QAtomicInt timeleft[2];
    QAtomicInt startedAt[2];
    bool timeStopped[2];
    QBasicTimer *timer;

    /* What choice we allow the players to have */
    QList<BattleChoices> options;
    /* Is set to false once a player choses it move */
    QList<int> hasChoice;
    /* just indicates if the player could originally move or not */
    QList<bool> couldMove;

    void timerEvent(QTimerEvent *);

    void startClock(int player, bool broadCoast = true);
    void stopClock(int player, bool broadCoast = false);
    int timeLeft(int player);

    void yield();
    void schedule();

    /* if battle ends, stop the battle thread */
    void testWin();
    void endBattle(int result, int winner, int loser); //must always be called from the thread

    bool canCancel(int player);
    void cancel(int player);
    void addDraw(int player);

    bool validChoice(const BattleChoice &b);
    virtual void storeChoice(const BattleChoice &b);
    bool allChoicesOkForPlayer(int player);
    bool allChoicesSet();

    virtual BattleChoice &choice (int p) = 0;
public:
    const QHash<int, QPair<int, QString> > &getSpectators() const {
        QMutexLocker m(&spectatorMutex);
        return spectators;
    }

    const QList<QPointer<Player> > &getPendingSpectators() const {
        return pendingSpectators;
    }

    bool acceptSpectator(int id, bool authed=false) const;
    /* In case it's one of the battler, resends the current info to the battler */
    void addSpectator(Player *p);
    void removeSpectator(int id);

    /* Server tells a player forfeited */
    void playerForfeit(int forfeiterId);

    struct QuitException {};


    /* Generate a random number from 0 to max-1. Could be improved to use something better than modulo */
    unsigned randint(int max) const {
        return unsigned(rand_generator()) % max;
    }
    unsigned randint() const {
        return unsigned(rand_generator());
    }
    /* Return true with probability (heads_chance/total). Could be improved to use something better than modulo. */
    bool coinflip(unsigned heads_chance, unsigned total) const {
        return (unsigned(rand_generator()) % total) < heads_chance;
    }
protected:
    QList<BattlePlugin*> plugins;
    QList<BattlePStorage*> calls;

    void buildPlugins(PluginManager *p);
    void removePlugin(BattlePlugin *p);
    /* Calls a plugin function. the parameter is the enum of BattlePStorage
       corresponding to the function to call */
    void callp(int function);


    template<class T1, class T2, class T3>
    void callp(int function, T1 arg1, T2 arg2, T3 arg3) {
        //qDebug() << "Beginning callp for " << this;
        foreach(BattlePStorage *p, calls) {
            if (p->call(function, this, arg1, arg2, arg3) == -1)
                removePlugin(p->plugin);
        }
        //qDebug() << "Ending callp for " << this;
    }

    mutable MTRand_int32 rand_generator;

    BattleConfiguration conf;
public:
    /* Sleep clause necessity: only pokes asleep because of something else than rest are put there */
    // Public because used by Yawn
    int currentForcedSleepPoke[2];

    int weather;
    int weatherCount;

    void sendMoveMessage(int move, int part=0, int src=0, int type=0, int foe=-1, int other=-1, const QString &q="");
    void sendAbMessage(int move, int part=0, int src=0, int foe=-1, int type=0, int other=-1);
    void sendItemMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);
    void sendBerryMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);

    void notifyFail(int p);
};

#endif // BATTLEBASE_H

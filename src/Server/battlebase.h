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
#include <algorithm>

class BattleBase : public ContextCallee, public BattleInterface
{
    Q_OBJECT

    PROPERTY(int, turn);
    PROPERTY(int , publicId);
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

    typedef QVariantHash context;

    void init(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id, int nteam1, int nteam2, PluginManager *p);

    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();

    /* Starts the battle -- use the time before to connect signals / slots */
    void start(ContextSwitcher &ctx);
protected:
    void onDestroy(); //call in the sub class destructor

    virtual void engageBattle();
    virtual void beginTurn();
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
    bool rated() const;

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
    QHash<quint16, quint16> &items(int player);
    const QHash<quint16, quint16> &items(int player) const;
    bool koed(int player) const;
    bool isOut(int player, int poke) const;
    bool isOut(int poke) const;
    bool hasSubstitute(int player);
    int PP(int player, int slot) const;
    bool hasMove(int player, int move);
    int move(int player, int slot);
    bool hasMoved(int slot);
    Pokemon::uniqueId pokenum(int player);

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

    virtual std::vector<int> sortedBySpeed();

    void notifyClause(int clause);
    void notifyMiss(bool multitar, int player, int target);
    void notifyKO(int player);

    void rearrangeTeams();

    int currentInternalId(int slot) const;

    virtual void changeStatus(int player, int status, bool tell = true, int turns = 0) = 0;
    virtual void changeStatus(int team, int poke, int status);

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
    virtual void sendBack(int player, bool silent = false);
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

    virtual void notifySituation(int dest);

    int ratings[2];
    /*.*/
    int myid[2];
    QString winMessage[2];
    QString loseMessage[2];
    QString tieMessage[2];

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
    /* This time the pokelong effects */
    virtual void callpeffects(int source, int target, const QString &name) = 0;

    /* The players ordered by speed are stored there */
    std::vector<int> speedsVector;
    bool applyingMoveStatMods;

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

    void requestChoices();
    /* requests choice of action from the player */
    bool requestChoice(int player, bool acq = true /*private arg */, bool custom = false); /* return true if the pokemon has a choice to make (including switching & struggle)*/

    virtual BattleChoices createChoice(int slot) = 0;

    void setupChoices() ;
    virtual void setupMove(int i, int move) = 0;

    virtual void analyzeChoices() = 0;

    virtual void inflictSubDamage(int player, int damage, int source);
public:
    virtual void inflictDamage(int player, int damage, int source, bool straightattack=false, bool goForSub=false) = 0;
    virtual bool isMovePossible(int player, int slot);

    /* Sleep clause necessity: only pokes asleep because of something else than rest are put there */
    // Public because used by Yawn
    int currentForcedSleepPoke[2];

    int weather;
    int weatherCount;

    void setupLongWeather(int weather);
    void setupItems(int player, const QHash<quint16,quint16> &items);

    void sendMoveMessage(int move, int part=0, int src=0, int type=0, int foe=-1, int other=-1, const QString &q="");
    void sendAbMessage(int move, int part=0, int src=0, int foe=-1, int type=0, int other=-1);
    void sendItemMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);
    void sendBerryMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);

    void notifyFail(int p);
    void notifyChoices(int p);
    void notifyInfos(int tosend = All);
    void notifySub(int player, bool sub);

    void failSilently(int player);

    virtual BattleDynamicInfo constructInfo(int player);
    BattleStats constructStats(int player);

    struct BasicPokeInfo {
        Pokemon::uniqueId id;
        int weight;
        int type1;
        int type2;
        QVector<int> types;
        int ability;
        int level;
        quint32 flags;
        quint16 substituteLife;
        quint16 lastMoveUsed;
        quint16 lastMoveSlot;

        enum Flag {
            Transformed = 1,
            Substitute = 2,
            HadSubstitute = 4
        };

        int moves[4];
        quint8 pps[4];
        quint8 dvs[6];
        int stats[6];
        //The boost in HP and sdef are useless but avoids headaches
        int boosts[8];

        void init(const PokeBattle &p, Pokemon::gen gen);

        inline bool substitute() const {return flags & Substitute;}
        inline void remove(Flag f) {flags &= ~f;}
        inline void add(Flag f) {flags |= f;}
        inline bool is(Flag f) {return (flags & f) != 0;}
    };

    struct BasicMoveInfo {
        char critRaise;
        char repeatMin;
        char repeatMax;
        signed char priority;
        int flags;
        int power; /* unsigned char in the game, but can be raised by effects */
        int accuracy; /* Same */
        char type;
        char category; /* Physical/Special/Other */
        int rate; /* Same */
        char flinchRate;
        signed char recoil;
        int attack;
        char targets;
        signed char healing;
        char classification;
        char status;
        char statusKind;
        char minTurns;
        char maxTurns;
        quint32 statAffected;
        quint32 boostOfStat;
        quint32 rateOfStat;
        bool kingRock;

        void reset();
    };

    struct TurnMemory {
        TurnMemory() {
            reset();
        }

        void reset() {
            flags = 0;
            damageTaken = 0;
            typeMod = 0;
            stab = 0;
        }

        quint32 flags;
        quint16 damageTaken;
        int typeMod;
        quint8 stab;

        enum Flag {
            Incapacitated = 1,
            NoChoice = 2,
            HasMoved = 4,
            WasKoed = 8,
            Failed = 16,
            FailingMessage = 32,
            HasPassedStatus = 64,
            Flinched = 128,
            CriticalHit = 256,
            KeepAttack = 512, //For RBY
            UsePP = 1024, //For RBY
            BuildUp = 2048 //For RBY
        };

        inline void remove(Flag f) {flags &= ~f;}
        inline void add(Flag f) {flags |= f;}
        inline bool contains(Flag f) const {return (flags & f) != 0;}
        inline bool failed() const { return contains(Failed);}
        inline bool failingMessage() const { return contains(FailingMessage);}
    };

    virtual BasicPokeInfo &fpoke(int slot) = 0;
    virtual BasicPokeInfo const &fpoke(int slot) const = 0;
    virtual TurnMemory &turnMem(int slot) = 0;
    virtual const TurnMemory &turnMem(int slot) const = 0;
    virtual context &pokeMemory(int slot) = 0;
    virtual const context &pokeMemory(int slot) const = 0;
    virtual context &turnMemory(int slot) = 0;
    virtual const context &turnMemory(int slot) const = 0;
    virtual BasicMoveInfo &tmove(int slot) = 0;
    virtual const BasicMoveInfo &tmove(int slot) const = 0;

    ShallowBattlePoke opoke(int slot, int play, int i) const; /* aka 'opp poke', or what you need to know if it's your opponent's poke */

    virtual void inflictRecoil(int x, int target) = 0;
    void healLife(int player, int healing);
    virtual void changeHp(int player, int newHp);
    virtual void koPoke(int player, int source, bool straight=false);

    void changeSprite(int player, Pokemon::uniqueId newForme);

    bool wasKoed(int) const;

    virtual void requestSwitchIns();
    virtual void requestEndOfTurnSwitchIns();

    /* called just after requestChoice(s) */
    virtual void analyzeChoice(int player);

    /* Attack... */
    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
    represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
    virtual void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true) = 0;
    virtual bool testStatus(int player);

    void healStatus(int player, int status);
    bool isConfused(int player);
    void healConfused(int player);
    void inflictConfusedDamage(int player);

    virtual void losePP(int player, int move, int loss);
    virtual void changePP(int player, int move, int PP);
    virtual void changeTempMove(int player, int slot, int move);

    bool testFail(int player);
    virtual bool testAccuracy(int player, int target, bool silent = false) = 0;

    virtual PokeFraction getStatBoost(int player, int stat);
    virtual void calculateTypeModStab(int player=-1, int target=-1);
    virtual int repeatNum(int player);

    virtual void testCritical(int player, int target);
    virtual int calculateDamage(int player, int target) = 0;
    void healDamage(int player, int target);
    void notifyHits(int spot, int hits);
    void unthaw(int player);
    virtual void testFlinch(int player, int target);
    virtual void applyMoveStatMods(int player, int target);
    virtual void inflictConfused(int player, int attacker, bool tell);
    virtual void inflictStatus(int target, int status, int player, int minTurns, int maxTurns);
    virtual bool canGetStatus(int player, int status);
    virtual bool canSendPreventSMessage(int player, int attacker);
    bool hasType(int player, int type);
    virtual int getType(int player, int slot);
    virtual bool inflictStatMod(int player, int stat, int mod, int attacker, bool tell=true);

    virtual bool gainStatMod(int player, int stat, int bonus, int attacker, bool tell=true);
    /* Returns false if blocked */
    virtual bool loseStatMod(int player, int stat, int malus, int attacker, bool tell=true)=0;
    /* Does not do extra operations,just a setter */
    void changeStatMod(int player, int stat, int newstat);
};

#endif // BATTLEBASE_H

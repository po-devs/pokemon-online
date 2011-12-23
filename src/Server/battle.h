#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/mtrand.h"
#include "../Utilities/contextswitch.h"
#include "battleinterface.h"
#include "battlepluginstruct.h"
#include "battlecounters.h"

class Player;
class PluginManager;
class BattlePlugin;
class BattlePStorage;

/* Fixme: needs some sort of cache to avoid revs() creating a list
   each time */

class BattleSituation : public ContextCallee, public BattleInterface
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
public:
    typedef QVariantHash context;

    BattleSituation(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id, PluginManager *p);
    ~BattleSituation();

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

    bool acceptSpectator(int id, bool authed=false) const;
    void addSpectator(Player *p);

    bool sleepClause() const {
        return clauses() & ChallengeInfo::SleepClause;
    }

    void notifyClause(int clause);
    void notifyMiss(bool multitar, int player, int target);
    void notifyKO(int player);

    void removeSpectator(int id);

    int gen() const {return conf.gen;}
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
    bool wasKoed(int player) const;
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
    bool multiples() const {
        return mode() != ChallengeInfo::Singles && mode() != ChallengeInfo::Rotation;
    }
    bool arePartners(int p1, int p2) const {
        return player(p1) == player(p2);
    }
    int numberPerSide() const {
        return numberOfSlots()/2;
    }

    /* Starts the battle -- use the time before to connect signals / slots */
    void start(ContextSwitcher &ctx);
    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();
    /* requests choice of action from the player */
    bool requestChoice(int player, bool acq = true /*private arg */, bool custom = false); /* return true if the pokemon has a choice to make (including switching & struggle)*/
    void requestChoices(); /* request from both players */
    /* Shows what attacks are allowed or not */
    BattleChoices createChoice(int player);
    bool isMovePossible(int player, int slot);
    /* called just after requestChoice(s) */
    void analyzeChoice(int player);
    void analyzeChoices(); 
    std::vector<int> sortedBySpeed();

    /* Commands for the battle situation */
    void rearrangeTeams();
    void engageBattle();
    void beginTurn();
    void endTurn();
    void personalEndTurn(int source);
    void endTurnStatus(int player);
    void endTurnWeather();
    void endTurnDefrost();
    void callForth(int weather, int turns);
    void setupLongWeather(int weather);
    /* Attack... */
    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
	represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
    void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true);
    /* Returns true or false if an attack is going on or not */
    bool attacking();
    void makeTargetList(const QVector<int> &base);
    /* Does not do extra operations,just a setter */
    void changeHp(int player, int newHp);
    /* Sends a poke back to his pokeball (not koed) */
    void sendBack(int player, bool silent = false);
    void shiftSpots(int spot1, int spot2, bool silent = false);
    void notifyHits(int spot, int number);
    void sendPoke(int player, int poke, bool silent = false);
    void callEntryEffects(int player);
    void koPoke(int player, int source, bool straightattack = false);
    /* Does not do extra operations,just a setter */
    void changeStatMod(int player, int stat, int newstatmod);
    void changeForme(int player, int poke, const Pokemon::uniqueId &forme);
    void changePokeForme(int slot, const Pokemon::uniqueId &forme);
    void calculateTypeModStab(int orPlayer = -1, int orTarget = -1);
    void changeAForme(int player, int newforme);
    bool hasMinimalStatMod(int player, int stat);
    bool hasMaximalStatMod(int player, int stat);
    bool inflictStatMod(int player, int stat, int mod, int attacker, bool tell = true, bool *negative = NULL);
private:
    bool gainStatMod(int player, int stat, int bonus, int attacker, bool tell=true);
    /* Returns false if blocked */
    bool loseStatMod(int player, int stat, int malus, int attacker, bool tell=true);
public:
    bool canSendPreventMessage(int defender, int attacker);
    bool canSendPreventSMessage(int defender, int attacker);
    void preventStatMod(int player, int attacker);
    /* Does not do extra operations,just a setter */
    void changeStatus(int player, int status, bool tell = true, int turns = 0);
    void changeStatus(int team, int poke, int status);
    void unthaw(int player);
    void healStatus(int player, int status);
    void healConfused(int player);
    void healLife(int player, int healing);
    void healDamage(int player, int target);
    bool canGetStatus(int player, int status);
    void inflictStatus(int player, int Status, int inflicter, int minturns = 0, int maxturns = 0);
    bool isConfused(int player);
    void inflictConfused(int player, int source, bool tell=true);
    void inflictConfusedDamage(int player);
    void inflictRecoil(int source, int target);
    void inflictDamage(int player, int damage, int source, bool straightattack = false, bool goForSub = false);
    void inflictPercentDamage(int player, int percent, int source, bool straightattack = false);
    void inflictSubDamage(int player, int damage, int source);
    void disposeItem(int player);
    void eatBerry(int player, bool show=true);
    /* Eats a berry, not caring about the item the pokemon has, etc. */
    void devourBerry(int player, int berry, int target);
    void acqItem(int player, int item);
    void loseItem(int player, bool real = true);
    void loseAbility(int player);
    /* Removes PP.. */
    void changePP(int player, int move, int PP);
    void losePP(int player, int move, int loss);
    void gainPP(int player, int move, int gain);
    //Uproarer
    bool isThereUproar();
    void addUproarer(int player);
    void removeUproarer(int player);
    //Change turn order, use it carefully when you know those are correct values
    void makePokemonNext(int player);
    void makePokemonLast(int player);

    int calculateDamage(int player, int target);
    void applyMoveStatMods(int player, int target);
    bool testAccuracy(int player, int target, bool silent = false);
    void testCritical(int player, int target);
    void testFlinch(int player, int target);
    bool testStatus(int player);
    bool testFail(int player);
    void failSilently(int player);
    void fail(int player, int move, int part=0, int type=0, int trueSource = -1);
    bool hasType(int player, int type);
    bool hasWorkingAbility(int play, int ability);
    bool opponentsHaveWorkingAbility(int play, int ability);
    void acquireAbility(int play, int ability, bool firstTime=false);
    int ability(int player);
    int weight(int player);
    int currentInternalId(int slot) const;
    Pokemon::uniqueId pokenum(int player);
    bool hasWorkingItem(int player, int item);
    bool isWeatherWorking(int weather);
    bool isSeductionPossible(int seductor, int naiveone);
    int move(int player, int slot);
    int PP(int player, int slot) const;
    bool hasMove(int player, int move);
    int getType(int player, int slot);
    bool isFlying(int player);
    bool isOut(int player, int poke);
    bool hasSubstitute(int slot);
    bool hasMoved(int slot);
    void requestSwitchIns();
    void requestEndOfTurnSwitchIns();
    void requestSwitch(int player);
    bool linked(int linked, QString relationShip);
    void link(int linker, int linked, QString relationShip);
    int linker(int linked, QString relationShip);
    void notifySub(int player, bool sub);
    int repeatNum(int player);
    PokeFraction getStatBoost(int player, int stat);
    /* "Pure" stat is without items */
    int getStat(int player, int stat, int purityLevel = 0);
    int getBoostedStat(int player, int stat);
    /* conversion for sending a message */
    quint8 ypoke(int, int i) const { return i; } /* aka 'your poke', or what you need to know if it's your poke */
    ShallowBattlePoke opoke(int slot, int play, int i) const; /* aka 'opp poke', or what you need to know if it's your opponent's poke */
    BattleDynamicInfo constructInfo(int player);
    void notifyInfos(int player = All);
    BattleStats constructStats(int player);

    void changeTempMove(int player, int slot, int move);
    void changeDefMove(int player, int slot, int move);
    void changeSprite(int player, Pokemon::uniqueId newForme);

    enum WeatherM
    {
	ContinueWeather,
	EndWeather,
	HurtWeather
    };

    enum Weather
    {
	NormalWeather = 0,
	Hail = 1,
	Rain = 2,
	SandStorm = 3,
	Sunny = 4
    };

    enum StatusFeeling
    {
	FeelConfusion,
	HurtConfusion,
	FreeConfusion,
	PrevParalysed,
	PrevFrozen,
	FreeFrozen,
	FeelAsleep,
	FreeAsleep,
	HurtBurn,
	HurtPoison
    };

    void sendMoveMessage(int move, int part=0, int src=0, int type=0, int foe=-1, int other=-1, const QString &q="");
    void sendAbMessage(int move, int part=0, int src=0, int foe=-1, int type=0, int other=-1);
    void sendItemMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);
    void sendBerryMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);

    void notifyFail(int p);
    /* Here C++0x would make it so much better looking with variadic templates! */
    void notify(int player, int command, int who);
    template<class T>
    void notify(int player, int command, int who, const T& param);
    template<class T1, class T2>
    void notify(int player, int command, int who, const T1& param1, const T2& param2);
    template<class T1, class T2, class T3>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3);
    template<class T1, class T2, class T3, class T4>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4);
    template<class T1, class T2, class T3, class T4, class T5>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5);
    template<class T1, class T2, class T3, class T4, class T5, class T6>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5, const T6 &param6);
public slots:
    void battleChoiceReceived(int id, const BattleChoice &b);
    void battleChat(int id, const QString &str);
public:
    void spectatingChat(int id, const QString &str);
private:
    bool canCancel(int player);
    void cancel(int player);
    void addDraw(int player);

    bool validChoice(const BattleChoice &b);
    void storeChoice(const BattleChoice &b);
    bool allChoicesOkForPlayer(int player);
    bool allChoicesSet();

    void appendBattleLog(const QString &command, const QString &message);
    void writeUsageLog();
    QString name(int id);
    QString nick(int slot);
signals:
    /* Due to threading issue, and the signal not being direct,
       The battle might already be deleted when the signal is received.

       So the parameter "publicId" is for the server to not to have to use
       sender(); */
    void battleInfo(int publicId, int id, const QByteArray &info);
    void battleFinished(int battleid, int result, int winner, int loser);
private:
    mutable QMutex spectatorMutex;

    /* if battle ends, stop the battle thread */
    void testWin();
    void endBattle(int result, int winner, int loser);
    int spectatorKey(int id) const {
        return 10000 + id;
    }

    /* What choice we allow the players to have */
    QList<BattleChoices> options;
    /* Is set to false once a player choses it move */
    QList<int> hasChoice;
    /* just indicates if the player could originally move or not */
    QList<bool> couldMove;
    QList<QPointer<Player> > pendingSpectators;

    int ratings[2];

    /* timers */
    QAtomicInt timeleft[2];
    QAtomicInt startedAt[2];
    bool timeStopped[2];
    QBasicTimer *timer;
    /*.*/
    int myid[2];
    QString winMessage[2];
    QString loseMessage[2];

    bool useBattleLog;
    bool recordUsage;
    QFile battleLog;
    QFile usageLog;
protected:
    void timerEvent(QTimerEvent *);

    void startClock(int player, bool broadCoast = true);
    void stopClock(int player, bool broadCoast = false);
    int timeLeft(int player);

    void yield();
    void schedule();
public:
    std::vector<int> targetList;
    /* Calls the effects of source reacting to name */
    void calleffects(int source, int target, const QString &name);
    /* This time the pokelong effects */
    void callpeffects(int source, int target, const QString &name);
    /* this time the general battle effects (imprison, ..) */
    void callbeffects(int source, int target, const QString &name, bool stopOnFail = false);
    /* The team zone effects */
    void callzeffects(int source, int target, const QString &name);
    /* The slot effects */
    void callseffects(int source, int target, const QString &name);
    /* item effects */
    void callieffects(int source, int target, const QString &name);
    /* Ability effects */
    void callaeffects(int source, int target, const QString &name);

    void emitCommand(int player, int players, const QByteArray &data);
public:
    /* The players ordered by speed are stored there */
    std::vector<int> speedsVector;
    unsigned int currentSlot;

    int weather;
    int weatherCount;

    bool applyingMoveStatMods;

    struct BasicPokeInfo {
        Pokemon::uniqueId id;
        float weight;
        int type1;
        int type2;
        int ability;
        int level;

        int moves[4];
        quint8 pps[4];
        quint8 dvs[6];
        int stats[6];
        //The boost in HP is useless but avoids headaches
        int boosts[8];
    };

    struct BasicMoveInfo {
        char critRaise;
        char repeatMin;
        char repeatMax;
        char priority;
        int flags;
        int power; /* unsigned char in the game, but can be raised by effects */
        int accuracy; /* Same */
        char type;
        char category; /* Physical/Special/Other */
        int rate; /* Same */
        char flinchRate;
        char recoil;
        int attack;
        char targets;
        char healing;
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
    enum EffectType {
        TurnEffect,
        PokeEffect,
        SlotEffect,
        ItemEffect,
        AbilityEffect,
        ZoneEffect,
        FieldEffect,
        OwnEffect
    } ;
private:
    /**************************************/
    /*** VIVs: very important variables ***/
    /**************************************/
    /* Those variable are used throughout the battle to describe the situation.
       These are 'dynamic', in contrary to 'static'. For exemple when a pokemon
       is switched in, its type and ability and moves are stored in there, taken
       from their 'static' value that are in the TeamBattle struct. Then they
       can be changed (like with ability swap) but when the poke is sent back then
       back in the dynamic value is restored to the static one. */

    struct PokeContext {
        /* Variables that are reset when the poke is switched out.
            Like for exemple a Requiem one... */
        context pokelong;
        /* Variables that are reset every turn right before everything else happens
            at the very beginning of a turn */
        context turnlong;

        /* Structs containing raw information */
        BasicPokeInfo fieldpoke;
        BasicMoveInfo fieldmove;

        /* The counters (Encore, Taun, Disable) associated with the pokemon */
        BattleCounters counters;

        /* The choice of a player, accessed by move ENCORE */
        BattleChoice choice;
    };

    /* General things like last move ever used, etc. */
    context battlelong;
    /* Moves that affect a team */
    context teamzone[2];
    /* Moves that affect a particular Slot (wish, ...) */
    QList<context> slotzone;

    QList<PokeContext> contexts;

public:
    struct priorityBracket {
        quint8 bracket;
        quint8 priority;

        priorityBracket(quint8 b=0, quint8 p=0) : bracket(b), priority(p) {

        }

        operator int() const {
            return (this->bracket << 8) | priority;
        }

        bool operator <= (const priorityBracket &b) const {
            return this->bracket < b.bracket || (this->bracket==b.bracket && priority <= b.priority);
        }

        bool operator == (const priorityBracket &b) const {
            return this->bracket == b.bracket && priority == b.priority;
        }

        bool operator < (const priorityBracket &b) const {
            return this->bracket < b.bracket || (this->bracket == b.bracket && priority < b.priority);
        }
    };
private:
    QVector<priorityBracket> endTurnEffects;
    QHash<QString, priorityBracket> effectToBracket;
    QHash<priorityBracket, int> bracketCount;
    QHash<priorityBracket, int> bracketType;
    QHash<priorityBracket, QString> bracketToEffect;

    void getVectorRef(priorityBracket b);

    typedef void (BattleSituation::*VoidFunction)();
    QVector<QPair<int, VoidFunction> > ownEndFunctions;
    typedef void (BattleSituation::*IntFunction)(int);
    QHash<priorityBracket, IntFunction> ownSEndFunctions;

    void initializeEndTurnFunctions();
public:
    typedef void (*MechanicsFunction) (int source, int target, BattleSituation &b);

    void addEndTurnEffect(EffectType type, int bracket, int priority, int slot = 0, const QString &effect = QString(),
                            MechanicsFunction f=NULL,IntFunction f2 = NULL);
    void addEndTurnEffect(EffectType type, priorityBracket bracket, int slot = 0, const QString &effect = QString(),
                            MechanicsFunction f=NULL,IntFunction f2 = NULL);
    void removeEndTurnEffect(EffectType type, int slot, const QString &effect);

    context &battleMemory() {
        return battlelong;
    }

    context &teamMemory(int player) {
        return teamzone[player];
    }

    context &slotMemory(int slot) {
        return slotzone[slot];
    }

    PokeContext &getContext(int slot) {
        return contexts[indexes[slot]];
    }

    const PokeContext &getContext(int slot) const {
        return contexts[indexes[slot]];
    }

    context &turnMemory(int slot) {
        return getContext(slot).turnlong;
    }

    context &pokeMemory(int slot) {
        return getContext(slot).pokelong;
    }

    BasicMoveInfo &tmove(int slot) {
        return getContext(slot).fieldmove;
    }

    BasicPokeInfo &fpoke(int slot) {
        return getContext(slot).fieldpoke;
    }

    BattleChoice &choice(int slot) {
        return getContext(slot).choice;
    }

    BattleCounters &counters(int slot) {
        return getContext(slot).counters;
    }

    const context &battleMemory() const {
        return battlelong;
    }

    const context &teamMemory(int player) const {
        return teamzone[player];
    }

    const context &slotMemory(int slot) const {
        return slotzone[slot];
    }

    const context &turnMemory(int slot) const {
        return getContext(slot).turnlong;
    }

    const context &pokeMemory(int slot) const {
        return getContext(slot).pokelong;
    }

    const BasicMoveInfo &tmove(int slot) const {
        return getContext(slot).fieldmove;
    }

    const BasicPokeInfo &fpoke(int slot) const {
        return getContext(slot).fieldpoke;
    }

    const BattleChoice &choice(int slot) const {
        return getContext(slot).choice;
    }

    int getInternalId(int slot) const {
        return indexes[slot];
    }

    int fromInternalId(int id) const {
        return indexes.indexOf(id);
    }

    /* Sleep clause necessity: only pokes asleep because of something else than rest are put there */
    // Public because used by Yawn
    int currentForcedSleepPoke[2];

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
private:
    QHash<int,QPair<int, QString> > spectators;
    /* Used when pokemon shift slots */
    QVector<int> indexes;
    /* Generator of random numbers */
    mutable MTRand_int32 rand_generator;
public:
    const QHash<int, QPair<int, QString> > &getSpectators() const {
        QMutexLocker m(&spectatorMutex);
        return spectators;
    }

    const QList<QPointer<Player> > &getPendingSpectators() const {
        return pendingSpectators;
    }

    struct QuitException {};
private:
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

    BattleConfiguration conf;
};

inline void BattleSituation::notify(int player, int command, int who)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who);

    emitCommand(who, player, tosend);
}

template<class T>
void BattleSituation::notify(int player, int command, int who, const T& param)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param;

    emitCommand(who, player, tosend);
}

template<class T1, class T2>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param1 << param2;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param1 << param2 << param3;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4, class T5>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4 << param5;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5, const T6 &param6)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4 << param5 << param6;

    emitCommand(who, player, tosend);
}

Q_DECLARE_METATYPE(QSharedPointer<QSet<int> >)

#endif // BATTLE_H

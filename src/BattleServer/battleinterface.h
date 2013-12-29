#ifndef BATTLEINTERFACE_H
#define BATTLEINTERFACE_H

#include <QtCore>
#include <Utilities/functions.h>
#include <PokemonInfo/pokemonstructs.h>

class TeamBattle;
class PokeBattle;
class BattleConfiguration;

/* Fixme: needs some sort of cache to avoid revs() creating a list
   each time */

class BattleInterface
{
public:
    enum {
        AllButPlayer = -2,
        All = -1,
        Player1,
        Player2,
        Slot11,
        Slot21,
        Slot12,
        Slot22
    };
//    typedef QVariantHash context;

    virtual const int & turn() const = 0;
    virtual const int & publicId() const = 0;
//    virtual const bool & rated() const = 0;
    virtual const QString & tier() const = 0;
    virtual quint32 clauses() const = 0;
//    virtual const int & attacker() const = 0;
//    virtual const int & attacked() const = 0;
    virtual int mode() const = 0;
//    virtual const int & numberOfSlots() const = 0;
//    virtual const bool & blocked() const = 0;
    virtual Pokemon::gen gen() const = 0;
//    virtual const int & attackCount() const = 0;
//    virtual const bool & rearrangeTime() const = 0;
//    virtual const int & selfKoer() const = 0;
//    virtual const int & repeatCount() const = 0;
//    virtual const bool & heatOfAttack() const = 0;

//    virtual const TeamBattle &pubteam(int id) = 0;
//    /* returns 0 or 1, or -1 if that player is not involved */
//    virtual int spot(int id) const = 0;
//    /* The other player */
//    virtual int opponent(int player) const = 0;
//    virtual int partner(int spot) const = 0;
//    virtual QList<int> revs(int slot) const = 0;
//    virtual QList<int> allRevs(int slot) const = 0; //returns even koed opponents
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    virtual int id(int spot) const = 0;
    virtual int rating(int spot) const = 0;
    /* Return the configuration of the players (1 refer to that player, 0 to that one...) */
    virtual const BattleConfiguration &configuration() const = 0;

//    virtual bool acceptSpectator(int id, bool authed=false) const = 0;
//    virtual void addSpectator(Player *p) = 0;

//    virtual void notifyClause(int clause) = 0;
//    virtual void notifyMiss(bool multitar, int player, int target) = 0;
//    virtual void notifyKO(int player) = 0;
//    /*
//        Below Player is either 1 or 0, aka the spot of the id.
//        Use the functions above to make conversions
//    */
//    virtual TeamBattle &team(int player) = 0;
    virtual const TeamBattle &team(int player) const = 0;
//    virtual PokeBattle &poke(int player) = 0;
//    virtual const PokeBattle &poke(int player) const = 0;
//    virtual PokeBattle &poke(int player, int poke) = 0;
    virtual const PokeBattle &poke(int player, int poke) const = 0;
//    virtual bool koed(int player) const = 0;
//    virtual bool wasKoed(int player) const = 0;
//    virtual int player(int slot) const = 0;
//    /* Returns -1 if none */
//    virtual int randomOpponent(int slot) const = 0;
//    /* Returns a koed one if none */
//    virtual int randomValidOpponent(int slot) const = 0;
//    virtual int slot(int player, int poke = 0) const = 0;
//    virtual int slotNum(int slot) const = 0;
//    virtual int countAlive(int player) const = 0;
//    virtual int countBackUp(int player) const = 0;
//    virtual bool canTarget(int attack, int attacker, int defender) const = 0;
//    virtual bool areAdjacent(int attacker, int defender) const = 0;
//    virtual bool multiples() const = 0;
//    virtual bool arePartners(int p1, int p2) const = 0;
//    virtual int numberPerSide() const  = 0;

//    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
//    virtual void run() = 0;
//    /* requests choice of action from the player */
//    /* return true if the pokemon has a choice to make (including switching & struggle)*/
//    virtual bool requestChoice(int player, bool acq = true /*private arg */, bool custom = false) = 0;
//    virtual void requestChoices() = 0; /* request from both players */
//    /* Shows what attacks are allowed or not */
//    virtual BattleChoices createChoice(int player) = 0;
//    virtual bool isMovePossible(int player, int slot) = 0;
//    /* called just after requestChoice(s) */
//    virtual void analyzeChoice(int player) = 0;
//    virtual void analyzeChoices() = 0;
//    virtual std::vector<int> sortedBySpeed() = 0;

//    /* Commands for the battle situation */
//    virtual void rearrangeTeams() = 0;
//    virtual void engageBattle() = 0;
//    virtual void beginTurn() = 0;
//    virtual void endTurn() = 0;
//    virtual void personalEndTurn(int source) = 0;
//    virtual void endTurnStatus(int player) = 0;
//    virtual void endTurnWeather() = 0;
//    virtual void callForth(int weather, int turns) = 0;
//    virtual void setupLongWeather(int weather) = 0;
//    /* Attack... */
//    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
//        represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
//    virtual void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true) = 0;
//    /* Returns true or false if an attack is going on or not */
//    virtual bool attacking() = 0;
//    virtual void makeTargetList(const QVector<int> &base) = 0;
//    /* Does not do extra operations,just a setter */
//    virtual void changeHp(int player, int newHp) = 0;
//    /* Sends a poke back to his pokeball (not koed) */
//    virtual void sendBack(int player, bool silent = false) = 0;
//    virtual void shiftSpots(int spot1, int spot2, bool silent = false) = 0;
//    void notifyHits(int number) = 0;
//    virtual void sendPoke(int player, int poke, bool silent = false) = 0;
//    virtual void callEntryEffects(int player) = 0;
//    virtual void koPoke(int player, int source, bool straightattack = false) = 0;
//    /* Does not do extra operations,just a setter */
//    virtual void changeStatMod(int player, int stat, int newstatmod) = 0;
//    virtual void changeForme(int player, int poke, const Pokemon::uniqueId &forme) = 0;
//    virtual void changePokeForme(int slot, const Pokemon::uniqueId &forme) = 0;
//    virtual void calculateTypeModStab(int orPlayer = -1, int orTarget = -1) = 0;
//    virtual void changeAForme(int player, int newforme) = 0;
//    virtual bool hasMinimalStatMod(int player, int stat) = 0;
//    virtual bool hasMaximalStatMod(int player, int stat) = 0;
//    virtual bool inflictStatMod(int player, int stat, int mod, int attacker, bool tell = true, bool *negative = NULL) = 0;
//    virtual void setLogging(bool logging) = 0;
//    virtual QString getBattleLogFilename() const = 0;

//    virtual bool canSendPreventMessage(int defender, int attacker) = 0;
//    virtual bool canSendPreventSMessage(int defender, int attacker) = 0;
//    virtual void preventStatMod(int player, int attacker) = 0;
//    /* Does not do extra operations,just a setter */
//    virtual void changeStatus(int player, int status, bool tell = true, int turns = 0) = 0;
//    virtual void changeStatus(int team, int poke, int status) = 0;
//    virtual void healStatus(int player, int status) = 0;
//    virtual void healConfused(int player) = 0;
//    virtual void healLife(int player, int healing) = 0;
//    virtual void healDamage(int player, int target) = 0;
//    virtual bool canGetStatus(int player, int status) = 0;
//    virtual void inflictStatus(int player, int Status, int inflicter, int minturns = 0, int maxturns = 0) = 0;
//    virtual bool isConfused(int player) = 0;
//    virtual void inflictConfused(int player, int source, bool tell=true) = 0;
//    virtual void inflictConfusedDamage(int player) = 0;
//    virtual void inflictRecoil(int source, int target) = 0;
//    virtual void inflictDamage(int player, int damage, int source, bool straightattack = false, bool goForSub = false) = 0;
//    virtual void inflictPercentDamage(int player, int percent, int source, bool straightattack = false) = 0;
//    virtual void inflictSubDamage(int player, int damage, int source) = 0;
//    virtual void disposeItem(int player) = 0;
//    virtual void eatBerry(int player, bool show=true) = 0;
//    /* Eats a berry, not caring about the item the pokemon has, etc. */
//    virtual void devourBerry(int player, int berry, int target) = 0;
//    virtual void acqItem(int player, int item) = 0;
//    virtual void loseItem(int player, bool real = true) = 0;
//    virtual void loseAbility(int player) = 0;
//    /* Removes PP.. */
//    virtual void changePP(int player, int move, int PP) = 0;
//    virtual void losePP(int player, int move, int loss) = 0;
//    virtual void gainPP(int player, int move, int gain) = 0;
//    //Uproarer
//    virtual bool isThereUproar() = 0;
//    virtual void addUproarer(int player) = 0;
//    virtual void removeUproarer(int player) = 0;
//    //Change turn order, use it carefully when you know those are correct values
//    virtual void makePokemonNext(int player) = 0;
//    virtual void makePokemonLast(int player) = 0;

//    virtual int calculateDamage(int player, int target) = 0;
//    virtual void applyMoveStatMods(int player, int target) = 0;
//    virtual bool testAccuracy(int player, int target, bool silent = false) = 0;
//    virtual void testCritical(int player, int target) = 0;
//    virtual void testFlinch(int player, int target) = 0;
//    virtual bool testStatus(int player) = 0;
//    virtual bool testFail(int player) = 0;
//    virtual void failSilently(int player) = 0;
//    virtual void fail(int player, int move, int part=0, int type=0, int trueSource = -1) = 0;
//    virtual bool hasType(int player, int type) = 0;
//    virtual bool hasWorkingAbility(int play, int ability) = 0;
//    virtual bool opponentsHaveWorkingAbility(int play, int ability) = 0;
//    virtual void acquireAbility(int play, int ability, bool firstTime=false) = 0;
//    virtual int ability(int player) = 0;
//    virtual int weight(int player) = 0;
//    virtual int currentInternalId(int slot) const = 0;
//    virtual Pokemon::uniqueId pokenum(int player) = 0;
//    virtual bool hasWorkingItem(int player, int item) = 0;
//    virtual bool isWeatherWorking(int weather) = 0;
//    virtual bool isSeductionPossible(int seductor, int naiveone) = 0;
//    virtual int move(int player, int slot) = 0;
//    virtual int PP(int player, int slot) const = 0;
//    virtual bool hasMove(int player, int move) = 0;
//    virtual int getType(int player, int slot) = 0;
//    virtual bool isFlying(int player) = 0;
//    virtual bool isOut(int player, int poke) = 0;
//    virtual bool hasSubstitute(int slot) = 0;
//    virtual bool hasMoved(int slot) = 0;
//    virtual void requestSwitchIns() = 0;
//    virtual void requestSwitch(int player) = 0;
//    virtual bool linked(int linked, QString relationShip) = 0;
//    virtual void link(int linker, int linked, QString relationShip) = 0;
//    virtual int linker(int linked, QString relationShip) = 0;
//    virtual void notifySub(int player, bool sub) = 0;
//    virtual int repeatNum(int player) = 0;
//    virtual PokeFraction getStatBoost(int player, int stat) = 0;
//    /* "Pure" stat is without items */
//    virtual int getStat(int player, int stat, int purityLevel = 0) = 0;
//    virtual int getBoostedStat(int player, int stat) = 0;
//    /* conversion for sending a message */
//    virtual quint8 ypoke(int, int i) const { return i = 0; } /* aka 'your poke', or what you need to know if it's your poke */
//    virtual ShallowBattlePoke opoke(int slot, int play, int i) const = 0; /* aka 'opp poke', or what you need to know if it's your opponent's poke */
//    virtual BattleDynamicInfo constructInfo(int player) = 0;
//    virtual void notifyInfos(int player = All) = 0;
//    virtual BattleStats constructStats(int player) = 0;

//    virtual void changeTempMove(int player, int slot, int move) = 0;
//    virtual void changeDefMove(int player, int slot, int move) = 0;
//    virtual void changeSprite(int player, Pokemon::uniqueId newForme) = 0;

//    enum WeatherM
//    {
//        ContinueWeather,
//        EndWeather,
//        HurtWeather
//    };

//    enum Weather
//    {
//        NormalWeather = 0,
//        Hail = 1,
//        Rain = 2,
//        SandStorm = 3,
//        Sunny = 4
//    };

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

//    virtual void sendMoveMessage(int move, int part=0, int src=0, int type=0, int foe=-1, int other=-1, const QString &q="") = 0;
//    virtual void sendAbMessage(int move, int part=0, int src=0, int foe=-1, int type=0, int other=-1) = 0;
//    virtual void sendItemMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1) = 0;
//    virtual void sendBerryMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1) = 0;

//    virtual void notifyFail(int p) = 0;

//    virtual void battleChoiceReceived(int id, const BattleChoice &b) = 0;
//    virtual void battleChat(int id, const QString &str) = 0;

//    virtual void spectatingChat(int id, const QString &str) = 0;

//    /* Calls the effects of source reacting to name */
//    virtual void calleffects(int source, int target, const QString &name) = 0;
//    virtual void calle6effects(int source) = 0;
//    /* This time the pokelong effects */
//    virtual void callpeffects(int source, int target, const QString &name) = 0;
//    /* this time the general battle effects (imprison, ..) */
//    virtual void callbeffects(int source, int target, const QString &name, bool stopOnFail = false) = 0;
//    /* The team zone effects */
//    virtual void callzeffects(int source, int target, const QString &name) = 0;
//    /* The slot effects */
//    virtual void callseffects(int source, int target, const QString &name) = 0;
//    /* item effects */
//    virtual void callieffects(int source, int target, const QString &name) = 0;
//    /* Ability effects */
//    virtual void callaeffects(int source, int target, const QString &name) = 0;

//    virtual void emitCommand(int player, int players, const QByteArray &data) = 0;

//    struct BasicPokeInfo {
//        Pokemon::uniqueId id;
//        float weight;
//        int type1;
//        int type2;
//        int ability;
//        int level;

//        int moves[4];
//        quint8 pps[4];
//        quint8 dvs[6];
//        int stats[6];
//        //The boost in HP is useless but avoids headaches
//        int boosts[8];
//    };

//    struct BasicMoveInfo {
//        char critRaise;
//        char repeatMin;
//        char repeatMax;
//        char priority;
//        int flags;
//        int power; /* unsigned char in the game, but can be raised by effects */
//        int accuracy; /* Same */
//        char type;
//        char category;
//        int rate; /* Same */
//        char flinchRate;
//        char recoil;
//        int attack;
//        char targets;
//        char healing;
//        char classification;
//        char status;
//        char statusKind;
//        char minTurns;
//        char maxTurns;
//        quint32 statAffected;
//        quint32 boostOfStat;
//        quint32 rateOfStat;
//        bool kingRock;

//        void reset();
//    };
//    virtual context &battleMemory() = 0;
//    virtual context &teamMemory(int player) = 0;
//    virtual context &slotMemory(int slot) = 0;
//    virtual context &turnMemory(int slot) = 0;
//    virtual context &pokeMemory(int slot) = 0;
//    virtual BasicMoveInfo &tmove(int slot) = 0;
//    virtual BasicPokeInfo &fpoke(int slot) = 0;
//    virtual BattleChoice &choice(int slot) = 0;
//    virtual const context &battleMemory() = 0;
//    virtual const context &teamMemory(int player) = 0;
//    virtual const context &slotMemory(int slot) = 0;
//    virtual const context &turnMemory(int slot) = 0;
//    virtual const context &pokeMemory(int slot) = 0;
//    virtual const BasicMoveInfo &tmove(int slot) = 0;
//    virtual const BasicPokeInfo &fpoke(int slot) = 0;
//    virtual const BattleChoice &choice(int slot) = 0;
//    virtual int getInternalId(int slot) const  = 0;
//    virtual int fromInternalId(int id) const = 0;

    /* Generator of random numbers */
//    virtual unsigned true_rand() const  = 0;
};

#endif // BATTLEINTERFACE_H

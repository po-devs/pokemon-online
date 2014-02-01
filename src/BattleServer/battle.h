#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include <PokemonInfo/pokemonstructs.h>
#include <Utilities/mtrand.h>
#include <Utilities/coreclasses.h>
#include "battlebase.h"
#include "battlepluginstruct.h"
#include "battlecounters.h"

class BattlePlugin;
class BattlePStorage;

/* Fixme: needs some sort of cache to avoid revs() creating a list
   each time */

class BattleSituation : public BattleBase
{
    Q_OBJECT
public:
    typedef QVariantHash context;

    BattleSituation(const BattlePlayer &p1, const BattlePlayer &p2, const ChallengeInfo &additionnalData, int id, const TeamBattle &t1, const TeamBattle &t2, BattleServerPluginManager *p);
    ~BattleSituation();

    /* Shows what attacks are allowed or not */
    BattleChoices createChoice(int player);
    bool isMovePossible(int player, int slot);
    /* called just after requestChoice(s) */
    void analyzeChoice(int player);
    void analyzeChoices(); 
    std::vector<int> sortedBySpeed();

    /* Commands for the battle situation */
    void engageBattle();
    void endTurn();
    void personalEndTurn(int source);
    void endTurnStatus(int player);
    void endTurnWeather();
    void endTurnDefrost();
    void callForth(int weather, int turns);
    /* Attack... */
    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
	represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
    void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true);
    void useItem(int player, int item, int target, int attack);
    void makeTargetList(const QVector<int> &base);
    /* Does not do extra operations,just a setter */
    void changeHp(int player, int newHp);
    /* Sends a poke back to his pokeball (not koed) */
    void sendBack(int player, bool silent = false);
    void shiftSpots(int spot1, int spot2, bool silent = false);
    void sendPoke(int player, int poke, bool silent = false);
    void callEntryEffects(int player);
    void koPoke(int player, int source, bool straightattack = false);
    void changeForme(int player, int poke, const Pokemon::uniqueId &forme);
    void changePokeForme(int slot, const Pokemon::uniqueId &forme);
    void calculateTypeModStab(int orPlayer = -1, int orTarget = -1);
    void changeAForme(int player, int newforme);
    bool hasMinimalStatMod(int player, int stat);
    bool hasMaximalStatMod(int player, int stat);
    bool inflictStatMod(int player, int stat, int mod, int attacker, bool tell, bool *negative);
    bool inflictStatMod(int player, int stat, int mod, int attacker, bool tell = true) {return inflictStatMod(player, stat, mod, attacker, tell, NULL);}
private:
    /* Returns false if blocked */
    bool loseStatMod(int player, int stat, int malus, int attacker, bool tell=true);
public:
    void debug(const QString &message);

    bool canSendPreventMessage(int defender, int attacker);
    bool canSendPreventSMessage(int defender, int attacker);
    void preventStatMod(int player, int attacker);
    /* Does not do extra operations,just a setter */
    virtual void changeStatus(int team, int poke, int status) { BattleBase::changeStatus(team, poke, status);}
    void changeStatus(int player, int status, bool tell = true, int turns = 0);
    bool canGetStatus(int player, int status);
    bool canHeal(int s);
    bool canBypassSub(int t);
    void inflictStatus(int player, int Status, int inflicter, int minturns = 0, int maxturns = 0);
    void inflictConfused(int player, int source, bool tell=true);
    void inflictRecoil(int source, int target);
    void inflictDamage(int player, int damage, int source, bool straightattack = false, bool goForSub = false);
    void inflictPercentDamage(int player, int percent, int source, bool straightattack = false);
    void inflictSubDamage(int player, int damage, int source);
    void disposeItem(int player);
    void eatBerry(int player, bool show=true);
    /* Eats a berry, not caring about the item the pokemon has, etc. */
    void devourBerry(int player, int berry, int target);
    void acqItem(int player, int item);
    bool canLoseItem(int player, int attacker);
    bool canBoostKnockOff(int player, int attacker);
    void loseItem(int player, bool real = true);
    void loseAbility(int player);
    /* Removes PP.. */
    void changePP(int player, int move, int PP);
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
    void fail(int player, int move, int part=0, int type=0, int trueSource = -1);
    bool hasWorkingAbility(int play, int ability);
    bool hasWorkingTeamAbility(int play, int ability);
    bool opponentsHaveWorkingAbility(int play, int ability);
    void acquireAbility(int play, int ability, bool firstTime=false);
    int ability(int player);
    int weight(int player);
    bool hasWorkingItem(int player, int item);
    bool isWeatherWorking(int weather);
    bool isSeductionPossible(int seductor, int naiveone);
    int getType(int player, int slot) const;
    QVector<int> getTypes(int player) const;
    void addType(int player, int type);
    void setType(int player, int type);
    int rawTypeEff(int type, int player);
    PokeFraction effFraction(int typeeff);
    bool ineffective(int typeeff) {return typeeff < -50;}
    bool isFlying(int player);
    bool hasFlyingEffect(int player); //returns true if has flying effect outside of flying type
    bool hasGroundingEffect(int player); //returns true for gravity, ingrain, ...
    void requestSwitchIns();
    void requestEndOfTurnSwitchIns();
    void requestSwitch(int player, bool entryEffects=true);
    bool linked(int linked, QString relationShip);
    void link(int linker, int linked, QString relationShip);
    int linker(int linked, QString relationShip);
    int repeatNum(int player);
    PokeFraction getStatBoost(int player, int stat);
    /* "Pure" stat is without items */
    int getStat(int player, int stat) {return getStat(player, stat, 0);}
    int getStat(int player, int stat, int purityLevel);
    int getBoostedStat(int player, int stat);
    /* conversion for sending a message */
    quint8 ypoke(int, int i) const { return i; } /* aka 'your poke', or what you need to know if it's your poke */
    ShallowBattlePoke opoke(int slot, int play, int i) const; /* aka 'opp poke', or what you need to know if it's your opponent's poke */
    BattleStats constructStats(int player);
    BattleDynamicInfo constructInfo(int player);

    void changeDefMove(int player, int slot, int move);

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

private:
    virtual void notifySituation(int dest);

    virtual void storeChoice(const BattleChoice &b);
    void setupMove(int player, int move);
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

public:
    unsigned int currentSlot;

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
        TurnMemory turnhard;

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
    QVector<int> bpmodifiers;

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

    void chainBp(int p, int pow);

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

    TurnMemory &turnMem(int slot) {
        return getContext(slot).turnhard;
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

    const TurnMemory &turnMem(int slot) const {
        return getContext(slot).turnhard;
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
private:
    /* Used when pokemon shift slots */
    QVector<int> indexes;
    bool megas[2];
};

Q_DECLARE_METATYPE(BattleSituation::MechanicsFunction)
Q_DECLARE_METATYPE(QSharedPointer<QSet<int> >)

#endif // BATTLE_H

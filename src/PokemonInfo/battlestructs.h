#ifndef BATTLESTRUCTS_H
#define BATTLESTRUCTS_H

/* damage formula: http://www.smogon.com/dp/articles/damage_formula */

#include <QtCore>
#include "../Utilities/functions.h"
#include "pokemoninfo.h"

class TeamInfo;
class PokePersonal;

enum BattleResult
{
    Forfeit,
    Win,
    Tie,
    Close
};

class BattleMove
{
    PROPERTY(quint8, PP);
    PROPERTY(quint8, totalPP);
    PROPERTY(quint16, num);
public:
    BattleMove();

    void load(int gen);
    operator int () {return num();}
};

class PokeBattle;

/* A pokemon as viewed by the opponent: nearly no info */
class ShallowBattlePoke
{
    PROPERTY(QString, nick)
    PROPERTY(quint32, fullStatus)
    PROPERTY(Pokemon::uniqueId, num)
    PROPERTY(bool, shiny)
    PROPERTY(quint8, gender)
    PROPERTY(quint8, level)
public:
    ShallowBattlePoke();
    ShallowBattlePoke(const PokeBattle &poke);

    int status() const;
    void changeStatus(int status);

    void addStatus(int status);
    void removeStatus(int status);

    void init(const PokeBattle &poke);
    virtual quint8 lifePercent() const { return m_prop_lifePercent; }
    quint8 &lifePercent() { return m_prop_lifePercent; }
private:
    quint8 m_prop_lifePercent;
};

QDataStream & operator >> (QDataStream &in, ShallowBattlePoke &po);
QDataStream & operator << (QDataStream &out, const ShallowBattlePoke &po);

QDataStream & operator >> (QDataStream &in, BattleMove &mo);
QDataStream & operator << (QDataStream &out, const BattleMove &mo);

class PokeBattle : public ShallowBattlePoke
{
    PROPERTY(QList<int>, dvs)
    PROPERTY(QList<int>, evs)
    PROPERTY(quint16, lifePoints)
    PROPERTY(quint16, totalLifePoints)
    PROPERTY(quint16, item)
    PROPERTY(quint16, ability)
    PROPERTY(quint8, nature)
    PROPERTY(quint8, happiness)
    /* Below is only known by battle */
    PROPERTY(quint16, itemUsed)
    PROPERTY(quint16, itemUsedTurn)
    PROPERTY(qint8, statusCount)
    PROPERTY(qint8, oriStatusCount)
public:
    PokeBattle();

    void init(PokePersonal &poke);

    BattleMove& move(int i);
    const BattleMove& move(int i) const;

    quint16 normalStat(int stat) const;
    void updateStats(int gen);

    bool ko() const {return lifePoints() == 0 || num() == Pokemon::NoPoke || status() == Pokemon::Koed;}
    bool isFull() const { return lifePoints() == totalLifePoints(); }
    quint8 lifePercent() const { return lifePoints() == 0 ? 0 : std::max(1, lifePoints()*100/totalLifePoints());}

    void setNormalStat(int, quint16);
private:
    BattleMove m_moves[4];

    quint16 normal_stats[5];
};

QDataStream & operator >> (QDataStream &in, PokeBattle &po);
QDataStream & operator << (QDataStream &out, const PokeBattle &po);

class TeamBattle
{
public:
    TeamBattle();
    /* removes the invalid pokemons */
    TeamBattle(TeamInfo &other);

    void init(TeamInfo &other);
    void generateRandom(int gen);

    PokeBattle& poke(int i);
    const PokeBattle& poke(int i) const;

    int internalId(const PokeBattle &p) const;
    const PokeBattle &getByInternalId(int i) const;

    void switchPokemon(int pok1, int pok2);
    template<class T>
    void setIndexes(T indexes[6]) {
        for (int i = 0; i < 6; i++) {
            m_indexes[i] = indexes[i];
        }
    }

    void resetIndexes();

    bool invalid() const;

    QString name;
    QString info;
    int gen;
private:
    PokeBattle m_pokemons[6];
    int m_indexes[6];
};

QDataStream & operator >> (QDataStream &in, TeamBattle &te);
QDataStream & operator << (QDataStream &out, const TeamBattle &te);

struct ShallowShownPoke
{
public:
    ShallowShownPoke();
    void init(const PokeBattle &b);

    bool item;
    Pokemon::uniqueId num;
    quint8 level;
    quint8 gender;
};

QDataStream & operator >> (QDataStream &in, ShallowShownPoke &po);
QDataStream & operator << (QDataStream &out, const ShallowShownPoke &po);

class ShallowShownTeam
{
public:
    ShallowShownTeam(){}
    ShallowShownTeam(const TeamBattle &t);

    ShallowShownPoke &poke(int index) {
        return pokemons[index];
    }
    const ShallowShownPoke &poke(int index) const {
        return pokemons[index];
    }
private:
    ShallowShownPoke pokemons[6];
};

QDataStream & operator >> (QDataStream &in, ShallowShownTeam &po);
QDataStream & operator << (QDataStream &out, const ShallowShownTeam &po);

struct BattleChoices
{
    /* Sets everything to true */
    BattleChoices();
    void disableSwitch();
    void disableAttack(int attack);
    void disableAttacks();

    bool switchAllowed;
    bool attacksAllowed;
    bool attackAllowed[4];
    quint8 numSlot;

    bool struggle() const { return qFind(attackAllowed, attackAllowed+4, true) == attackAllowed+4; }

    static BattleChoices SwitchOnly(quint8 numslot);
};

QDataStream & operator >> (QDataStream &in, BattleChoices &po);
QDataStream & operator << (QDataStream &out, const BattleChoices &po);

enum ChoiceType {
    CancelType,
    AttackType,
    SwitchType,
    RearrangeType,
    CenterMoveType,
    DrawType
};

struct CancelChoice {

};

struct AttackChoice {
    qint8 attackSlot;
    qint8 attackTarget;
};

struct SwitchChoice {
    qint8 pokeSlot;
};

struct RearrangeChoice {
    qint8 pokeIndexes[6];
};

struct MoveToCenterChoice {

};

struct DrawChoice {

};

union ChoiceUnion
{
    CancelChoice cancel;
    AttackChoice attack;
    SwitchChoice switching;
    RearrangeChoice rearrange;
    MoveToCenterChoice move;
    DrawChoice draw;
};

struct BattleChoice {
    quint8 type;
    quint8 playerSlot;
    ChoiceUnion choice;

    BattleChoice(){}
    BattleChoice(int slot, const CancelChoice &c) {
        choice.cancel = c;
        playerSlot = slot;
        type = CancelType;
    }
    BattleChoice(int slot, const AttackChoice &c) {
        choice.attack = c;
        type = AttackType;
        playerSlot = slot;
    }
    BattleChoice(int slot, const SwitchChoice &c) {
        choice.switching = c;
        type = SwitchType;
        playerSlot = slot;
    }
    BattleChoice(int slot, const RearrangeChoice &c) {
        choice.rearrange = c;
        type = RearrangeType;
        playerSlot = slot;
    }
    BattleChoice(int slot, const MoveToCenterChoice &c) {
        choice.move = c;
        type = CenterMoveType;
        playerSlot = slot;
    }
    BattleChoice(int slot, const DrawChoice &c) {
        choice.draw = c;
        type = DrawType;
        playerSlot = slot;
    }

    bool attackingChoice() const {
        return type == AttackType;
    }

    bool switchChoice() const {
        return type == SwitchType;
    }

    bool moveToCenterChoice() const {
        return type == CenterMoveType;
    }

    bool cancelled() const {
        return type == CancelType;
    }

    bool rearrangeChoice() const {
        return type == RearrangeType;
    }

    bool drawChoice() const {
        return type == DrawType;
    }

    int target() const {
        return choice.attack.attackTarget;
    }

    int attackSlot() const {
        return choice.attack.attackSlot;
    }

    int pokeSlot() const {
        return choice.switching.pokeSlot;
    }

    /* The person who's making the choice */
    int slot() const {
        return playerSlot;
    }

    void setTarget(int target) {
        choice.attack.attackTarget = target;
    }

    void setAttackSlot(int slot) {
        choice.attack.attackSlot = slot;
    }

    void setPokeSlot(int slot) {
        choice.switching.pokeSlot = slot;
    }

    bool match(const BattleChoices &avail) const;
};

QDataStream & operator >> (QDataStream &in, BattleChoice &po);
QDataStream & operator << (QDataStream &out, const BattleChoice &po);

struct ChallengeInfo
{
    enum ChallengeDesc
    {
        Sent,
        Accepted,
        Cancelled,
        Busy,
        Refused,
        InvalidTeam,
        InvalidGen,

        ChallengeDescLast
    };

    enum Clauses
    {
        SleepClause = 1,
        FreezeClause = 2,
        DisallowSpectator = 4,
        ItemClause = 8,
        ChallengeCup = 16,
        NoTimeOut = 32,
        SpeciesClause = 64,
        RearrangeTeams = 128,
        SelfKO = 256
    };

    enum Mode
    {
        Singles,
        Doubles,
        Triples,
        Rotation,
        ModeFirst = Singles,
        ModeLast = Rotation
    };

    static const int numberOfClauses = 9;

    static QString clauseText[numberOfClauses];
    static QString clauseBattleText[numberOfClauses];
    static QString clauseDescription[numberOfClauses];

    static const int numberOfModes = ModeLast-ModeFirst+1;

    static QString modeText[numberOfModes];

    static QString clause(int index) {
        return index >= 0 && index < numberOfClauses ? clauseText[index] : "";
    }
    
    static QString battleText(int index) {
        return index >= 0 && index < numberOfClauses ? clauseBattleText[index] : "";
    }

    static QString description(int index) {
        return index >= 0 && index < numberOfClauses ? clauseDescription[index] : "";
    }

    static QString modeName(int index) {
        return index >= ModeFirst && index <= ModeLast ? modeText[index-ModeFirst] : "";
    }

    /* Insensitive case search for the clause, returns -1 if not found */
    static int clause (const QString &name) {
        for (int i = 0; i < numberOfClauses; i++) {
            if (clauseText[i].toLower() == name.toLower()) {
                return i;
            }
        }
        return -1;
    }

    quint32 clauses;

    qint8 dsc;
    qint32 opp;
    quint8 mode;
    bool rated;

    explicit ChallengeInfo(int desc=0, int opponent=0, quint32 clauses = SleepClause, quint8 mode=Singles)
        : clauses(clauses), dsc(desc), opp(opponent), mode(mode)
    {
    }

    int opponent() const {return opp;}
    qint8 desc() const { return dsc;}
    bool sleepClause() const {return clauses & SleepClause;}

    operator int () const {
        return opponent();
    }
};

QDataStream & operator >> (QDataStream &in, ChallengeInfo &c);
QDataStream & operator << (QDataStream &out, const ChallengeInfo &c);

struct BattleConfiguration
{
    quint8 gen;
    quint8 mode;
    qint32 ids[2];
    quint32 clauses;

    int slot(int spot, int poke = 0) const  {
        return spot + poke*2;
    }

    int spot(int id) const {
        return ids[0] == id ? 0 : 1;
    }
};

inline QDataStream & operator >> (QDataStream &in, BattleConfiguration &c)
{
    in >> c.gen >> c.mode >> c.ids[0] >> c.ids[1] >> c.clauses;

    return in;
}

inline QDataStream & operator << (QDataStream &out, const BattleConfiguration &c)
{
    out << c.gen << c.mode << c.ids[0] << c.ids[1] << c.clauses;

    return out;
}

struct BattleDynamicInfo
{
    qint8 boosts[7];
    enum {
        Spikes=1,
        SpikesLV2=2,
        SpikesLV3=4,
        StealthRock=8,
        ToxicSpikes=16,
        ToxicSpikesLV2=32
    };
    quint8 flags;
};

inline QDataStream & operator >> (QDataStream &in, BattleDynamicInfo &c)
{
    in >> c.boosts[0] >> c.boosts[1] >> c.boosts[2] >> c.boosts[3] >> c.boosts[4] >> c.boosts[5] >> c.boosts[6] >> c.flags;

    return in;
}

inline QDataStream & operator << (QDataStream &out, const BattleDynamicInfo &c)
{
    out << c.boosts[0] << c.boosts[1] << c.boosts[2] << c.boosts[3] << c.boosts[4] << c.boosts[5] << c.boosts[6] << c.flags;

    return out;
}

struct BattleStats
{
    qint16 stats[5];
};

inline QDataStream & operator >> (QDataStream &in, BattleStats &c)
{
    in >> c.stats[0] >> c.stats[1] >> c.stats[2] >> c.stats[3] >> c.stats[4];

    return in;
}

inline QDataStream & operator << (QDataStream &out, const BattleStats &c)
{
    out << c.stats[0] << c.stats[1] << c.stats[2] << c.stats[3] << c.stats[4];

    return out;
}

struct FindBattleData
{
    bool rated;
    bool sameTier;
    bool ranged;
    quint16 range;
    quint8 mode;
    //quint32 forcedClauses;
    //quint32 bannedClauses;
};

QDataStream& operator >> (QDataStream &in, FindBattleData &f);
QDataStream& operator << (QDataStream &out, const FindBattleData &f);


#endif // BATTLESTRUCTS_H

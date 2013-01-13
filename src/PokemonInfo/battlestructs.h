#ifndef BATTLESTRUCTS_H
#define BATTLESTRUCTS_H

/* damage formula: http://www.smogon.com/dp/articles/damage_formula */

#include <QtCore>
#include "../Utilities/functions.h"
#include "../Utilities/coreclasses.h"
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
    PROPERTY(quint8, PP)
    PROPERTY(quint8, totalPP)
    PROPERTY(quint16, num)
public:
    BattleMove();

    void load(Pokemon::gen gen);
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
    bool hasStatus(int status) const;
    bool ko() const {return lifePercent() == 0 || num() == Pokemon::NoPoke || status() == Pokemon::Koed;}

    void init(const PokeBattle &poke);
    virtual quint8 lifePercent() const { return mLifePercent; }
    virtual int life() const { return mLifePercent; }
    virtual int totalLife() const { return 100; }
    virtual void setLife(int newLife) { mLifePercent = newLife;}
    virtual void setLifePercent(quint8 percent) {mLifePercent = percent;}
    void setNum(Pokemon::uniqueId num) {this->num() = num;}

    bool operator == (const ShallowBattlePoke &other) const {
        return gender() == other.gender() && level() == other.level()
                && shiny() == other.shiny() && num() == other.num()
                && nick() == other.nick() && status() == other.status()
                && mLifePercent == other.mLifePercent;
    }

private:
    quint8 mLifePercent;
};

DataStream & operator >> (DataStream &in, ShallowBattlePoke &po);
DataStream & operator << (DataStream &out, const ShallowBattlePoke &po);

DataStream & operator >> (DataStream &in, BattleMove &mo);
DataStream & operator << (DataStream &out, const BattleMove &mo);

class PokeBattle : public ShallowBattlePoke
{
    PROPERTY(QList<int>, dvs)
    PROPERTY(QList<int>, evs)
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
    void updateStats(Pokemon::gen gen);

    bool isFull() const { return lifePoints() == totalLifePoints(); }
    quint8 lifePercent() const { return lifePoints() == 0 ? 0 : std::max(1, lifePoints()*100/totalLifePoints());}
    virtual void setLife(int newLife) {mLifePoints = newLife;}
    virtual void setLifePercent(quint8 percent) {mLifePoints = percent == 1 ? 1 :percent * totalLifePoints() / 100;}
    virtual int life() const { return mLifePoints; }
    virtual int totalLife() const { return m_prop_totalLifePoints;}
    quint16 lifePoints() const { return mLifePoints;}
    quint16 &lifePoints() { return mLifePoints;}

    void setNormalStat(int, quint16);
private:
    BattleMove m_moves[4];

    quint16 normal_stats[5];
    quint16 mLifePoints;
};

DataStream & operator >> (DataStream &in, PokeBattle &po);
DataStream & operator << (DataStream &out, const PokeBattle &po);

class PersonalTeam;

class TeamBattle
{
public:
    TeamBattle();
    /* removes the invalid pokemons */
    TeamBattle(PersonalTeam &other);

    void generateRandom(Pokemon::gen gen);

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

    void setItems(const QHash<quint16,quint16> &items) {
        this->items = items;
    }

    void resetIndexes();

    bool invalid() const;

    QHash<quint16,quint16> items;
    QString name;
    QString tier;
    Pokemon::gen gen;
private:
    PokeBattle m_pokemons[6];
    int m_indexes[6];
};

DataStream & operator >> (DataStream &in, TeamBattle &te);
DataStream & operator << (DataStream &out, const TeamBattle &te);

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

DataStream & operator >> (DataStream &in, ShallowShownPoke &po);
DataStream & operator << (DataStream &out, const ShallowShownPoke &po);

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

DataStream & operator >> (DataStream &in, ShallowShownTeam &po);
DataStream & operator << (DataStream &out, const ShallowShownTeam &po);

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

DataStream & operator >> (DataStream &in, BattleChoices &po);
DataStream & operator << (DataStream &out, const BattleChoices &po);

enum ChoiceType {
    CancelType,
    AttackType,
    SwitchType,
    RearrangeType,
    CenterMoveType,
    DrawType,
    ItemType
};

struct ItemChoice {
    quint16 item;
    qint8 target;
    qint8 attack;
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
    ItemChoice item;
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
    BattleChoice(int slot, const ItemChoice &c) {
        choice.item = c;
        type = ItemType;
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

    bool itemChoice() const {
        return type == ItemType;
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

    int item() const {
        return choice.item.item;
    }

    int itemTarget() const {
        return choice.item.target;
    }

    int itemAttack() const {
        return choice.item.attack;
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

DataStream & operator >> (DataStream &in, BattleChoice &po);
DataStream & operator << (DataStream &out, const BattleChoice &po);

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
        InvalidTier,

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
    quint8 team;
    bool rated;
    QString srctier, desttier;
    Pokemon::gen gen;

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

DataStream & operator >> (DataStream &in, ChallengeInfo &c);
DataStream & operator << (DataStream &out, const ChallengeInfo &c);

struct BattleConfiguration
{
    enum ReceivingMode {
        Spectator = 0,
        Player = 1
    };

    Pokemon::gen gen;
    quint8 mode;
    qint32 ids[2];
    quint32 clauses;
    quint8 receivingMode[2];
    quint16 avatar[2];

    /* Rated or not */
    Flags flags;
    bool oldconf; /* Will become obsolete. Used to store internally if the conf was read with oldDeserialize */

    enum Network {
        HasNumIds,
        HasItems
    };

    enum FlagEnum {
        Rated = 0
    };

    TeamBattle *teams[2];

    bool teamOwnership;

    int slot(int spot, int poke = 0) const  {
        return spot + poke*2;
    }

    int spot(int id) const {
        return ids[0] == id ? 0 : 1;
    }

    bool rated() const {
        return flags[Rated];
    }

    bool isPlayer(int slot) const {
        return receivingMode[slot] == Player;
    }

    bool isInBattle(int id) const {
        return ids[0] == id || ids[1] == id;
    }

    void setTeam(int i, TeamBattle *team) {
        teams[i] = team;
        receivingMode[i] = Player;
    }

    int numberOfSlots() const {
        if (mode == ChallengeInfo::Doubles) {
            return 4;
        } else if (mode == ChallengeInfo::Triples) {
            return 6;
        } else {
            return 2;
        }
    }

    BattleConfiguration() {
        oldconf = false;
        teamOwnership = false;
        receivingMode[0] = receivingMode[1] = Spectator;
    }

    explicit BattleConfiguration(const BattleConfiguration &other);
    BattleConfiguration& operator = (const BattleConfiguration& conf);
    ~BattleConfiguration();

    inline DataStream & oldDeserialize (DataStream &in) {
        in >> this->gen >> this->mode >> this->ids[0] >> this->ids[1] >> this->clauses;
        oldconf = true;

        return in;
    }

    inline DataStream & oldSerialize (DataStream &out) const
    {
        out << this->gen << this->mode << this->ids[0] << this->ids[1] << this->clauses;

        return out;
    }
};

inline DataStream & operator >> (DataStream &in, BattleConfiguration &c)
{
    c.oldconf = false;

    VersionControl v;
    in >> v;

    if (v.versionNumber != 0) {
        return in;
    }

    Flags network;
    v.stream >> network >> c.flags >> c.gen >> c.mode >> c.clauses >> c.ids[0] >> c.ids[1];

    return in;
}

inline DataStream & operator << (DataStream &out, const BattleConfiguration &c)
{
    VersionControl v;
    v.stream << Flags() << c.flags << c.gen << c.mode << c.clauses << c.ids[0] << c.ids[1];

    out << v;

    return out;
}

struct FullBattleConfiguration : public BattleConfiguration
{
    QString name[2];

    const QString getName(int player) const {return receivingMode[player] == Spectator ? name[player] : teams[player]->name;}

    FullBattleConfiguration& operator = (const BattleConfiguration& conf) {
        return * (FullBattleConfiguration*)(& (this->BattleConfiguration::operator = (conf)));
    }
};

DataStream & operator >> (DataStream &in, FullBattleConfiguration &c);
DataStream & operator << (DataStream &out, const FullBattleConfiguration &c);

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

inline DataStream & operator >> (DataStream &in, BattleDynamicInfo &c)
{
    in >> c.boosts[0] >> c.boosts[1] >> c.boosts[2] >> c.boosts[3] >> c.boosts[4] >> c.boosts[5] >> c.boosts[6] >> c.flags;

    return in;
}

inline DataStream & operator << (DataStream &out, const BattleDynamicInfo &c)
{
    out << c.boosts[0] << c.boosts[1] << c.boosts[2] << c.boosts[3] << c.boosts[4] << c.boosts[5] << c.boosts[6] << c.flags;

    return out;
}

struct BattleStats
{
    qint16 stats[5];
};

inline DataStream & operator >> (DataStream &in, BattleStats &c)
{
    in >> c.stats[0] >> c.stats[1] >> c.stats[2] >> c.stats[3] >> c.stats[4];

    return in;
}

inline DataStream & operator << (DataStream &out, const BattleStats &c)
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
    quint8 teams;
};

DataStream& operator >> (DataStream &in, FindBattleData &f);
DataStream& operator << (DataStream &out, const FindBattleData &f);

struct FindBattleDataAdv : public FindBattleData
{
    void shuffle(int total);

    QVector<quint8> shuffled;
};

#endif // BATTLESTRUCTS_H

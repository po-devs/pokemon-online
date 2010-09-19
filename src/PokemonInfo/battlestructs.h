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

QDataStream & operator >> (QDataStream &in, BattleMove &mo);
QDataStream & operator << (QDataStream &out, const BattleMove &mo);

class PokeBattle
{
    PROPERTY(QString, nick);
    PROPERTY(QList<int>, dvs);
    PROPERTY(QList<int>, evs);
    PROPERTY(quint16, lifePoints);
    PROPERTY(quint16, totalLifePoints);
    PROPERTY(Pokemon::uniqueId, num);
    PROPERTY(quint16, item);
    PROPERTY(quint16, ability);
    PROPERTY(quint32, fullStatus);
    PROPERTY(qint8, statusCount);
    PROPERTY(quint8, gender);
    PROPERTY(quint8, level);
    PROPERTY(quint8, nature);
    PROPERTY(quint8, happiness);
    PROPERTY(bool, shiny);
public:
    PokeBattle();

    void init(PokePersonal &poke);

    BattleMove& move(int i);
    const BattleMove& move(int i) const;

    quint16 normalStat(int stat) const;
    void updateStats();

    bool ko() const {return lifePoints() == 0 || num() == Pokemon::NoPoke || status() == Pokemon::Koed;}
    bool isFull() const { return lifePoints() == totalLifePoints(); }
    int lifePercent() const { return lifePoints() == 0 ? 0 : std::max(1, lifePoints()*100/totalLifePoints());}

    void setNormalStat(int, quint16);
    int status() const;
    void changeStatus(int status);
private:
    BattleMove m_moves[4];

    quint16 normal_stats[5];
};

QDataStream & operator >> (QDataStream &in, PokeBattle &po);
QDataStream & operator << (QDataStream &out, const PokeBattle &po);

/* A pokemon as viewed by the opponent: nearly no info */
class ShallowBattlePoke
{
    PROPERTY(QString, nick);
    PROPERTY(quint32, fullStatus);
    PROPERTY(Pokemon::uniqueId, num);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, gender);
    PROPERTY(quint8, lifePercent);
    PROPERTY(quint8, level);
public:
    ShallowBattlePoke();
    ShallowBattlePoke(const PokeBattle &poke);

    int status() const;
    void changeStatus(int status);

    void init(const PokeBattle &poke);
};

QDataStream & operator >> (QDataStream &in, ShallowBattlePoke &po);
QDataStream & operator << (QDataStream &out, const ShallowBattlePoke &po);

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

    bool invalid() const;

    QString name;
    QString info;
    int gen;
private:
    PokeBattle m_pokemons[6];
};

QDataStream & operator >> (QDataStream &in, TeamBattle &te);
QDataStream & operator << (QDataStream &out, const TeamBattle &te);

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

struct BattleChoice
{
    static const int Cancel = -10;

    BattleChoice(bool pokeSwitch = false, qint8 numSwitch = 0, quint8 numslot=0, quint8 target=0);

    bool pokeSwitch; /* True if poke switch, false if attack switch */
    qint8 numSwitch; /* The num of the poke or the attack to use, -1 for Struggle, -10 for move cancelled */
    quint8 targetPoke; /* The targetted pokemon */
    quint8 numSlot;

    /* returns true if the choice is valid */
    bool match(const BattleChoices &avail) const;
    int  getChoice() const { return numSwitch; }
    bool attack() const { return !pokeSwitch; }
    bool poke() const { return pokeSwitch; }
    bool cancelled() const { return numSwitch == Cancel; }
    int target() const {return targetPoke; }
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
        EvasionClause = 4,
        OHKOClause = 8,
        DisallowSpectator = 16,
        ItemClause = 32,
        ChallengeCup = 64,
        NoTimeOut = 128,
        SpeciesClause = 256
    };

    enum Mode
    {
        Singles,
        Doubles
    };

    static const int numberOfClauses = 9;

    static QString clauseText[numberOfClauses];
    static QString clauseBattleText[numberOfClauses];
    static QString clauseDescription[numberOfClauses];

    static QString clause(int index) {
        return index >= 0 && index < numberOfClauses ? clauseText[index] : "";
    }
    
    static QString battleText(int index) {
        return index >= 0 && index < numberOfClauses ? clauseBattleText[index] : "";
    }

    static QString description(int index) {
        return index >= 0 && index < numberOfClauses ? clauseDescription[index] : "";
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
    bool doubles;
    qint32 ids[2];

    int slot(int spot, int poke = 0) const  {
        return spot + poke*2;
    }

    int spot(int id) const {
        return ids[0] == id ? 0 : 1;
    }
};

inline QDataStream & operator >> (QDataStream &in, BattleConfiguration &c)
{
    in >> c.gen >> c.doubles >> c.ids[0] >> c.ids[1];

    return in;
}

inline QDataStream & operator << (QDataStream &out, const BattleConfiguration &c)
{
    out << c.gen << c.doubles << c.ids[0] << c.ids[1];

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

#ifndef BATTLESTRUCTS_H
#define BATTLESTRUCTS_H

/* damage formula: http://www.smogon.com/dp/articles/damage_formula */

#include <QtCore>
#include "../Utilities/functions.h"

class TeamInfo;
class PokePersonal;

namespace Pokemon
{
    enum Status
    {
	Koed = -2,
	Fine=0,
	Paralysed=1,
	Burnt=2,
	Frozen=3,
	Asleep=4,
	Poisoned=5,
	DeeplyPoisoned=6
    };
}

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
    PROPERTY(quint8, power);
    PROPERTY(quint8, type);
    PROPERTY(quint16, num);
public:
    BattleMove();

    void load();
    operator int () {return num();}
};

QDataStream & operator >> (QDataStream &in, BattleMove &mo);
QDataStream & operator << (QDataStream &out, const BattleMove &mo);

class PokeBattle
{
    PROPERTY(QString, nick);
    PROPERTY(quint16, lifePoints);
    PROPERTY(quint16, totalLifePoints);
    PROPERTY(qint8, status);
    PROPERTY(qint8, sleepCount);
    PROPERTY(quint16, num);
    PROPERTY(quint16, item);
    PROPERTY(quint16, ability);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, gender);
    PROPERTY(quint8, level);
    PROPERTY(QList<int>, dvs);
public:
    PokeBattle();

    void init(const PokePersonal &poke);

    BattleMove& move(int i);
    const BattleMove& move(int i) const;

    quint16 normalStat(int stat) const;

    bool ko() const {return lifePoints() == 0 || num() == 0 || status() == Pokemon::Koed;}
    bool isFull() const { return lifePoints() == totalLifePoints(); }
    int lifePercent() const { return lifePoints()*100/totalLifePoints();}

    void setNormalStat(int, quint16);
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
    PROPERTY(qint8, status);
    PROPERTY(quint16, num);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, gender);
    PROPERTY(quint8, lifePercent);
    PROPERTY(quint8, level);
public:
    ShallowBattlePoke();
    ShallowBattlePoke(const PokeBattle &poke);

    void init(const PokeBattle &poke);
};

QDataStream & operator >> (QDataStream &in, ShallowBattlePoke &po);
QDataStream & operator << (QDataStream &out, const ShallowBattlePoke &po);

class TeamBattle
{
public:
    TeamBattle();
    /* removes the invalid pokemons */
    TeamBattle(const TeamInfo &other);
    void init(const TeamInfo &other);

    PokeBattle& poke(int i);
    const PokeBattle& poke(int i) const;

    bool invalid() const;

    QString name;
    QString info;
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

    bool struggle() const { return qFind(attackAllowed, attackAllowed+4, true) == attackAllowed+4; }

    static BattleChoices SwitchOnly();
};

QDataStream & operator >> (QDataStream &in, BattleChoices &po);
QDataStream & operator << (QDataStream &out, const BattleChoices &po);

struct BattleChoice
{
    static const int Cancel = -10;

    BattleChoice(bool pokeSwitch = false, qint8 numSwitch = 0);

    bool pokeSwitch; /* True if poke switch, false if attack switch */
    qint8 numSwitch; /* The num of the poke or the attack to use, -1 for Struggle, -10 for move cancelled */

    /* returns true if the choice is valid */
    bool match(const BattleChoices &avail) const;
    bool attack() const { return !pokeSwitch; }
    bool poke() const { return pokeSwitch; }
    bool cancelled() const { return numSwitch == Cancel; }
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

        ChallengeDescLast
    };

    bool sleepClauseEnabled;
    qint8 dsc;
    qint32 opp;

    explicit ChallengeInfo(int desc=0, int opponent=0, bool sleepClauseEnabled = true)
        : sleepClauseEnabled(sleepClauseEnabled), dsc(desc), opp(opponent)
    {
    }

    int opponent() const {return opp;}
    qint8 desc() const { return dsc;}
    bool sleepClause() const {return sleepClauseEnabled;}

    operator int () const {
        return opponent();
    }
};

QDataStream & operator >> (QDataStream &in, ChallengeInfo &c);
QDataStream & operator << (QDataStream &out, const ChallengeInfo &c);

struct BattleConfiguration
{
    qint32 ids[2];
};

inline QDataStream & operator >> (QDataStream &in, BattleConfiguration &c)
{
    in >> c.ids[0] >> c.ids[1];

    return in;
}

inline QDataStream & operator << (QDataStream &out, const BattleConfiguration &c)
{
    out << c.ids[0] << c.ids[1];

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


#endif // BATTLESTRUCTS_H

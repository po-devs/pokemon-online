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
    Tie
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
};

QDataStream & operator >> (QDataStream &in, BattleMove &mo);
QDataStream & operator << (QDataStream &out, const BattleMove &mo);

class PokeBattle
{
    PROPERTY(QString, nick);
    PROPERTY(bool, confused);
    PROPERTY(quint16, lifePoints);
    PROPERTY(quint16, totalLifePoints);
    PROPERTY(quint8, status);
    PROPERTY(quint16, num);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, gender);
    PROPERTY(quint8, level);
public:
    PokeBattle();

    void init(const PokePersonal &poke);

    BattleMove& move(int i);
    const BattleMove& move(int i) const;

    quint16 normalStat(int stat) const;
    quint8 statMod(int stat) const;

    bool ko() const {return lifePoints() == 0;}

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
    PROPERTY(quint8, status);
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
    BattleChoice(bool pokeSwitch = false, qint8 numSwitch = 0);

    bool pokeSwitch; /* True if poke switch, false if attack switch */
    qint8 numSwitch; /* The num of the poke or the attack to use, -1 for Struggle */

    /* returns true if the choice is valid */
    bool match(const BattleChoices &avail) const;
    bool attack() const { return !pokeSwitch; }
    bool poke() const { return pokeSwitch; }
};

QDataStream & operator >> (QDataStream &in, BattleChoice &po);
QDataStream & operator << (QDataStream &out, const BattleChoice &po);

#endif // BATTLESTRUCTS_H

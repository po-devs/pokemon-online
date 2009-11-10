#ifndef BATTLESTRUCTS_H
#define BATTLESTRUCTS_H

/* damage formula: http://www.smogon.com/dp/articles/damage_formula */

#include <QtCore>

namespace Pokemon
{
    enum Status
    {
	Fine,
	Paralysed,
	Burnt,
	Asleep,
	Poisoned,
	DeeplyPoisoned
    };
}

class BattleMove
{
public:
    BattleMove();

    quint8 PP() const;
    quint8 totalPP() const;
    quint8 power() const;
    quint8 type() const;
    quint16 num() const;

    void setPP(quint8);
    void setTotalPP(quint8);
    void setPower(quint8);
    void setType(quint8);
    void setNum(quint16);

private:
    quint8 m_PP;
    quint8 m_totalPP;
    quint8 m_power; /* 0 for no power, quint8(-1) for unknown (ie not fixed) power, otherwise the normal power */
    quint16 m_num;
    quint8 m_type;
};

class PokeBattle
{
public:
    PokeBattle();

    BattleMove& move(int i);
    const BattleMove& move(int i) const;

    void setConfused(bool);
    void setStatus(quint8);
    void setLifePoints(quint16);
    void setStatMod(int, qint8);
    void setNum(quint16);
    void setTotalLifePoints(quint16);
    void resetStatMods();
private:
    BattleMove m_moves[4];

    bool m_confused;
    quint8 m_status;
    quint16 m_num;
    qint8 stat_mods[5]; /* -6, -5, -4, ..., 0, 1, 2, ..., 6 */
    quint16 m_lifepoints;
    quint16 m_totalLifepoints;
};

class TeamBattle
{
public:
    TeamBattle();

    PokeBattle& poke(int i);
    const PokeBattle& poke(int i) const;
private:
    PokeBattle m_pokemons[6];
};

#endif // BATTLESTRUCTS_H

#include "battlestructs.h"
#include "pokemoninfo.h"
#include "networkstructs.h"

BattleMove::BattleMove()
{
    num() = 0;
    type() = 0;
    power() = 0;
    PP() = 0;
    totalPP() = 0;
}

void BattleMove::load() {
    power() = MoveInfo::Power(num());
    PP() = MoveInfo::PP(num());
    totalPP() = PP();
    type() = MoveInfo::Type(num());
}

QDataStream & operator >> (QDataStream &in, BattleMove &mo)
{
    in >> mo.num() >> mo.type() >> mo.power() >> mo.PP() >> mo.totalPP();

    return in;
}

QDataStream & operator << (QDataStream &out, const BattleMove &mo)
{
    out << mo.num() << mo.PP() << mo.totalPP() << mo.power() << mo.type();

    return out;
}

PokeBattle::PokeBattle()
{
    num() =0;
    confused() = false;
    status() = Pokemon::Fine;
    lifePoints() = 0;
    totalLifePoints() = 0;

    resetStatMods();
}

const BattleMove & PokeBattle::move(int i) const
{
    return m_moves[i];
}

BattleMove & PokeBattle::move(int i)
{
    return m_moves[i];
}

quint16 PokeBattle::normalStat(int stat) const
{
    return normal_stats[stat-1];
}

void PokeBattle::setNormalStat(int stat, quint16 i)
{
    normal_stats[stat-1] = i;
}

void PokeBattle::setStatMod(int stat, qint8 mod)
{
    stat_mods[stat-1] = mod;
}

void PokeBattle::resetStatMods()
{
    for (int i = Attack; i <= SpDefense; i++) {
	setStatMod(i, 0);
    }
}


void PokeBattle::init(const PokePersonal &poke)
{
    PokeGeneral p;
    p.num() = poke.num();
    p.load();

    num() = poke.num();
    nick() = poke.nickname();
    gender() = poke.gender();
    shiny() = poke.shiny();

    for (int i = 0; i < 4; i++) {
	move(i).num() = poke.move(i);
	move(i).load();
    }

    totalLifePoints() = PokemonInfo::FullStat(poke.nature(), Hp, p.baseStats().baseHp(), poke.level(), poke.hpDV(), poke.hpEV());
    lifePoints() = totalLifePoints();

    for (int i = 0; i < 5; i++) {
	normal_stats[i] = PokemonInfo::FullStat(poke.nature(), i+1, p.baseStats().baseStat(i+1), poke.level(), poke.DV(i+1), poke.EV(i+1));
    }
}

QDataStream & operator >> (QDataStream &in, PokeBattle &po)
{
    in >> po.num() >> po.nick() >> po.totalLifePoints() >> po.lifePoints() >> po.gender() >> po.shiny();

    for (int i = 0; i < 5; i++) {
	quint16 st;
	in >> st;
	po.setNormalStat(i+1, st);
    }

    for (int i = 0; i < 4; i++) {
	in >> po.move(i);
    }

    return in;
}

QDataStream & operator << (QDataStream &out, const PokeBattle &po)
{
    out << po.num() << po.nick() << po.totalLifePoints() << po.lifePoints() << po.gender() << po.shiny();

    for (int i = 0; i < 5; i++) {
	out << po.normalStat(i+1);
    }

    for (int i = 0; i < 4; i++) {
	out << po.move(i);
    }

    return out;
}

TeamBattle::TeamBattle()
{
}

TeamBattle::TeamBattle(const TeamInfo &other)
{
    for (int i = 0; i < 6; i++) {
	poke(i).init(other.pokemon(i));
    }
}

PokeBattle & TeamBattle::poke(int i)
{
    return m_pokemons[i];
}

const PokeBattle & TeamBattle::poke(int i) const
{
    return m_pokemons[i];
}

QDataStream & operator >> (QDataStream &in, TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
	in >> te.poke(i);
    }

    return in;
}

QDataStream & operator << (QDataStream &out, const TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
	out << te.poke(i);
    }

    return out;
}

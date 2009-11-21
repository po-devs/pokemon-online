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
    out << mo.num() << mo.type() << mo.power() << mo.PP() << mo.totalPP();

    return out;
}

PokeBattle::PokeBattle()
{
    num() =0;
    confused() = false;
    status() = Pokemon::Fine;
    lifePoints() = 0;
    totalLifePoints() = 0;
    level() = 100;
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

void PokeBattle::init(const PokePersonal &poke)
{
    PokeGeneral p;
    p.num() = poke.num();
    p.load();

    num() = poke.num();
    nick() = poke.nickname();
    gender() = poke.gender();
    shiny() = poke.shiny();
    level() = poke.level();

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
    in >> po.num() >> po.nick() >> po.totalLifePoints() >> po.lifePoints() >> po.gender() >> po.shiny() >> po.level();

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
    out << po.num() << po.nick() << po.totalLifePoints() << po.lifePoints() << po.gender() << po.shiny() << po.level();

    for (int i = 0; i < 5; i++) {
	out << po.normalStat(i+1);
    }

    for (int i = 0; i < 4; i++) {
	out << po.move(i);
    }

    return out;
}

ShallowBattlePoke::ShallowBattlePoke()
{
}

ShallowBattlePoke::ShallowBattlePoke(const PokeBattle &p)
{
    init(p);
}

void ShallowBattlePoke::init(const PokeBattle &poke)
{
    nick() = poke.nick();
    status() = poke.status();
    num() = poke.num();
    shiny() = poke.shiny();
    gender() = poke.gender();
    lifePercent() = (poke.lifePoints() * 100) / poke.totalLifePoints();
    level() = poke.level();
}

QDataStream & operator >> (QDataStream &in, ShallowBattlePoke &po)
{
    in >> po.num() >> po.nick() >> po.lifePercent() >> po.status() >> po.gender() >> po.shiny() >> po.level();

    return in;
}

QDataStream & operator << (QDataStream &out, const ShallowBattlePoke &po)
{
    out << po.num() << po.nick() << po.lifePercent() << po.status() << po.gender() << po.shiny() << po.level();

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

BattleChoices::BattleChoices()
{
    switchAllowed = true;
    attacksAllowed = true;
    std::fill(attackAllowed, attackAllowed+4, true);
}

void BattleChoices::disableSwitch()
{
    switchAllowed = false;
}

void BattleChoices::disableAttack(int attack)
{
    attackAllowed[attack] = false;
}

void BattleChoices::disableAttacks()
{
    std::fill(attackAllowed, attackAllowed+4, false);
}

BattleChoices BattleChoices::SwitchOnly()
{
    BattleChoices ret;
    ret.disableAttacks();

    return ret;
}

QDataStream & operator >> (QDataStream &in, BattleChoices &po)
{
    in >> po.switchAllowed >> po.attacksAllowed >> po.attackAllowed[0] >> po.attackAllowed[1] >> po.attackAllowed[2] >> po.attackAllowed[3];
    return in;
}

QDataStream & operator << (QDataStream &out, const BattleChoices &po)
{
    out << po.switchAllowed << po.attacksAllowed << po.attackAllowed[0] << po.attackAllowed[1] << po.attackAllowed[2] << po.attackAllowed[3];
    return out;
}

BattleChoice::BattleChoice(bool pokeswitch, qint8 numswitch)
	: pokeSwitch(pokeswitch), numSwitch(numswitch)
{
}

/* Tests if the attack chosen is allowed */
bool BattleChoice::match(const BattleChoices &avail) const
{
    if (!avail.attacksAllowed && attack()) {
	return false;
    }
    if (!avail.switchAllowed && poke()) {
	return false;
    }

    if (attack()) {
	if (avail.struggle() != (numSwitch == -1))
	    return false;
	if (!avail.struggle()) {
	    if (numSwitch < 0 || numSwitch > 3) {
		//Crash attempt!!
		return false;
	    }
	    return avail.attackAllowed[numSwitch];
	}
	return true;
    }
    if (poke()) {
	if (numSwitch < 0 || numSwitch > 5) {
	    //Crash attempt!!
	    return false;
	}
	return true;
    }
    //Never reached
    return true; // but avoids warnings anyway
}

QDataStream & operator >> (QDataStream &in, BattleChoice &po)
{
    in >> po.pokeSwitch >> po.numSwitch;
    return in;
}

QDataStream & operator << (QDataStream &out, const BattleChoice &po)
{
    out << po.pokeSwitch << po.numSwitch;
    return out;
}

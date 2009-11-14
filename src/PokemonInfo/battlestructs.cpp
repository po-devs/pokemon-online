#include "battlestructs.h"
#include "pokemoninfo.h"
#include "networkstructs.h"

BattleMove::BattleMove()
{
    setNum(0);
    setType(0);
    setPower(0);
    setPP(0);
    setTotalPP(0);
}

quint16 BattleMove::num() const {
    return m_num;
}

quint8 BattleMove::power() const {
    return m_power;
}

quint8 BattleMove::PP() const {
    return m_PP;
}

quint8 BattleMove::type() const {
    return m_type;
}

quint8 BattleMove::totalPP() const {
    return m_totalPP;
}

void BattleMove::setNum(quint16 num) {
    m_num = num;
}

void BattleMove::setPower(quint8 power) {
    m_power = power;
}

void BattleMove::setPP(quint8 pp) {
    m_PP = pp;
}

void BattleMove::setType(quint8 type) {
    m_type = type;
}

void BattleMove::setTotalPP(quint8 totalPP) {
    m_totalPP = totalPP;
}

void BattleMove::load() {
    setPower(MoveInfo::Power(num()));
    setPP(MoveInfo::PP(num()));
    setTotalPP(PP());
    setType(MoveInfo::Type(num()));
}

QDataStream & operator >> (QDataStream &in, BattleMove &mo)
{
    quint16 num;
    quint8 totalPP,PP,power,type;

    in >> num >> PP >> totalPP >> power >> type;

    mo.setNum(num);
    mo.setTotalPP(totalPP);
    mo.setPP(PP);
    mo.setPower(power);
    mo.setType(type);

    return in;
}

QDataStream & operator << (QDataStream &out, const BattleMove &mo)
{
    out << mo.num() << mo.PP() << mo.totalPP() << mo.power() << mo.type();

    return out;
}

PokeBattle::PokeBattle()
{
    setNum(0);
    resetStatMods();
    setConfused(false);
    setStatus(Pokemon::Fine);
    setLifePoints(0);
    setTotalLifePoints(0);
}

void PokeBattle::setConfused(bool confused) {
    m_confused = confused;
}

const BattleMove & PokeBattle::move(int i) const
{
    return m_moves[i];
}

BattleMove & PokeBattle::move(int i)
{
    return m_moves[i];
}

quint16 PokeBattle::num() const
{
    return m_num;
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

void PokeBattle::setLifePoints(quint16 lifePoints)
{
    m_lifepoints = lifePoints;
}

void PokeBattle::setTotalLifePoints(quint16 totalLifePoints)
{
    m_totalLifepoints = totalLifePoints;
}

void PokeBattle::setStatus(quint8 status)
{
    m_status = status;
}

void PokeBattle::setNum(quint16 num) {
    m_num = num;
}

void PokeBattle::resetStatMods()
{
    for (int i = Attack; i <= SpDefense; i++) {
	setStatMod(i, 0);
    }
}

quint16 PokeBattle::lifePoints() const
{
    return m_lifepoints;
}

quint16 PokeBattle::totalLifePoints() const
{
    return m_totalLifepoints;
}

void PokeBattle::init(const PokePersonal &poke)
{
    PokeGeneral p;
    p.setNum(poke.num());
    p.load();

    for (int i = 0; i < 4; i++) {
	move(i).setNum(poke.move(i));
	move(i).load();
    }
    m_nick = poke.nickname();

    setTotalLifePoints(PokemonInfo::FullStat(poke.nature(), Hp, p.baseStats().baseHp(), poke.level(), poke.hpDV(), poke.hpEV()));
    setLifePoints(totalLifePoints());

    for (int i = 0; i < 5; i++) {
	normal_stats[i] = PokemonInfo::FullStat(poke.nature(), i+1, p.baseStats().baseStat(i+1), poke.level(), poke.DV(i+1), poke.EV(i+1));
    }
}

QDataStream & operator >> (QDataStream &in, PokeBattle &po)
{
    quint16 num,lifePoints;

    in >> num >> lifePoints;

    po.setNum(num);
    po.setTotalLifePoints(lifePoints);
    po.setLifePoints(lifePoints);

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
    out << po.num() << po.totalLifePoints();

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
    for (int i = 0; i < 4; i++) {
	in >> te.poke(i);
    }

    return in;
}

QDataStream & operator << (QDataStream &out, const TeamBattle &te)
{
    for (int i = 0; i < 4; i++) {
	out << te.poke(i);
    }

    return out;
}

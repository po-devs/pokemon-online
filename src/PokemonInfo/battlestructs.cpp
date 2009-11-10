#include "battlestructs.h"
#include "pokemoninfo.h"

BattleMove::BattleMove()
{
    setNum(0);
    setType(0);
    setPower(0);
    setPP(0);
    setTotalPP(0);
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

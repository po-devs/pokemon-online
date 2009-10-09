#include "pokemonstructs.h"


PokeBaseStats::PokeBaseStats(quint8 base_hp, quint8 base_att, quint8 base_def, quint8 base_spd, quint8 base_spAtt, quint8 base_spDef)
{
    setBaseHp(base_hp);
    setBaseAttack(base_att);
    setBaseDefense(base_def);
    setBaseSpeed(base_spd);
    setBaseSpAttack(base_spAtt);
    setBaseSpDefense(base_spDef);
}

quint8 PokeBaseStats::baseHp() const
{
    return m_BaseStats[0];
}

quint8 PokeBaseStats::baseAttack() const
{
    return m_BaseStats[1];
}

quint8 PokeBaseStats::baseDefense() const
{
    return m_BaseStats[2];
}

quint8 PokeBaseStats::baseSpeed() const
{
    return m_BaseStats[3];
}

quint8 PokeBaseStats::baseSpAttack() const
{
    return m_BaseStats[4];
}

quint8 PokeBaseStats::baseSpDefense() const
{
    return m_BaseStats[5];
}


void PokeBaseStats::setBaseHp(quint8 hp)
{
    m_BaseStats[0] = hp;
}

void PokeBaseStats::setBaseAttack(quint8 att)
{
    m_BaseStats[1] = att;
}

void PokeBaseStats::setBaseDefense(quint8 def)
{
    m_BaseStats[2] = def;
}

void PokeBaseStats::setBaseSpeed(quint8 speed)
{
    m_BaseStats[3] = speed;
}

void PokeBaseStats::setBaseSpAttack(quint8 spAtt)
{
    m_BaseStats[4] = spAtt;
}

void PokeBaseStats::setBaseSpDefense(quint8 spDef)
{
    m_BaseStats[5] = spDef;
}

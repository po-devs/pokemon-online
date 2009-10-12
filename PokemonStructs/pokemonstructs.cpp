#include "pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"

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

PokeGeneral::PokeGeneral()
	: m_num(0)
{
}

void PokeGeneral::setNum(int num)
{
    m_num = num;
}

int PokeGeneral::num() const
{
    return m_num;
}

void PokeGeneral::loadBaseStats()
{
    setBaseStats(PokemonInfo::BaseStats(num()));
}

void PokeGeneral::loadMoves()
{
    m_moves = PokemonInfo::Moves(num());
}

void PokeGeneral::loadTypes()
{
    ;
}

void PokeGeneral::loadAbilities()
{
    m_abilities = PokemonInfo::Abilities(num());
}

void PokeGeneral::load()
{
    loadBaseStats();
    loadMoves();
    loadTypes();
    loadAbilities();
}

const QList<int> &PokeGeneral::moves() const
{
    return m_moves;
}

void PokeGeneral::setBaseStats(const PokeBaseStats &stats)
{
    m_stats = stats;
}

PokePersonal::PokePersonal()
	: m_num(0)
{
}

void PokePersonal::setNum(int num)
{
    m_num = num;
}

PokeGraphics::PokeGraphics()
	: m_num(0), m_uptodate(false)
{
}

void PokeGraphics::setNum(int num)
{
    m_num = num;
    setUpToDate(false);
}

void PokeGraphics::setUpToDate(bool uptodate)
{
    m_uptodate = uptodate;
}

bool PokeGraphics::upToDate()
{
    return m_uptodate;
}

void PokeGraphics::load(int gender, bool shiny)
{
    if (upToDate() && gender==m_storedgender && shiny == m_storedshininess)
	return;

    m_storedgender = gender;
    m_storedshininess = shiny;
    m_picture = PokemonInfo::Picture(num(), gender, shiny);
    setUpToDate(true);
}

QPixmap PokeGraphics::picture()
{
    return m_picture;
}

int PokeGraphics::num() const
{
    return m_num;
}

PokeTeam::PokeTeam()
{
    setNum(0);
}

void PokeTeam::setNum(int num)
{
    PokeGeneral::setNum(num);
    PokePersonal::setNum(num);
    PokeGraphics::setNum(num);
}

int PokeTeam::num() const
{
    return PokeGeneral::num();
}

void PokeTeam::load()
{
    PokeGeneral::load();
    PokeGraphics::load(0, 0);
}



Team::Team()
{
}

const PokeTeam & Team::poke(int index) const
{
    return m_pokes[index];
}

PokeTeam & Team::poke(int index)
{
    return m_pokes[index];
}



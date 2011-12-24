#include "../PokemonInfo/battlestructs.h"

#include "battledataaccessor.h"
#include "teamdata.h"
#include "battledata.h"

PokeProxy::PokeProxy() : hasOwnerShip(true), pokeData(new ShallowBattlePoke())
{
}

PokeProxy::PokeProxy(ShallowBattlePoke *pokemon) : hasOwnerShip(false), pokeData(pokemon)
{
}

PokeProxy::~PokeProxy()
{
    if (hasOwnerShip) {
        delete pokeData;
    }
}

void PokeProxy::adaptTo(const ShallowBattlePoke *pokemon, bool soft) {
    const PokeBattle *trans = dynamic_cast<const PokeBattle*>(pokemon);
    if (trans) {
        adaptTo(trans);
    }
    if (*pokemon == *pokeData) {
        return;
    }
    if (soft) {
        pokeData->changeStatus(pokemon->status());
        return;
    }
    /* Could be more granular, change if it matters */
    *pokeData = *pokemon;
    emit numChanged(); emit statusChanged(); emit lifeChanged();
    emit pokemonReset();
}

void PokeProxy::adaptTo(const PokeBattle *pokemon) {
    PokeBattle *trans = dynamic_cast<PokeBattle*>(pokeData);

    if (trans) {
        *trans = *pokemon;
    } else {
        if (*pokeData == *(static_cast<const ShallowBattlePoke*>(pokemon))) {
            return;
        }
        *pokeData = *pokemon;
    }

    /* Could be more granular, change if it matters */
    emit numChanged(); emit statusChanged(); emit lifeChanged();
    emit pokemonReset();
}

void PokeProxy::changeStatus(int fullStatus)
{
    if (d()->status() == fullStatus) {
        return;
    }
    d()->changeStatus(fullStatus);
    emit statusChanged();
}

void PokeProxy::setLife(int newLife)
{
    if (d()->life() == newLife) {
        return;
    }
    d()->setLife(newLife);
    emit lifeChanged();
}

void PokeProxy::setNum(Pokemon::uniqueId num){
    if (d()->num() == num) {
        return;
    }
    d()->num() = num;
    emit numChanged();
}

int PokeProxy::basestat(int stat) const
{
    if (hasExposedData())
        return PokemonInfo::FullStat(num(), 5, nature(), stat, level(), dvs()[stat], evs()[stat]);
    else
        return 0;
}

TeamProxy::TeamProxy()
{
    teamData = new TeamData();
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(new PokeProxy(teamData->poke(i)));
    }
    hasOwnerShip = true;
}

TeamProxy::TeamProxy(TeamData *teamData) : teamData(teamData), hasOwnerShip(false)
{
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(new PokeProxy(teamData->poke(i)));
    }
    hasOwnerShip = false;
}

TeamProxy::~TeamProxy()
{
    foreach(PokeProxy *item, pokemons) {
        delete item;
    }
    if (hasOwnerShip) {
        delete teamData;
    }
}

QString TeamProxy::name() const {
    return teamData->name();
}

void TeamProxy::switchPokemons(int index, int prevIndex)
{
    std::swap(pokemons[index], pokemons[prevIndex]);
    teamData->switchPokemons(index, prevIndex);

    emit pokemonsSwapped(index, prevIndex);
}

void TeamProxy::setPoke(int index, ShallowBattlePoke *pokemon, bool soft)
{
    poke(index)->adaptTo(pokemon, soft);
}

void TeamProxy::setName(const QString &name)
{
    teamData->name() = name;
}

void TeamProxy::setTeam(const TeamBattle *team)
{
    for (int i = 0; i < 6; i++) {
        poke(i)->adaptTo(&team->poke(i));
    }
}

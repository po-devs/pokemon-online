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

void PokeProxy::adaptTo(ShallowBattlePoke *pokemon) {
    if (*pokemon == *pokeData) {
        return;
    }
    /* Could be more granular, change if it matters */
    *pokeData = *pokemon;
    emit numChanged(); emit statusChanged(); emit pokemonReset();
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

QString TeamProxy::name() {
    return teamData->name();
}

void TeamProxy::switchPokemons(int index, int prevIndex)
{
    std::swap(pokemons[index], pokemons[prevIndex]);
    teamData->switchPokemons(index, prevIndex);

    emit pokemonsSwapped(index, prevIndex);
}

void TeamProxy::setPoke(int index, ShallowBattlePoke *pokemon)
{
    poke(index)->adaptTo(pokemon);
}

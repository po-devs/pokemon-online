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

void PokeProxy::adaptTo(const ShallowBattlePoke *pokemon) {
    if (*pokemon == *pokeData) {
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

        /* Could be more granular, change if it matters */
        emit numChanged(); emit statusChanged(); emit lifeChanged();
        emit pokemonReset();
    } else {
        adaptTo((ShallowBattlePoke*)pokemon);
    }
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

quint16 TeamProxy::avatar() const {
    return teamData->avatar();
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

void TeamProxy::setName(const QString &name)
{
    teamData->name() = name;
}

void TeamProxy::setAvatar(int avatar)
{
    teamData->avatar() = avatar;
}

void TeamProxy::setTeam(const TeamBattle *team)
{
    for (int i = 0; i < 6; i++) {
        poke(i)->adaptTo(&team->poke(i));
    }
}

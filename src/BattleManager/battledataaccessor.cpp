#include "../PokemonInfo/battlestructs.h"

#include "battledataaccessor.h"
#include "teamdata.h"
#include "battledata.h"

PokeProxy::PokeProxy(ShallowBattlePoke *pokemon) : pokeData(pokemon)
{

}

QString PokeProxy::nickname() {
    return d()->nick();
}

bool PokeProxy::shiny() {
    return d()->shiny();
}

int PokeProxy::gender() {
    return d()->gender();
}

int PokeProxy::level() {
    return d()->level();
}

Pokemon::uniqueId PokeProxy::num() {
    return d()->num();
}

int PokeProxy::status() {
    return d()->status();
}

TeamProxy::TeamProxy(TeamData *teamData) : teamData(teamData)
{
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(new PokeProxy(&teamData->poke(i)));
    }
}

QString TeamProxy::name() {
    return teamData->name();
}

TeamProxy::~TeamProxy()
{
    foreach(PokeProxy *item, pokemons) {
        delete item;
    }
}

BattleDataProxy::BattleDataProxy(BattleData *battleData) : battleData(battleData)
{
    /* Needed for QML use */
    if (QMetaType::type("pokeid") == 0) {
        qRegisterMetaType<Pokemon::uniqueId>("pokeid");
    }

    for (int i = 0; i < 2; i++) {
        teams.push_back(new TeamProxy(&battleData->team(i)));
    }
}

BattleDataProxy::~BattleDataProxy()
{
    foreach(TeamProxy* team, teams) {
        delete team;
    }
}

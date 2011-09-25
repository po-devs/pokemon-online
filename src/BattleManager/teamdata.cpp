#include "teamdata.h"

TeamData::TeamData(bool player)
{
    if (!player) {
        for (int i = 0; i < 6; i++) {
            pokemons.push_back(std::shared_ptr<ShallowBattlePoke>(new ShallowBattlePoke()));
        }
    } else {
        for (int i = 0; i < 6; i++) {
            pokemons.push_back(std::shared_ptr<ShallowBattlePoke>(new PokeBattle()));
        }
    }
}

ShallowBattlePoke* TeamData::poke(int slot)
{
    return pokemons[slot].get();
}

QString &TeamData::name()
{
    return mName;
}

void TeamData::setPoke(int slot, ShallowBattlePoke *poke)
{
    *pokemons[slot] = *poke;
}

void TeamData::switchPokemons(int slot1, int slot2)
{
    pokemons[slot1].swap(pokemons[slot2]);
}

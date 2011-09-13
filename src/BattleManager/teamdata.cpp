#include "teamdata.h"

TeamData::TeamData() : _init(false)
{

}

ShallowBattlePoke* TeamData::poke(int slot)
{
    if (!_init) {
        init();
    }
    return pokemons[slot].get();
}

void TeamData::init()
{
    _init = true;
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(std::shared_ptr<ShallowBattlePoke>(new ShallowBattlePoke()));
    }
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

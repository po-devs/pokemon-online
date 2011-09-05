#include "teamdata.h"

TeamData::TeamData() : _init(false)
{

}

ShallowBattlePoke& TeamData::poke(int slot)
{
    if (!_init) {
        init();
    }
    return *pokemons[slot];
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

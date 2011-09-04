#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"

#include <memory>

class TeamData
{
    TeamData();
    ShallowBattlePoke& poke(int slot);
protected:
    void init();

    bool _init;
    std::vector<std::shared_ptr<ShallowBattlePoke> > pokemons;
};

#endif // TEAMDATA_H

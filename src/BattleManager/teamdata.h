#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"
#include <memory>

class TeamData
{
    ShallowBattlePoke& poke(int slot);
protected:
    void init();

    bool _init = false;
    std::vector<std::shared_pointer<ShallowBattlePoke> > pokemons;
};

#endif // TEAMDATA_H

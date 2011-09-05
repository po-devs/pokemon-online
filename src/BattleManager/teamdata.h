#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"

#include <memory>

class TeamData
{
public:
    TeamData();
    ShallowBattlePoke& poke(int slot);
    QString& name();
protected:
    void init();

    bool _init;
    std::vector<std::shared_ptr<ShallowBattlePoke> > pokemons;
    QString mName;
};

#endif // TEAMDATA_H

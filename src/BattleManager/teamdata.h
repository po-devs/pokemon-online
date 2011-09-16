#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"

#include <memory>

class TeamData
{
public:
    TeamData();
    ShallowBattlePoke* poke(int slot);
    QString& name();

    void setPoke(int slot, ShallowBattlePoke* poke);
    void switchPokemons(int slot1, int slot2);
protected:
    std::vector<std::shared_ptr<ShallowBattlePoke> > pokemons;
    QString mName;
};

#endif // TEAMDATA_H

#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"

#include <memory>

class TeamData
{
public:
    TeamData(const TeamBattle* team=NULL);
    ~TeamData();

    ShallowBattlePoke* poke(int slot);
    QString& name();
    QString name() const;
    Pokemon::gen gen() const {return mGen;}

    void setPoke(int slot, const ShallowBattlePoke* poke);
    void setPoke(int slot, const PokeBattle* poke);
    void switchPokemons(int slot1, int slot2);
    void setTeam(const TeamBattle *team);
    void setGen(Pokemon::gen gen);
protected:
    std::vector< ShallowBattlePoke* > pokemons;
    QString mName;
    Pokemon::gen mGen;
};

#endif // TEAMDATA_H

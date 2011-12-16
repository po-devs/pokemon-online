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
    quint16& avatar();
    quint16 avatar() const;

    void setPoke(int slot, const ShallowBattlePoke* poke);
    void setPoke(int slot, const PokeBattle* poke);
    void switchPokemons(int slot1, int slot2);
    void setTeam(const TeamBattle *team);
protected:
    std::vector< ShallowBattlePoke* > pokemons;
    QString mName;
    quint16 mAvatar;
};

#endif // TEAMDATA_H

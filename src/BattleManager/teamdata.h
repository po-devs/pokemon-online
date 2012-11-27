#ifndef TEAMDATA_H
#define TEAMDATA_H

#include "../PokemonInfo/battlestructs.h"

#include <memory>

class TeamData
{
public:
    TeamData(const TeamBattle* team=NULL, bool fullPokemon=false);
    ~TeamData();

    ShallowBattlePoke* poke(int slot);
    QString& name();
    QString name() const;
    QHash<quint16, quint16> &items() { return mItems;}
    Pokemon::gen gen() const {return mGen;}

    void setPoke(int slot, const ShallowBattlePoke* poke);
    void setPoke(int slot, const PokeBattle* poke);
    void switchPokemons(int slot1, int slot2);
    void setTeam(const TeamBattle *team);
    void setItems(const QHash<quint16, quint16>&items);
    void removeItem(int item);
    void changeItemCount(int item, int count);
    void setGen(Pokemon::gen gen);
protected:
    std::vector< ShallowBattlePoke* > pokemons;
    QString mName;
    Pokemon::gen mGen;
    QHash<quint16, quint16> mItems;
};

#endif // TEAMDATA_H

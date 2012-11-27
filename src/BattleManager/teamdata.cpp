#include "teamdata.h"

TeamData::TeamData(const TeamBattle *team, bool fullPokemon)
{
    if (!team) {
        for (int i = 0; i < 6; i++) {
            pokemons.push_back(fullPokemon ? new PokeBattle() : new ShallowBattlePoke());
        }
    } else {
        for (int i = 0; i < 6; i++) {
            pokemons.push_back(new PokeBattle(team->poke(i)));
        }
        mName = team->name;
        setGen(team->gen);
        setItems(team->items);
    }
}

TeamData::~TeamData() {
    foreach(ShallowBattlePoke *poke, pokemons) {
        delete poke;
    }
}

ShallowBattlePoke* TeamData::poke(int slot)
{
    return pokemons[slot];
}

QString &TeamData::name()
{
    return mName;
}

QString TeamData::name() const
{
    return mName;
}

void TeamData::setPoke(int slot, const ShallowBattlePoke *poke)
{
    *pokemons[slot] = *poke;
}

void TeamData::setPoke(int slot, const PokeBattle *poke)
{
    PokeBattle* trans = dynamic_cast<PokeBattle*>(pokemons[slot]);

    if (!trans)  {
        *pokemons[slot] = *poke;
    } else {
        *trans = *poke;
    }
}

void TeamData::setTeam(const TeamBattle *team)
{
    for (int i = 0; i < 6; i++) {
        setPoke(i, &team->poke(i));
    }
    mName = team->name;
    setGen(team->gen);
    setItems(team->items);
}

void TeamData::setItems(const QHash<quint16, quint16> &items)
{
    mItems = items;
}

void TeamData::setGen(Pokemon::gen gen)
{
    mGen = gen;
}

void TeamData::switchPokemons(int slot1, int slot2)
{
    std::swap(pokemons[slot1],pokemons[slot2]);
}

void TeamData::removeItem(int item)
{
    mItems.remove(item);
}

void TeamData::changeItemCount(int item, int count)
{
    mItems[item] = count;
}

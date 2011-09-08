#include "battledata.h"

BattleData::BattleData()
{
    /* Inits as if triples. Maybe later, will have
      mode at instanciation and so will only allocate
      necessary number */
    auxdata.resize(6);
}

void BattleData::onKo(int spot)
{
    poke(spot).changeStatus(Pokemon::Koed);
}

void BattleData::onSendOut(int spot, int previndex, std::shared_ptr<ShallowBattlePoke> pokemon, bool)
{
    int player = this->player(spot);
    int slot = this->slotNum(spot);

    team(player).switchPokemons(slot, previndex);
    team(player).setPoke(slot, pokemon);

    fieldPoke(spot).onSendOut();
}

void BattleData::onSendBack(int spot)
{
    fieldPoke(spot).onSendBack();
}

TeamData &BattleData::team(int player)
{
    return teams[player];
}

QString BattleData::name(int player)
{
    return teams[this->player(player)].name();
}

int BattleData::player(int spot)
{
    return spot % 2;
}

ShallowBattlePoke &BattleData::poke(int player)
{
    return teams[this->player(player)].poke(slotNum(player));
}

int BattleData::slotNum(int player)
{
    return player/2;
}

AuxPokeData& BattleData::fieldPoke(int player)
{
    return  auxdata[player];
}

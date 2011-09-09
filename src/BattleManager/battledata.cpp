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

void BattleData::onHpChange(int spot, int newHp)
{
    poke(spot).lifePercent() = newHp;
}

void BattleData::onMajorStatusChange(int spot, int status, bool)
{
    //TODO: handle confusion better
    if (status != Pokemon::Confused) {
        poke(spot).changeStatus(status);
    }
}

void BattleData::onPokeballStatusChanged(int player, int poke, int status)
{
    if (status != Pokemon::Confused) {
        team(player).poke(poke).changeStatus(status);
    }
}

void BattleData::onSubstituteStatus(int spot, bool substitute)
{
    fieldPoke(spot).subsitute = substitute;
}

void BattleData::onPokemonVanish(int spot)
{
    fieldPoke(spot).showing = false;
}

void BattleData::onPokemonReappear(int spot)
{
    fieldPoke(spot).showing = true;
}

void BattleData::onSpriteChange(int spot, int newSprite)
{
    fieldPoke(spot).alternateSprite = newSprite;
}

void BattleData::onDefiniteFormeChange(int player, int poke, int newPoke)
{
    team(player).poke(poke).num() = newPoke;
}

void BattleData::onCosmeticFormeChange(int spot, int subforme)
{
    fieldPoke(spot).alternateSprite.subnum = subforme;
}

void BattleData::onShiftSpots(int player, int spot1, int spot2, bool)
{
    std::swap(fieldPoke(spot(player, spot1)), fieldPoke(spot(player, spot2)));
    team(player).switchPokemons(spot1, spot2);
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

int BattleData::opponent(int player)
{
    return (player+1) %2;
}

ShallowBattlePoke &BattleData::poke(int player)
{
    return teams[this->player(player)].poke(slotNum(player));
}

int BattleData::slotNum(int player)
{
    return player/2;
}

int BattleData::spot(int player, int slot)
 {
    return player + 2*slot;
}

AuxPokeData& BattleData::fieldPoke(int player)
{
    return  auxdata[player];
}

int BattleData::gen()
{
    return GEN_MAX;
}

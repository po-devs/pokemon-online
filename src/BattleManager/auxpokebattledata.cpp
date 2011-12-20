#include "auxpokebattledata.h"

AuxPokeData::AuxPokeData()
{
    showing = true;
    onTheField = false;
    substitute = false;
}

void AuxPokeData::onSendOut(ShallowBattlePoke*)
{
    showing = true;
    onTheField = true;
    substitute = false;
    alternateSprite = Pokemon::NoPoke;
}

void AuxPokeData::onSendBack()
{
    onTheField = false;
}

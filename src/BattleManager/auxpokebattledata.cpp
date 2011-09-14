#include "auxpokebattledata.h"

AuxPokeData::AuxPokeData()
{
    showing = true;
    onTheField = false;
    substitute = false;
}

void AuxPokeData::onSendOut()
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

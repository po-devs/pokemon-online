#include "auxpokebattledata.h"

AuxPokeData::AuxPokeData()
{
    showing = true;
    onTheField = false;
    subsitute = false;
}

void AuxPokeData::onSendOut()
{
    showing = true;
    onTheField = true;
    subsitute = false;
    alternateSprite = Pokemon::NoPoke;
}

void AuxPokeData::onSendBack()
{
    onTheField = false;
}

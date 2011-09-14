#include "auxpokedataproxy.h"

AuxPokeDataProxy::AuxPokeDataProxy()
{
    showing = true;
    onTheField = false;
    substitute = false;
}

void AuxPokeDataProxy::onSendOut()
{
    setShowing(true);
    setSubstitute(false);
    setAlternateSprite(Pokemon::NoPoke);
    setOnTheField(true);
}

void AuxPokeDataProxy::onSendBack()
{
    setOnTheField(false);
}

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


FieldProxy::FieldProxy() {
    /* Resizes for triple. Later, when loaded with battle configuration, will get
              more accurate loading */
    for (int i = 0; i < 6; i++) {
        AuxPokeDataProxy *ptr = new AuxPokeDataProxy();
        auxdata.push_back(ptr);
    }
}

FieldProxy::~FieldProxy() {
    for (int i = 0; i < 6; i++) {
        delete auxdata[i];
    }
}

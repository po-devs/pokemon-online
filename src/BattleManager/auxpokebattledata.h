#ifndef SLOTBATTLEDATA_H
#define SLOTBATTLEDATA_H

#include "../PokemonInfo/pokemonstructs.h"

struct AuxPokeData
{
    AuxPokeData();

    void onSendOut();
    void onSendBack();

    bool onTheField;
    bool subsitute;
    bool showing;
    Pokemon::uniqueId alternateSprite;
};

#endif // SLOTBATTLEDATA_H

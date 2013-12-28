#ifndef SLOTBATTLEDATA_H
#define SLOTBATTLEDATA_H

#include "../PokemonInfo/pokemonstructs.h"
#include <vector>

class ShallowBattlePoke;

struct AuxPokeData
{
public:
    AuxPokeData();

    void onSendOut(ShallowBattlePoke* poke);
    void onSendBack();

    void setOnTheField(bool on) {onTheField = on;}
    void setSubstitute(bool on) {substitute = on;}
    void setShowing(bool on) {showing = on;}
    void setAlternateSprite(Pokemon::uniqueId num) {alternateSprite=num;}
    Pokemon::uniqueId getAlternateSprite() { return alternateSprite;}

    void vanish() {setShowing(false);}
    void reappear() {setShowing(true);}
    void changeSprite(Pokemon::uniqueId sprite) {setAlternateSprite(sprite);}
    void changeForme(int subnum) {alternateSprite.subnum = subnum;}
public:
    bool onTheField;
    bool substitute;
    bool showing;
    Pokemon::uniqueId alternateSprite;
};

struct FieldData
{
    FieldData(int numberOfSlots = 2) {
        auxdata.resize(numberOfSlots);
    }

    AuxPokeData &poke(int num) {
        return auxdata[num];
    }

    void shiftSpots(int spot1, int spot2) {
        std::swap(auxdata[spot1], auxdata[spot2]);
    }

    std::vector<AuxPokeData> auxdata;
};

#endif // SLOTBATTLEDATA_H

#include "teamdata.h"

ShallowBattlePoke& TeamData::poke(int slot) {
    if (!_init) {
        init();
    }
    return *pokemons[slot];
}

void TeamData::init() {
    _init = true;
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(new ShallowBattlePoke());
    }
}

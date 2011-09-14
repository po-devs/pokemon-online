#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <vector>

#include "teamdata.h"
#include "auxpokebattledata.h"

class DataContainer {
public:
    DataContainer() {
        /* Resizes for triple. Later, when loaded with battle configuration, will get
          more accurate loading */
        auxdata.resize(6);
    }

    TeamData *team(int player) {
        return &teams[player];
    }

    AuxPokeData &fieldPoke(int spot) {
        return auxdata[spot];
    }

    void swapFieldPokemons(int spot1, int spot2) {
        std::swap(auxdata[spot1], auxdata[spot2]);
    }

    std::vector<AuxPokeData> auxdata;
    TeamData teams[2];
};

#endif // DATACONTAINER_H

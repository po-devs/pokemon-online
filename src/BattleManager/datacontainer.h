#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <vector>

#include "teamdata.h"
#include "auxpokebattledata.h"

class DataContainer {
public:
    TeamData *team(int player) {
        return &teams[player];
    }

    AuxPokeData &fieldPoke(int spot) {
        return auxdata.poke(spot);
    }

    FieldData *field() {
        return &auxdata;
    }

    FieldData auxdata;
    TeamData teams[2];
};

#endif // DATACONTAINER_H

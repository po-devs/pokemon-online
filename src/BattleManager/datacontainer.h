#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <vector>

#include "teamdata.h"
#include "auxpokebattledata.h"

class BattleConfiguration;

class DataContainer {
public:
    DataContainer(const BattleConfiguration *configuration);
    ~DataContainer();

    TeamData *team(int player) {
        return teams[player];
    }

    AuxPokeData &fieldPoke(int spot) {
        return auxdata.poke(spot);
    }

    FieldData *field() {
        return &auxdata;
    }

    void reloadTeam(int player);

    FieldData auxdata;
    TeamData* teams[2];
    const BattleConfiguration *conf;
};

#endif // DATACONTAINER_H

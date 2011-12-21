#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <vector>

#include "teamdata.h"
#include "auxpokebattledata.h"

class BattleConfiguration;

class DataContainer {
public:
    typedef TeamData teamType;
    typedef FieldData fieldType;

    DataContainer(const BattleConfiguration *configuration);
    ~DataContainer();

    TeamData *team(int player) {
        return teams[player];
    }

    const TeamData *team(int player) const {
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

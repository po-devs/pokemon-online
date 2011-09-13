#ifndef PROXYDATACONTAINER_H
#define PROXYDATACONTAINER_H

#include "battledataaccessor.h"
#include "auxpokebattledata.h"

class ProxyDataContainer : public QObject {
    Q_OBJECT
public:
    ProxyDataContainer() {
        /* Needed for QML use */
        if (QMetaType::type("pokeid") == 0) {
            qRegisterMetaType<Pokemon::uniqueId>("pokeid");
        }

        /* Resizes for triple. Later, when loaded with battle configuration, will get
          more accurate loading */
        auxdata.resize(6);
    }

    Q_INVOKABLE TeamProxy *team(int player) {
        return &teams[player];
    }

    AuxPokeData &fieldPoke(int spot) {
        return auxdata[spot];
    }

    void swapFieldPokemons(int spot1, int spot2) {
        std::swap(auxdata[spot1], auxdata[spot2]);
    }

    std::vector<AuxPokeData> auxdata;
    TeamProxy teams[2];
};


#endif // PROXYDATACONTAINER_H

#ifndef PROXYDATACONTAINER_H
#define PROXYDATACONTAINER_H

#include "battledataaccessor.h"
#include "auxpokedataproxy.h"

class ProxyDataContainer : public QObject {
    Q_OBJECT
public:
    ProxyDataContainer() {
        /* Needed for QML use */
        if (QMetaType::type("pokeid") == 0) {
            qRegisterMetaType<Pokemon::uniqueId>("pokeid");
        }
    }

    Q_INVOKABLE TeamProxy *team(int player) {
        return &teams[player];
    }

    Q_PROPERTY (FieldProxy *field READ field CONSTANT)

    AuxPokeDataProxy &fieldPoke(int spot) {
        return *auxdata.poke(spot);
    }

    FieldProxy *field() {
        return &auxdata;
    }

    FieldProxy auxdata;

    TeamProxy teams[2];
};


#endif // PROXYDATACONTAINER_H

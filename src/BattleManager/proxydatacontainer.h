#ifndef PROXYDATACONTAINER_H
#define PROXYDATACONTAINER_H

#include "battledataaccessor.h"
#include "auxpokedataproxy.h"

class BattleConfiguration;

class ProxyDataContainer : public QObject {
    Q_OBJECT
public:
    ProxyDataContainer(BattleConfiguration *conf=NULL);
    ~ProxyDataContainer();

    Q_INVOKABLE TeamProxy *team(int player) {
        return teams[player];
    }

    Q_PROPERTY (FieldProxy *field READ field CONSTANT)

    AuxPokeDataProxy &fieldPoke(int spot) {
        return *auxdata.poke(spot);
    }

    FieldProxy *field() {
        return &auxdata;
    }

    void reloadTeam(int player);

private:
    FieldProxy auxdata;

    TeamProxy* teams[2];
    BattleConfiguration *conf;
};


#endif // PROXYDATACONTAINER_H

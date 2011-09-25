#include "proxydatacontainer.h"

ProxyDataContainer::ProxyDataContainer(BattleConfiguration *conf)
{
    /* Needed for QML use */
    if (QMetaType::type("pokeid") == 0) {
        qRegisterMetaType<Pokemon::uniqueId>("pokeid");
    }

    teams[0] = new TeamProxy(conf->receivingMode[0]==BattleConfiguration::Player);
    teams[1] = new TeamProxy(conf->receivingMode[1]==BattleConfiguration::Player);
}

ProxyDataContainer::~ProxyDataContainer()
{
    delete teams[0];
    delete teams[1];
}

#include "proxydatacontainer.h"
#include "teamdata.h"

ProxyDataContainer::ProxyDataContainer(const BattleConfiguration *conf) : conf(conf)
{
    /* Needed for QML use */
    if (QMetaType::type("pokeid") == 0) {
        qRegisterMetaType<Pokemon::uniqueId>("pokeid");
    }

    for (int i = 0; i < 2; i++) {
        if (conf->receivingMode[i]==BattleConfiguration::Player) {
            teams[i] = new TeamProxy(new TeamData(conf->teams[i]));
        } else {
            teams[i] = new TeamProxy();
        }
    }
}

void ProxyDataContainer::reloadTeam(int player)
{
    if (conf->receivingMode[player] == BattleConfiguration::Player) {
        teams[player]->setTeam(conf->teams[player]);
    }
}

ProxyDataContainer::~ProxyDataContainer()
{
    delete teams[0];
    delete teams[1];
}

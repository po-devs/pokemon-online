#include "proxydatacontainer.h"
#include "teamdata.h"

ProxyDataContainer::ProxyDataContainer(const BattleConfiguration *conf) : auxdata(conf->numberOfSlots()), conf(conf)
{
    /* Needed for QML use */
    if (QMetaType::type("pokeid") == 0) {
        qRegisterMetaType<Pokemon::uniqueId>("pokeid");
    }

    int numberInTeam = conf->numberOfSlots()/2;

    for (int i = 0; i < 2; i++) {
        if (conf->isPlayer(i)) {
            teams[i] = new TeamProxy(new TeamData(conf->teams[i]));

            for (int j = 0; j < numberInTeam; j++) {
                auxdata.poke(i+j*2)->setPlayerPoke(true);
            }
        } else {
            teams[i] = new TeamProxy();
        }
        for (int j = 0; j < numberInTeam; j++) {
            auxdata.poke(i+j*2)->setPoke(teams[i]->poke(j));
        }
    }
}

void ProxyDataContainer::reloadTeam(int player)
{
    if (conf->receivingMode[player] == BattleConfiguration::Player) {
        teams[player]->setTeam(conf->teams[player]);
    
        int numberInTeam = conf->numberOfSlots()/2;
        for (int j = 0; j < numberInTeam; j++) {
            auxdata.poke(player+j*2)->setPoke(teams[player]->poke(j));
        }
    }
}

ProxyDataContainer::~ProxyDataContainer()
{
    delete teams[0];
    delete teams[1];
}

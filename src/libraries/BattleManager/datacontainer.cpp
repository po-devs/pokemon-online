#include "datacontainer.h"

DataContainer::DataContainer(const BattleConfiguration *conf) : auxdata(conf->numberOfSlots()), conf(conf)
{
    for (int i = 0; i < 2; i++) {
        if (conf->receivingMode[i]==BattleConfiguration::Player) {
            teams[i] = new TeamData(conf->teams[i]);
        } else {
            teams[i] = new TeamData();
        }
    }
}

void DataContainer::reloadTeam(int player)
{
    if (conf->receivingMode[player] == BattleConfiguration::Player) {
        teams[player]->setTeam(conf->teams[player]);
    }
}

DataContainer::~DataContainer()
{
    delete teams[0];
    delete teams[1];
}

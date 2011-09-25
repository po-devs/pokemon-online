#include "datacontainer.h"

DataContainer::DataContainer(BattleConfiguration *conf)
{
    for (int i = 0; i < 2; i++) {
        if (conf->receivingMode[i]==BattleConfiguration::Player) {
            teams[i] = new TeamData(conf->teams[i]);
        } else {
            teams[i] = new TeamData();
        }
    }
}

DataContainer::~DataContainer()
{
    delete teams[0];
    delete teams[1];
}

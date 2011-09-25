#include "datacontainer.h"

DataContainer::DataContainer(BattleConfiguration *conf)
{
    teams[0] = new TeamData(conf->receivingMode[0]==BattleConfiguration::Player);
    teams[1] = new TeamData(conf->receivingMode[1]==BattleConfiguration::Player);
}

DataContainer::~DataContainer()
{
    delete teams[0];
    delete teams[1];
}

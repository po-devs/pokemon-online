#include "battledata.h"

void BattleData::onKo(int)
{
}

TeamData &BattleData::team(int player)
{
    return teams[player];
}

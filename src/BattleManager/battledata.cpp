#include "battledata.h"

void BattleData::onKo(int)
{
}

TeamData &BattleData::team(int player)
{
    return teams[player];
}

QString BattleData::name(int player)
{
    return teams[this->player(player)].name();
}

int BattleData::player(int spot)
{
    return spot % 2;
}

ShallowBattlePoke &BattleData::poke(int player)
{
    return teams[this->player(player)].poke(slotNum(player));
}

int BattleData::slotNum(int player)
{
    return player/2;
}

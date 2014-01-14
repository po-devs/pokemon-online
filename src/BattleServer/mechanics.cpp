#include "mechanics.h"


BattleBase::context & Mechanics::team(BattleSituation &b, int player)
{
    return b.teamMemory(player);
}

BattleBase::context & Mechanics::slot(BattleSituation &b, int player)
{
    return b.slotMemory(player);
}

BattleSituation::priorityBracket Mechanics::makeBracket(int b, int p)
{
    return BattleSituation::priorityBracket(b, p);
}

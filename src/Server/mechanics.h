#ifndef MECHANICS_H
#define MECHANICS_H

#include "mechanicsbase.h"
#include "battle.h"

struct Mechanics : public MechanicsBase<BattleSituation::MechanicsFunction>
{
    static BattleBase::context & team(BattleSituation &b, int player);
    static BattleBase::context & slot(BattleSituation &b, int player);

    static BattleSituation::priorityBracket makeBracket(int b, int p);

    typedef BattleSituation::MechanicsFunction function;
};

#endif // MECHANICS_H

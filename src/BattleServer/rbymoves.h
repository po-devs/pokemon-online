#ifndef RBYMOVES_H
#define RBYMOVES_H

#include "mechanicsbase.h"
#include "battlerby.h"

/* MoveMechanics works by inserting functions into the contexts of the battle situation,
    that are then called when necessary. The functions are inserted by MoveEffect::setup,
    in fact it checks the non-NULL members of type 'function' of a derived class of MoveMechanics
    and insert them into the battle situation according to the name of the effect */

struct RBYMechanics : public MechanicsBase<BattleRBY::MechanicsFunction> {
    typedef BattleRBY::MechanicsFunction function;

    static BattleRBY::SlotMemory & slot(BattleRBY &battle, int s);
};

typedef RBYMechanics RBYMoveMechanics;

/* Used to tell us everything about a move */
struct RBYMoveEffect
{
    RBYMoveEffect(int num, Pokemon::gen gen, BattleBase::BasicMoveInfo &bmi);

    static void setup(int movenum, int source, int target, BattleBase &b);
    static void unsetup(int movenum, int source, BattleBase &b);

    static QHash<int, RBYMoveMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
};

#endif // RBYMOVES_H

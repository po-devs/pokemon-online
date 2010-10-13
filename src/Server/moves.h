#ifndef MOVES_H
#define MOVES_H

#include "mechanics.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "battle.h"

/* MoveMechanics works by inserting functions into the contexts of the battle situation,
    that are then called when necessary. The functions are inserted by MoveEffect::setup,
    in fact it checks the non-NULL members of type 'function' of a derived class of MoveMechanics
    and insert them into the battle situation according to the name of the effect */
struct MoveMechanics : public Mechanics
{
    static int num(const QString &name);
};

/* Used to tell us everything about a move */
struct MoveEffect
{
    MoveEffect(int num, int gen, BattleSituation::BasicMoveInfo &bmi);

    static void setup(int movenum, int source, int target, BattleSituation &b);
    static void unsetup(int movenum, int source, BattleSituation &b);

    static QHash<int, MoveMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
};

#endif // MOVES_H

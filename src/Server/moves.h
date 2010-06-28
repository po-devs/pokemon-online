#ifndef MOVES_H
#define MOVES_H

#include "mechanics.h"

/* MoveMechanics works by inserting functions into the contexts of the battle situation,
    that are then called when necessary. The functions are inserted by MoveEffect::setup,
    in fact it checks the non-NULL members of type 'function' of a derived class of MoveMechanics
    and insert them into the battle situation according to the name of the effect */
struct MoveMechanics : public Mechanics
{
    static int num(const QString &name);
};

/* Used to tell us everything about a move */
struct MoveEffect : public QVariantHash
{
    MoveEffect(int num);

    static void setup(int movenum, int source, int target, BattleSituation &b);
    static void unsetup(int movenum, int source, BattleSituation &b);

    static QHash<int, MoveMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
};

struct MirrorMoveAmn : public QSet<int>
{
    /*
Mirror Move cannot replicate the following moves:
Acupressure, Chatter, Counter, Curse, Encore, Doom Desire, Feint, Focus Punch, Future Sight, Helping Hand, Mimic, Mirror Coat, Role Play, Psych Up, Sketch, Spit Up, Struggle, and Transform

Mirror Move cannot replicate moves that call other moves, nor the moves that are called.
These moves include: Me First, Nature Power. These moves are not added to the Mirror Move user's 'memory'.
*/
    MirrorMoveAmn() {
	(*this) << 4 << 58 <<  70 << 78 << 92 << 114 << 128 << 144 << 154 << 182 << 247 << 251 << 330 << 298 << 358 << 383 << 394 << 432;
	(*this) << 233 << 265;
    }
};

#endif // MOVES_H

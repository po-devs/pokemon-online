#ifndef MOVES_H
#define MOVES_H

#include "mechanics.h"
#include "../PokemonInfo/pokemonstructs.h"

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
These moves include: Assist, Copycat, Magic Coat, Me First, Metronome, Mirror Move, Nature Power, Sleep Talk, and Snatch. These moves are not added to the Mirror Move user's 'memory'.
*/
    MirrorMoveAmn() {
        (*this) << Move::Acupressure << Move::Chatter << Move::Counter << Move::Curse << Move::Encore << Move::DoomDesire
                << Move::Feint << Move::FocusPunch << Move::FutureSight << Move::HelpingHand << Move::Mimic << Move::MirrorCoat
                << Move::RolePlay << Move::PsychUp << Move::Sketch << Move::SpitUp << Move::Struggle << Move::Transform
                << Move::Assist << Move::Copycat << Move::MagicCoat << Move::MeFirst <<Move:: Metronome << Move::MirrorMove
                << Move::NaturePower << Move::SleepTalk << Move::Snatch;
    }
};

#endif // MOVES_H

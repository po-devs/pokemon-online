#ifndef MISCABILITIES_H
#define MISCABILITIES_H

#include "abilities.h"

typedef AbilityMechanics AM;
typedef BattleSituation BS;

struct AMFlashFire : public AM {
    AMFlashFire() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if ((b.gen() <= 4 && b.poke(s).status() == Pokemon::Frozen) || b.isProtected(s, t)) {
            return;
        }
        if (type(b,t) == Pokemon::Fire && (b.gen() >= 4 || tmove(b,t).power > 0) ) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            if (!poke(b,s).contains("FlashFired")) {
                b.sendAbMessage(19,0,s,s,Pokemon::Fire);
                poke(b,s)["FlashFired"] = true;
            } else {
                b.sendAbMessage(19,1,s,s,Pokemon::Fire, move(b,t));
            }
        }
    }
};

struct AMMagicBounce : public AM
{
    AMMagicBounce() {
        functions["UponSetup"] = &uas;
    }

    static void uas (int, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure2", "MagicBounce", &dgaf);
    }

    static void dgaf(int s, int t, BS &b) {
        bool bounced = tmove(b, s).flags & Move::MagicCoatableFlag;
        if (!bounced)
            return;
        /* Don't double bounce something */
        if (b.battleMemory().contains("CoatingAttackNow")) {
            return;
        }

        int target = -1;

        if (t != s && b.hasWorkingAbility(t, Ability::MagicBounce) ) {
            target = t;
        } else {
            /* Entry hazards */
            if (tmove(b,s).targets == Move::OpposingTeam) {
                foreach(int t, b.revs(s)) {
                    if (b.koed(t)) {
                        continue;
                    }
                    if (b.hasWorkingAbility(t, Ability::MagicBounce) && !(b.hasWorkingAbility(s, Ability::MoldBreaker) || b.hasWorkingAbility(s, Ability::TurboBlaze) ||b.hasWorkingAbility(s, Ability::TeraVolt))) {
                        target = t;
                        break;
                    }
                }
            }
        }

        if (target == -1)
            return;

        b.fail(s,76, 2, Pokemon::Psychic);

        bounceAttack(s, target, b);
    }

    static void bounceAttack(int s, int bouncer, BS &b) {
        int move = AM::move(b,s);
        /* Now Bouncing back ... */
        BS::context ctx = turn(b,bouncer);
        BS::BasicMoveInfo info = tmove(b,bouncer);
        BS::TurnMemory turnMem = fturn(b, bouncer);
        int lastMove = fpoke(b,bouncer).lastMoveUsed;

        turn(b,bouncer).clear();
        MoveEffect::setup(move,bouncer,s,b);

        turn(b,bouncer)["Target"] = s;
        b.battleMemory()["CoatingAttackNow"] = turn(b,s)["StealingAttack"] = true;
        b.useAttack(bouncer,move,true,false);
        turn(b,s).remove("StealingAttack");
        b.battleMemory().remove("CoatingAttackNow");

        /* Restoring previous state. Only works because moves reflected don't store useful data in the turn memory,
            and don't cause any such data to be stored in that memory

            If the original pokemon is no longer on the field we skip this step (Read: Parting Shot + Mega Evolve + Magic Bounce)
            Doesn't matter what move the pokemon last used if it switches out because pokememory is cleared.
        */
        if (turn(b,bouncer).contains("UTurnSuccess")) {
            return;
        }

        turn(b,bouncer) = ctx;
        tmove(b,bouncer) = info;
        fturn(b,bouncer) = turnMem;
        fpoke(b,bouncer).lastMoveUsed = lastMove;
    }
};


#endif // MISCABILITIES_H

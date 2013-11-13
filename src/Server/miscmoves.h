#ifndef MISCMOVES_H
#define MISCMOVES_H

#include "moves.h"
#include "battlecounterindex.h"

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;
typedef BattleCounterIndex BC;
typedef BattleSituation::TurnMemory TM;

struct MMDisable : public MM
{
    MMDisable() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
        if (failOn(t, b))
            fturn(b,s).add(TM::Failed);
    }

    static bool failOn(int t, BS &b) {
        if (b.counters(t).hasCounter(BC::Disable)) {
            return true;
        }

        /* Gen 1 Disable works even when opponent hasn't moved yet, but not with 0 PP in all moves */
        if (b.gen().num == 1) {
            for (int i = 0; i<4; i++) {
                if (b.PP(t, i) > 0) {
                    return false;
                }
            }
            /* All moves have 0PP, therefore move fails */
            return true;
        }

        if (!poke(b,t).contains("LastMoveUsedTurn")) {
            return true;
        }

        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
        if (tu + 1 < b.turn() || (tu + 1 == b.turn() && fturn(b,t).contains(TM::HasMoved))) {
            return true;
        }

        int move = poke(b,t)["LastMoveUsed"].toInt();
        int sl = -1;
        for (int i = 0; i < 4; i++) {
            if (b.move(t, i) == move) {
                sl = i;
            }
        }

        if (sl == -1 || b.PP(t, sl) == 0 ) {
            return true;
        }

        return false;
    }

    static BS::priorityBracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 12) : makeBracket(14, 0) ;
    }

    static void uas (int s, int t, BS &b) {
        int mv = poke(b,t)["LastMoveUsed"].toInt();
        /* Disable disables a random move in gen 1 */
        if (b.gen().num == 1) {
            /* Number of Moves on moveset */
            int moves = 4;
            if (b.move(t,1) == 0) {
                moves = 1;
            }
            else if (b.move(t,2) == 0) {
                moves = 2;
            }
            else if (b.move(t,3) == 0) {
                moves = 3;
            }
            mv = 0;
            int randnum = b.randint(moves);
            while (mv == 0) {
                mv = b.move(t,randnum);
                /* Checks that move doesn't have 0 PP */
                if (b.PP(t, randnum) == 0) {
                    mv = 0;
                }
                randnum = b.randint(moves);
            }
        }
        b.sendMoveMessage(28,0,s,0,t,mv);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            b.counters(t).addCounter(BC::Disable, 3 + (b.randint(4)));
            poke(b,t)["DisabledMove"] = mv;
            addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
            addFunction(poke(b,t), "MovePossible", "Disable", &mp);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Disable", &et);
        }
    }

    static void et (int s, int, BS &b)
    {
        if (b.counters(s).count(BC::Disable) < 0) {
            removeFunction(poke(b,s), "MovesPossible", "Disable");
            removeFunction(poke(b,s), "MovePossible", "Disable");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Disable");
            b.sendMoveMessage(28,2,s);
            b.counters(s).removeCounter(BC::Disable);
        }
    }

    static void msp(int s, int, BS &b) {
        int mv = poke(b,s)["DisabledMove"].toInt();
        for (int i = 0 ; i < 4; i++) {
            if (b.move(s, i) == mv)
                turn(b,s)[QString("Move%1Blocked").arg(i)] = true;
        }
    }

    static void mp(int s, int, BS &b) {
        //doesn't block moves called through sleep talk
        //maybe shouldn't block any moves called through another move
        if(move(b,s) == poke(b,s)["DisabledMove"] && !turn(b,s).contains("SleepTalkedMove")) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(28,1,s,0,s, move(b,s));
        }
    }
};

#endif // MISCMOVES_H

#ifndef MISCMOVES_H
#define MISCMOVES_H

#include "moves.h"
#include "battlecounterindex.h"

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;
typedef BattleCounterIndex BC;

struct MMDisable : public MM
{
    MMDisable() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
        if (failOn(t, b))
            turn(b,s)["Failed"] = true;
    }

    static bool failOn(int t, BS &b) {
        if (b.counters(t).hasCounter(BC::Disable)) {
            return true;
        }
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
            return true;
        }

        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
        if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
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

    static BS::priorityBracket bracket(int gen) {
        return gen <= 4 ? makeBracket(6, 12) : makeBracket(14, 0) ;
    }

    static void uas (int s, int t, BS &b) {
        int mv = poke(b,t)["LastMoveUsed"].toInt();
        b.sendMoveMessage(28,0,s,0,t,mv);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            b.counters(t).addCounter(BC::Disable, 3 + (b.true_rand()%4));
            poke(b,t)["DisabledMove"] = mv;
            addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
            addFunction(poke(b,t), "MovePossible", "Disable", &mp);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Disable");
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
        if(move(b,s) == poke(b,s)["DisabledMove"]) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(28,1,s,0,s,b.move(s,poke(b,s)["MoveSlot"].toInt()));
        }
    }
};

#endif // MISCMOVES_H

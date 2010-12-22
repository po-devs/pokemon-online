#ifndef MISCMOVES_H
#define MISCMOVES_H

#include "moves.h"

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

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
        if (poke(b,t).contains("DisablesUntil") && poke(b,t).value("DisablesUntil").toInt() >= b.turn()) {
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

        if (sl == -1 || b.poke(t).move(sl).PP() == 0 ) {
            return true;
        }

        return false;
    }

    static void uas (int s, int t, BS &b) {
        int mv = poke(b,t)["LastMoveUsed"].toInt();
        b.sendMoveMessage(28,0,s,0,t,mv);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            poke(b,t)["DisablesUntil"] = b.turn() + 3 + (b.true_rand()%4);
            poke(b,t)["DisabledMove"] = mv;
            addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
            addFunction(poke(b,t), "MovePossible", "Disable", &mp);
            addFunction(poke(b,t), "EndTurn611", "Disable", &et);
        }
    }

    static void et (int s, int, BS &b)
    {
        int tt = poke(b,s)["DisablesUntil"].toInt();
        if (tt <= b.turn()) {
            removeFunction(poke(b,s), "MovesPossible", "Disable");
            removeFunction(poke(b,s), "MovePossible", "Disable");
            removeFunction(poke(b,s), "EndTurn611", "Disable");
            poke(b,s).remove("DisablesUntil");
            b.sendMoveMessage(28,2,s);
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

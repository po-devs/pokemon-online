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
        if (b.gen() <= 4 && b.poke(s).status() == Pokemon::Frozen) {
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

#endif // MISCABILITIES_H

#include "berries.h"
#include "items.h"
#include "../PokemonInfo/pokemonstructs.h"

typedef BerryMechanics BM;
typedef BattleSituation BS;

struct BMStatusBerry : public BM
{
    BMStatusBerry() {
        functions["UponSetup"] = &asc;
        functions["AfterStatusChange"] = &asc;
    }

    static void asc(int p, int s, BS &b) {
        if (b.koed(s))
            return;

        int status = b.poke(s).status();
        bool conf = b.isConfused(s);
        int arg = poke(b,p)["ItemArg"].toInt();

        /* Confusion berry */
        if (arg == -1) {
            if (conf) {
                b.eatBerry(s, p==s);
                b.healConfused(s);
                b.sendBerryMessage(1, s, 0);
            }
            return;
        }

        /* Lum berry */
        if (conf && arg == 0) {
            b.healConfused(s);
            goto end;
        }

        if (status == Pokemon::Fine) {
            return;
        }

        /* Lum berry */
        if (arg == 0) {
            if (status == Pokemon::Fine)
                return;
            goto end;
        } else { /* Other Status Berry */
            if (status == arg) {
                goto end;
            }
        }

        return;

        end:
        b.eatBerry(s, p==s);
        b.healStatus(s, status);
        b.sendBerryMessage(1, s, arg + 1);
    }
};

struct BMLeppa : public BM
{
    BMLeppa() {
        functions["UponSetup"] = &appl;
        functions["AfterTargetList"] = &appl;
        functions["AfterPPLoss"] = &appl;
    }

    static void appl(int p, int s, BS &b) {
        if (b.koed(s))
            return;
        int minmove = 0;
        int minPP = 100;
        bool init = false;
        bool zeroPP = false;
        for (int i = 0; i < 4; i++) {
            if (b.move(s, i) == 0) {
                continue;
            }
            if (b.PP(s, i) == 0) {
                zeroPP = true;
                init = true;
                minmove = i;
                minPP = 0;
                break;
            }
            if (b.PP(s, i) < minPP && (fpoke(b, s).moves[i] == b.poke(s).move(i).num() ?
                                       b.PP(s, i) < b.poke(s).move(i).totalPP() : b.PP(s, i) < 5)) {
                minmove = i;
                init = true;
                minPP = b.PP(s, i);
            }
        }
            
        
        if (init && (zeroPP || turn(b,p).value("BugBiter").toBool())) {
            b.eatBerry(s, s==p);
            b.sendBerryMessage(2,s,0,0,0,b.move(s,minmove));

            b.gainPP(s,minmove,b.gen() <= 2 ? 5 : 10);
        }
    }
};

struct BMPinch : public BM
{
    static bool testpinch(int p, int s, BS &b, int ratio) {
        if (turn(b,p).value("BugBiter").toBool()) {
            b.eatBerry(s);
            return true;
        }
        //Gluttony
        if (b.hasWorkingAbility(s, Ability::Gluttony))
            ratio = 2;

        if (!b.koed(s)) {
            int lp = b.poke(s).lifePoints();
            int tp = b.poke(s).totalLifePoints();

            if (lp*ratio <= tp) {
                b.eatBerry(s,s==p);
                return true;
            }
        }

        return false;
    }
};

struct BMPinchHP : public BMPinch
{
    BMPinchHP() {
        functions["UponSetup"] = &tp;
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp(int p, int s, BS &b) {
        if (!b.canHeal(s))
            return;

        if (!testpinch(p, s, b, 2))
            return;

        b.sendBerryMessage(3,s,0);
        int arg = poke(b,p)["ItemArg"].toInt();
        if (arg == 10) /* oran berry */
            b.healLife(s, 10);
        else /* Sitrus Berry */
        {
            if (b.gen() >= 4)
                b.healLife(s, b.poke(s).totalLifePoints()/4);
            else
                b.healLife(s, 30);
        }
    }
};

struct BMAntiSuperEffective : public BM
{
    BMAntiSuperEffective() {
        functions["Mod3Items"] = &m3b;
    }

    static void m3b(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        if (!b.hasSubstitute(s) && fturn(b,t).typeMod > 0 && tmove(b,t).type == poke(b,s)["ItemArg"].toInt()) {
            b.sendBerryMessage(4,s,0,t,b.poke(s).item(),move(b,t));
            b.eatBerry(s,false);

            turn(b,t)["Mod3Berry"] = -5;
        }
    }
};

struct BMAntiNormal : public BM
{
    BMAntiNormal() {
        functions["Mod3Items"] = &m3b;
    }

    static void m3b(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        /* We never want to activate this berry if this is consumed by Bug Bite */
        if (b.gen() >= 4 && !turn(b,s).value("BugBiter").toBool()) {
            /* Normal moves */
            if (!b.hasSubstitute(s) && tmove(b,t).type == 0) {
                b.sendBerryMessage(4,s,0,t,b.poke(s).item(),move(b,t));
                b.eatBerry(s,false);

                turn(b,t)["Mod3Berry"] = -5;
            }
        }
    }
};

struct BMSuperHP : public BM
{
    BMSuperHP() {
        functions["TestPinch"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        if (!b.canHeal(s))
            return;
        if (fturn(b,t).typeMod <= 0)
            return;
        b.eatBerry(s);
        b.sendBerryMessage(6,s,0);
        b.healLife(s, b.poke(s).totalLifePoints()/5);
    }
};

struct BMPinchStat : public BMPinch
{
    BMPinchStat() {
        functions["UponSetup"] = &tp;
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == p && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp(int p, int s, BS &b) {
        /* The berry may change after the call to test pinch (eaten),
           so saved before. */
        int berry = b.poke(s).item();

        if (!testpinch(p, s, b, 4))
            return;

        int arg = poke(b,p)["ItemArg"].toInt();

        if (b.isOut(s)) {
            if (b.hasWorkingAbility(s, Ability::Contrary)) {
                b.sendBerryMessage(7,s,1,s, berry, arg);
            } else {
                b.sendBerryMessage(7,s,0,s, berry, arg);
            }
            b.inflictStatMod(s, arg, 1, s, false);
        }
    }
};

struct BMCriticalPinch : public BMPinch
{
    BMCriticalPinch() {
        functions["UponSetup"] = &tp;
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == p && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp(int p, int s, BS &b) {
        if (!testpinch(p, s, b,4))
            return;

        uas(p,s,b);
    }

    /* ripped off from focus energy */
    static void uas(int, int s, BS &b) {
        if (b.isOut(s)) {
            addFunction(poke(b,s), "TurnSettings", "FocusEnergy", &ts);
            b.sendMoveMessage(46,0,s);
        }
    }
    static void ts(int s, int, BS &b) {
        addFunction(turn(b,s), "BeforeTargetList", "FocusEnergy", &btl);
    }
    static void btl(int s, int, BS &b) {
        if (tmove(b,b.attacker()).power > 0) {
            tmove(b,s).critRaise += 2;
        }
    }
};

struct BMStarf : public BMPinch
{
    BMStarf() {
        functions["UponSetup"] = &tp;
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == p && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp(int p, int s, BS &b) {
        if (!b.isOut(s)) {
            return;
        }

        int berry = b.poke(s).item();

        QVector<int> stats;
        for (int i = Attack; i <= Evasion; i++) {
            if (fpoke(b,s).boosts[i] < 6) {
                stats.push_back(i);
            }
        }
        if (stats.empty())
            return;

        if (!testpinch(p, s, b, 4))
            return;

        int stat = stats[b.randint(stats.size())];
        if (b.hasWorkingAbility(s, Ability::Contrary)) {
            b.sendBerryMessage(9,s,1,s, berry, stat);
        } else {
            b.sendBerryMessage(9,s,0,s, berry, stat);
        }
        b.inflictStatMod(s, stat, 2, s, false);
    }
};

struct BMBerryLock : public BMPinch
{
    BMBerryLock() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == p && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp(int p, int s, BS &b) {
        if (!b.isOut(s)) {
            return;
        }

        if (!testpinch(p, s, b,4)) {
            return;
        }

        poke(b,s)["BerryLock"] = true;
        b.sendBerryMessage(10,s,0);
    }
};

struct BMCustap : public BMPinch
{
    BMCustap() {
        functions["TurnOrder"] = &to;
    }

    static void to (int p, int s, BS &b) {
        if (!b.isOut(p)) {
            return;
        }
        if (!testpinch(p, s, b, 4)) {
            return;
        }

        b.sendBerryMessage(11,s,0);
        turn(b,s)["TurnOrder"] = 3;
    }
};

struct BMBerryRecoil : public BM
{
    BMBerryRecoil() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        //Magic Guard
        if (tmove(b,t).category != poke(b,s)["ItemArg"].toInt() || b.koed(t) || b.hasWorkingAbility(t, Ability::MagicGuard)) {
            return;
        }
        b.eatBerry(s);
        b.sendBerryMessage(12,s,0,t);
        b.inflictDamage(t, b.poke(t).totalLife()/8,s,false);
    }
};

struct BMConfuseBerry : public BMPinch
{
    BMConfuseBerry() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int p, int s, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && tmove(b,b.attacker()).power > 0)
            return;
        tp(p, s, b);
    }

    static void tp (int p, int s, BS &b) {
        if (!b.isOut(s))
            return;

        if (!testpinch(p, s, b, 4))
            return;

        if (!b.canHeal(s))
            return;

        b.eatBerry(s);
        b.sendBerryMessage(6,s,0);
        b.healLife(s, b.poke(s).totalLifePoints()/8);
        //Fixme: Nature checking for Trick, Switcheroo, Covet, Thief, Bug Bite, Fling, etc.
        b.inflictConfused(s,s);
    }
};

struct BMPhysicalStat : public BM
{
    BMPhysicalStat() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        int arg = poke(b,s)["ItemArg"].toInt();
        int berry = b.poke(s).item();
        if (tmove(b,t).category != Move::Special) {
             return;
         }
        b.eatBerry(s);
        b.inflictStatMod(s,arg,1,s,false);
        if (b.isOut(s)) {
            if (b.hasWorkingAbility(s, Ability::Contrary)) {
                b.sendBerryMessage(7,s,1,s, berry, arg);
            } else {
                b.sendBerryMessage(7,s,0,s, berry, arg);
            }

    }
}
};
struct BMSpecialStat : public BM
{
    BMSpecialStat() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (!b.attacking()) {
            return;
        }
        int arg = poke(b,s)["ItemArg"].toInt();
        int berry = b.poke(s).item();
        if (tmove(b,t).category != Move::Special) {
             return;
         }
        b.eatBerry(s);
        b.inflictStatMod(s,arg,1,s,false);

        if (b.isOut(s)) {
            if (b.hasWorkingAbility(s, Ability::Contrary)) {
                b.sendBerryMessage(7,s,1,s, berry, arg);
            } else {
                b.sendBerryMessage(7,s,0,s, berry, arg);
            }

    }
}
};
#define REGISTER_BERRY(num, name) mechanics[num+8000] = BM##name(); names[num+8000] = #name; nums[#name] = num+8000;

void ItemEffect::initBerries()
{
    REGISTER_BERRY(1, StatusBerry);
    REGISTER_BERRY(2, Leppa);
    REGISTER_BERRY(3, PinchHP);
    REGISTER_BERRY(4, AntiSuperEffective);
    REGISTER_BERRY(5, AntiNormal);
    REGISTER_BERRY(6, SuperHP);
    REGISTER_BERRY(7, PinchStat);
    REGISTER_BERRY(8, CriticalPinch);
    REGISTER_BERRY(9, Starf);
    REGISTER_BERRY(10, BerryLock);
    REGISTER_BERRY(11, Custap);
    REGISTER_BERRY(12, BerryRecoil);
    REGISTER_BERRY(13, ConfuseBerry);
    REGISTER_BERRY(14, PhysicalStat);
    REGISTER_BERRY(15, SpecialStat)
}

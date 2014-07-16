#include "rbymoves.h"
#include "battlefunctions.h"

typedef RBYMoveMechanics MoveMechanics;

QHash<int, MoveMechanics> RBYMoveEffect::mechanics;
QHash<int, QString> RBYMoveEffect::names;
QHash<QString, int> RBYMoveEffect::nums;

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleRBY BS;
typedef BS::TurnMemory TM;

BS::SlotMemory & MoveMechanics::slot(BattleRBY &battle, int s) {
    return battle.slotMemory(s);
}

void RBYMoveEffect::setup(int num, int source, int target, BattleBase &b)
{
    /* first the basic info */
    MoveMechanics::initMove(num, b.gen(), MM::tmove(b,source));

    BS *_b = dynamic_cast<BS*>(&b);

    if (_b) {
        BS &b = *_b;
        /* then the hard info */
        QStringList specialEffects = MoveInfo::SpecialEffect(num, b.gen()).split('|');

        foreach (QString specialEffectS, specialEffects) {
            std::string s = specialEffectS.toStdString();

            int specialEffect = atoi(s.c_str());

            /* if the effect is invalid or not yet implemented then no need to go further */
            if (!mechanics.contains(specialEffect)) {
                break;
            }

            MoveMechanics &m = mechanics[specialEffect];
            QString &n = names[specialEffect];

            QHash<QString, MoveMechanics::function>::iterator i;

            size_t pos = s.find('-');
            if (pos != std::string::npos) {
                MM::turn(b,source)[n+"_Arg"] = specialEffectS.mid(pos+1);
            }

            for(i = m.functions.begin(); i != m.functions.end(); ++i) {
                if (i.key() == "OnSetup") {
                    i.value()(source,target,b);
                } else {
                    MM::addFunction(MM::turn(b,source), i.key(), n, i.value());
                }
            }
        }
    }
}


/* Used by moves like Metronome that may use moves like U-Turn. Then AfterAttackSuccessful would be called twice, and that would
    not be nice because U-Turning twice :s*/
void RBYMoveEffect::unsetup(int num, int source, BattleBase &b)
{
    BS *_b = dynamic_cast<BS*>(&b);

    if (_b) {
        BS &b = *_b;

        /* then the hard info */
        QStringList specialEffects = MoveInfo::SpecialEffect(num, b.gen()).split('|');

        foreach (QString specialEffectS, specialEffects) {
            std::string s = specialEffectS.toStdString();

            int specialEffect = atoi(s.c_str());

            /* if the effect is invalid or not yet implemented then no need to go further */
            if (!mechanics.contains(specialEffect)) {
                break;
            }

            MoveMechanics &m = mechanics[specialEffect];
            QString &n = names[specialEffect];

            QHash<QString, MoveMechanics::function>::iterator i;

            for(i = m.functions.begin(); i != m.functions.end(); ++i) {
                if (i.key() == "OnSetup") {
                    ;
                } else {
                    MM::removeFunction(MM::turn(b,source), i.key(), n);
                }
            }
        }
    }

    MM::tmove(b,source).classification = Move::StandardMove;
}

struct RBYBide : public MM
{
    RBYBide() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        t = b.opponent(s);
        poke(b,s)["BideCount"] = 2 + b.randint(2);
        poke(b,t).remove("DamageInflicted");
        poke(b,s)["BideDamage"] = 0;
        addFunction(poke(b,s), "TurnSettings", "Bide", &ts);
    }

    static void uas2(int s, int t, BS &b) {
        t = b.opponent(s);

        inc(poke(b,s)["BideCount"], -1);

        int count = poke(b,s)["BideCount"].toInt();

        inc(poke(b,s)["BideDamage"], poke(b,t).value("DamageInflicted").toInt());
        if (count > 0) {
            b.sendMoveMessage(9, 0, s);
        } else {
            int damage = poke(b,s)["BideDamage"].toInt();

            b.sendMoveMessage(9, 1, s);
            if (damage == 0) {
                b.notifyFail(s);
            } else {
                b.inflictDamage(t, 2*damage, s, true);
            }

            poke(b,s).remove("BideCount");
            removeFunction(poke(b,s), "TurnSettings", "Bide");
        }
    }

    static void ts(int s, int, BS &b) {
        fturn(b,s).add(TM::KeepAttack);
        addFunction(turn(b,s), "UponAttackSuccessful", "Bide", &uas2);

        turn(b,s)["TellPlayers"] = false;
    }
};

struct RBYBind : public MM
{
    RBYBind() {
        functions["MoveSettings"] = &ms;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int , int t, BS &b) {
        poke(b,t).remove("Recharging"); //Bind removes recharging from hyper beam
    }

    static void uas(int s, int t, BS &b) {
        poke(b,s)["BindCount"] = minMax(tmove(b,s).minTurns-1, tmove(b,s).maxTurns-1, b.gen().num, b.randint());
        poke(b,s)["LastBind"] = b.turn();
        poke(b,s)["BindDamage"] = poke(b,s)["DamageInflicted"];
        poke(b,t)["Bound"] = true;
        addFunction(poke(b,s), "TurnSettings", "Bind", &ts);
        addFunction(poke(b,t), "MovePossible", "Bind", &mp);
    }

    static void ts(int s, int, BS &b) {
        /* If the opponent was koed, reset everything */
        if(!poke(b, b.opponent(s)).contains("Bound")) {
            poke(b,s).remove("BindCount");
        }
        if (poke(b,s).value("LastBind").toInt() == b.turn()-1 && poke(b,s).value("BindCount").toInt() > 0) {
            fturn(b,s).add(TM::KeepAttack);
            addFunction(turn(b,s), "UponAttackSuccessful", "Bind", &uas2);
            addFunction(turn(b,s), "EvenWhenCantMove", "Bind", &ewcm);
            /* Bind does the same damage every turn */
            addFunction(turn(b,s), "CustomAttackingDamage", "Bind", &cad);
            if (b.gen() <= Pokemon::gen(Gen::Yellow)) {
                turn(b,b.opponent(s)) ["ForceBind"] = true;
            }
            initMove(fpoke(b,s).lastMoveUsed, b.gen(), tmove(b,s));
        }
        turn(b,s)["TellPlayers"] = false;
    }

    static void cad(int s, int t, BS &b) {
        if (poke(b,t).contains("Bound")) {
            turn(b,s)["CustomDamage"] = poke(b,s)["BindDamage"];
            b.sendMoveMessage(10, 2, s);
        }
    }

    static void ewcm(int s, int, BS &b) {
        int t = b.opponent(s);

        if (!poke(b,t).contains("Bound")) {
            fturn(b,s).add(TM::UsePP); //If the opponent switched out, we use an additional PP
        } else {
            tmove(b,s).accuracy = 0;
            tmove(b,s).power = 1;
        }
    }

    static void mp(int s, int t, BS &b) {
        t = b.opponent(s);
        /* Either Bind was used last turn and is ongoing, or was used this turn (and may have finished) */
        if (( (poke(b,s).contains("Bound") || poke(b,t).contains("BindCount")) && poke(b,t).value("LastBind").toInt() >= b.turn()-1) ||
                poke(b,t).value("LastBind").toInt() == b.turn() || turn(b,s).contains("ForceBind")) {
            turn(b,s)["ImpossibleToMove"] = true;
        }
    }

    static void uas2(int s, int t, BS &b) {
        // If the opponent switched, we start all over again
        if (!poke(b,t).contains("Bound")) {
            uas(s, t, b);
            return;
        }
        poke(b,s)["LastBind"] = b.turn();
        inc(poke(b,s)["BindCount"], -1);

        int count = poke(b,s)["BindCount"].toInt();

        if (count == 0) {
            poke(b,s).remove("BindCount");
            poke(b,t).remove("Bound");
            removeFunction(poke(b,s), "TurnSettings", "Bind");
            //RBY doesn't notify when Wrap ends
            //b.sendMoveMessage(10, 1, t, type(b,s), s, move(b,s));
        }
    }
};

struct RBYCounter : public MM
{
    RBYCounter() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void ms(int s, int t, BS &b) {
        /* If both use counter, it misses */
        t = b.opponent(s);
        if (move(b, t) == move(b, s) || turn(b,s).contains("MetronomeCall")) {
            tmove(b,s).accuracy = -1;
        }
    }

    static void daf(int s, int t, BS &b) {
        int mv = (b.gen() <= Pokemon::gen(Gen::Yellow) ? slot(b, t).lastMoveUsed : fpoke(b,t).lastMoveUsed);

        if (mv == 0) {
            fturn(b,s).add(TM::Failed);
            return;
        }

        int type = MoveInfo::Type(mv, b.gen());

        if (type != Type::Normal && type != Type::Fighting) {
            fturn(b,s).add(TM::Failed);
            return;
        }

        int damage = poke(b, s).value("DamageReceived").toInt();

        if (damage == 0) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void cad(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = 2 * poke(b,s).value("DamageReceived").toInt();
    }
};

struct RBYDig : public MM
{
    RBYDig() {
        functions["MoveSettings"] = &ms;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        fturn(b, s).add(TM::BuildUp);
        tmove(b,s).targets = Move::User;
    }

    static void uas(int s, int t, BS &b) {
        int arg = turn(b,s)["Dig_Arg"].toInt();

        b.sendMoveMessage(13,arg,s,type(b,s), t);

        poke(b,s)["DigChargeTurn"] = b.turn();
        poke(b,s)["Invulnerable"] = true;
        poke(b,s)["ChargeMove"] = move(b,s);
        b.changeSprite(s, -1);

        addFunction(poke(b,s), "TurnSettings", "Dig", &ts);
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s).value("DigChargeTurn") != b.turn()-1) {
            return;
        }
        //Dig uses its PP on the second turn
        fturn(b,s).add(TM::UsePP);
        fturn(b,s).add(TM::NoChoice);
        /* To restore the RBY paralysis glitch with fly, change TrueEnd to AttackSomehowFailed
          and uncomment the line after next*/
        addFunction(turn(b,s), "TrueEnd", "Dig", &asf);
        //addFunction(turn(b,s), "UponAttackSuccessful", "Dig", &asf);
        turn(b,s)["AutomaticMove"] = poke(b,s).value("ChargeMove");
        initMove(poke(b,s).value("ChargeMove").toInt(), b.gen(), tmove(b,s));
    }

    static void asf(int s, int, BS &b) {
        poke(b,s).remove("Invulnerable");
        b.changeSprite(s, 0);
    }
};

struct RBYDisable : public MM
{
    RBYDisable() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
        functions["AttackSomehowFailed"] = &asf;
    }

    static void daf(int s, int t, BS &b) {
        if (poke(b,t).contains("DisableCount")) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        bool enoughPPs = false;
        for (int i = 0; i < 4; i++) {
            if (b.PP(t, i) > 0) {
                enoughPPs = true;
            }
        }
        if (!enoughPPs) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void uas(int s, int t, BS &b) {
        int count = 1 + b.randint(8); //1-8 count
        QVector<int> possibilities;
        for (int i = 0; i < 4; i++) {
            if (b.PP(t, i) > 0) {
                possibilities.push_back(i);
            }
        }
        if (possibilities.size() == 0) {
            return; //Should be impossible
        }
        int slot = possibilities[b.randint(possibilities.size())];

        poke(b,t)["DisableCount"] = count;
        poke(b,t)["DisableSlot"] = slot;

        addFunction(poke(b,t), "MovePossible", "Disable", &mp);
        addFunction(poke(b,t), "MovesPossible", "Disable", &msp);

        b.sendMoveMessage(28, 0, s, 0, t, b.move(t, slot));
        b.callpeffects(t, s, "UponOffensiveDamageReceived");
    }

    static void asf(int s, int t, BS &b) {
        //RBY Bug: Disable builds up rage whenever
        b.callpeffects(t, s, "UponOffensiveDamageReceived");
    }

    static void mp(int s, int , BS &b) {
        inc(poke(b,s)["DisableCount"], -1);
        int slot = poke(b,s).value("DisableSlot").toInt();

        if (poke(b,s).value("DisableCount").toInt() <= 0) {
            poke(b,s).remove("DisableCount");
            removeFunction(poke(b,s), "MovePossible", "Disable");
            removeFunction(poke(b,s), "MovesPossible", "Disable");
            b.sendMoveMessage(28, 2, s, 0, s, b.move(s, slot));
            return;
        }

        if (move(b,s) == b.move(s, slot)) {
            b.sendMoveMessage(28, 1, s, 0, s, b.move(s, slot));
            turn(b,s)["ImpossibleToMove"] = true;
        }
    }

    static void msp(int s, int , BS &b) {
        if (!poke(b,s).contains("DisableCount")) {
            return;
        }
        poke(b,s)[QString("Move%1Blocked").arg(poke(b,s)["DisableSlot"].toInt())] = true;
    }
};

struct RBYDragonRage : public MM
{
    RBYDragonRage() {
        functions["CustomAttackingDamage"] = &cad;
    }

    static void cad(int s, int , BS &b) {
        turn(b,s)["CustomDamage"] = turn(b,s)["DragonRage_Arg"];
    }
};

struct RBYDreamEater : public MM
{
    RBYDreamEater() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int t, BS &b) {
        if (b.poke(t).status() != Pokemon::Asleep) {
            tmove(b,s).accuracy = -1;
        }
    }
};

struct RBYExplosion : public MM
{
    RBYExplosion() {
        functions["UponAttackSuccessful"] = &uas;
        functions["AttackSomehowFailed"] = &asf;
    }

    static void uas(int s, int t, BS &b) {
        /* Explosion doesn't faint the user if it breaks a sub.
         * However, it faints all the time in Stadium. */
        if (b.gen() <= Pokemon::gen(Gen::Yellow) && b.hadSubstitute(t))
            return;

        b.selfKoer() = s;
        b.koPoke(s, s);
    }

    static void asf(int s, int, BS &b) {
        b.selfKoer() = s;
        b.koPoke(s, s);
    }
};

struct RBYFocusEnergy : public MM
{
    RBYFocusEnergy() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).contains("Focused")) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(46, 0, s);
        poke(b,s)["Focused"] = true;
        addFunction(poke(b,s), "TurnSettings", "FocusEnergy", &ts);
    }

    static void ts(int s, int, BS &b) {
        addFunction(turn(b,s), "MoveSettings", "FocusEnergy", &ms);
    }

    static void ms(int s, int, BS &b) {
        if (poke(b,s).contains("Focused")) {
            tmove(b,s).critRaise+=2;
        }
    }
};

struct RBYHaze : public MM
{
    RBYHaze() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        int t = b.opponent(s);
        b.healStatus(t, b.poke(t).status());

        for (int i = Attack; i < AllStats; i++) {
            b.changeStatMod(s, i, 0);
            b.changeStatMod(t, i, 0);
            fpoke(b,s).stats[i] = b.getBoostedStat(s, i);
            fpoke(b,t).stats[i] = b.getBoostedStat(t, i);
        }

        b.healConfused(s);
        b.healConfused(t);

        poke(b,s).remove("Barrier1Count");
        poke(b,s).remove("Barrier2Count");
        poke(b,t).remove("Barrier1Count");
        poke(b,t).remove("Barrier2Count");

        poke(b,s).remove("Focused");
        poke(b,t).remove("Focused");

        poke(b,s).remove("Misted");
        poke(b,t).remove("Misted");

        b.poke(s).removeStatus(Pokemon::Seeded);
        b.poke(t).removeStatus(Pokemon::Seeded);

        //Haze clears major status that the user has in Stadium
        if (b.gen() > Pokemon::gen(Gen::Yellow)) {
            b.changeStatus(s, Pokemon::Fine,false);
        }
    }
};

struct RBYHiJumpKick : public MM
{
    RBYHiJumpKick() {
        functions["AttackSomehowFailed"] = &asf;
    }

    static void asf(int s, int, BS &b) {
        //1 damage if you miss the attack
        b.inflictDamage(s, 1, s, true);
    }
};

struct RBYHyperBeam : public MM
{
    RBYHyperBeam() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        /*Hyper Beam always needs to recharge in Stadium
         *KOs and sub breaks dont cause recharge in RBY*/
        if (b.gen() <= Pokemon::gen(Gen::Yellow) && (b.koed(t) || b.hadSubstitute(t)))
            return;

        poke(b,s)["Recharging"] = b.turn()+1;
        addFunction(poke(b,s), "TurnSettings", "HyperBeam", &ts);
    }

    static void ms(int s, int, BS &b) {
        if (!poke(b,s).contains("Recharging")) {
            //Opponent used bind, Restore hyper beam
            fturn(b,s).add(TM::UsePP);
            tmove(b,s).attack = fpoke(b,s).lastMoveUsed;
        } else {
            turn(b, s)["TellPlayers"] = false;
            tmove(b, s).targets = Move::User;
            addFunction(turn(b,s), "UponAttackSuccessful", "HyperBeam", &aas);
        }
    }

    static void aas(int s, int, BS &b) {
        b.sendMoveMessage(11, 0, s);
    }

    static void ts(int s, int, BS &b) {
        if (!poke(b,s).contains("Recharging") || poke(b,s)["Recharging"].toInt() != b.turn()) {
            return;
        }

        fturn(b, s).add(TM::NoChoice);
        turn(b,s)["AutomaticMove"] = 0;//So that confusion won't be inflicted on recharge

        addFunction(turn(b,s), "MoveSettings", "HyperBeam", &ms);
    }
};

struct RBYLeechSeed : public MM
{
    RBYLeechSeed() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasType(t, Type::Grass) || b.poke(t).hasStatus(Pokemon::Seeded)) {
            b.failSilently(s);
            b.sendMoveMessage(72, 0, s, Type::Grass);
        }
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(72, 1, s, Type::Grass, t);
        b.poke(t).addStatus(Pokemon::Seeded);
    }
};

struct RBYLightScreen : public MM
{
    RBYLightScreen() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        int cat = turn(b,s)["LightScreen_Arg"].toInt();
        if (poke(b,s).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int cat = turn(b,s)["LightScreen_Arg"].toInt();

        b.sendMoveMessage(73,(cat-1)+b.multiples()*2,s,type(b,s));
        poke(b,s)["Barrier" + QString::number(cat) + "Count"] = 1;
    }
};

struct RBYMetronome : public MM
{
    RBYMetronome() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "Metronome");
        turn(b,s)["MetronomeCall"] = true;

        while (1) {
            int move = b.randint(MoveInfo::NumberOfMoves());

            if (!MoveInfo::Exists(move, b.gen())) {
                continue;
            }

            bool correctMove = move != 0 && move != Move::Struggle && move != Move::Metronome;

            if (correctMove) {
                BS::BasicMoveInfo info = tmove(b,s);
                RBYMoveEffect::setup(move,s,t,b);
                b.useAttack(s,move,true,true);
                RBYMoveEffect::unsetup(move, s, b);
                tmove(b,s) = info;
                break;
            }
        }
    }
};

struct RBYMimic : public MM
{
    RBYMimic() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        int move = 0;
        /* Mimic copies a random move in Gen 1 */
        while (move == 0) {
            move = b.move(t, b.randint(4));
        }
        int slot = fpoke(b,s).lastMoveSlot;
        b.changeTempMove(s, slot, move);
        b.sendMoveMessage(81,0,s,type(b,s),t,move);
    }
};

struct RBYMirrorMove : public MM
{
    RBYMirrorMove() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        int lastMove = fpoke(b,t).lastMoveUsed;

        if (lastMove == 0 || lastMove == Move::MirrorMove) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "DetermineAttackFailure", "MirrorMove");
        removeFunction(turn(b,s), "UponAttackSuccessful", "MirrorMove");

        int move = fpoke(b,t).lastMoveUsed;
        BS::BasicMoveInfo info = tmove(b,s);
        RBYMoveEffect::setup(move,s,s,b);
        b.useAttack(s,move,true,true);
        RBYMoveEffect::unsetup(move,s,b);
        tmove(b,s) = info;
    }
};


struct RBYMist : public MM
{
    RBYMist() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"]=  &daf;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).contains("Misted"))
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(86,0,s,Pokemon::Ice);

        poke(b,s)["Misted"] = true;
    }
};

struct RBYNightShade : public MM
{
    RBYNightShade() {
        functions["CustomAttackingDamage"] = &cad;
    }

    static void cad(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = b.poke(s).level();
    }
};

struct RBYPetalDance : public MM
{
    RBYPetalDance() {
        functions["MoveSettings"] = &ms;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        poke(b,s)["PetalDanceCount"] = 3 + b.randint(2);
        addFunction(poke(b,s), "TurnSettings", "PetalDance", &ts);
    }

    static void uas(int s, int, BS &b) {
        inc(poke(b,s)["PetalDanceCount"], -1);
        if (poke(b,s).value("PetalDanceCount").toInt() <= 0) {
            poke(b,s).remove("PetalDanceCount");
            b.inflictConfused(s, s, true);
        }
    }

    static void ts(int s, int, BS &b) {
        if (!poke(b,s).contains("PetalDanceCount")) {
            return;
        }
        RBYMoveMechanics::initMove(fpoke(b,s).lastMoveUsed, b.gen(), tmove(b,s));
        addFunction(turn(b,s), "UponAttackSuccessful", "PetalDance", &uas);
        addFunction(turn(b,s), "AttackSomehowFailed", "PetalDance", &uas);
        fturn(b,s).add(TM::NoChoice);
    }
};

struct RBYPsywave : public MM
{
    RBYPsywave() {
        functions["CustomAttackingDamage"] = &cad;
    }

    static void cad (int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = 1 + b.randint(fpoke(b,s).level * 15/10);
    }
};

struct RBYRage : public MM
{
    RBYRage() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "TurnSettings", "Rage", &ts);
        addFunction(poke(b,s), "UponOffensiveDamageReceived", "Rage", &uodr);
    }

    static void ts(int s, int, BS &b) {
        fturn(b,s).add(TM::NoChoice);
        addFunction(turn(b,s), "AttackSomehowFailed", "Rage", &asf);

        initMove(fpoke(b,s).lastMoveUsed, b.gen(), tmove(b,s));
        if (poke(b,s).contains("RageFailed")) {
            tmove(b,s).accuracy = 1;
        }
    }

    static void uodr(int s, int, BS &b) {
        if (!b.koed(s)) {
            b.gainStatMod(s, Attack, 1, s, false);
            b.sendMoveMessage(102, 0, s);
        }
    }

    static void asf(int s, int, BS &b) {
        poke(b,s)["RageFailed"] = true;
    }
};

struct RBYRest : public MM
{
    RBYRest() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.poke(s).totalLifePoints() == b.poke(s).lifePoints() || (b.poke(s).totalLifePoints()-b.poke(s).lifePoints()) % 256 == 255) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        b.healLife(s, b.poke(s).totalLifePoints());
        b.sendMoveMessage(106,0,s,type(b,s));
        b.changeStatus(s, Pokemon::Asleep,false);
        b.poke(s).statusCount() =  2;
        b.poke(s).oriStatusCount() = 2;
    }
};

struct RBYRazorWind : public MM
{
    RBYRazorWind() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        fturn(b,s).add(TM::BuildUp);

        int mv = move(b,s);

        b.sendMoveMessage(104, turn(b,s)["RazorWind_Arg"].toInt(), s, type(b,s));
        /* Skull bash */

        poke(b,s)["ChargingMove"] = mv;
        poke(b,s)["ReleaseTurn"] = b.turn() + 1;
        turn(b,s)["TellPlayers"] = false;
        tmove(b, s).power = 0;
        tmove(b, s).status = Pokemon::Fine;
        tmove(b, s).targets = Move::User;
        addFunction(poke(b,s), "TurnSettings", "RazorWind", &ts);
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s).value("ReleaseTurn").toInt() != b.turn()) {
            return;
        }
        fturn(b,s).add(TM::NoChoice);
        fturn(b,s).add(TM::UsePP);
        int mv = poke(b,s)["ChargingMove"].toInt();
        initMove(mv, b.gen(), tmove(b, s));
        turn(b,s)["AutomaticMove"] = mv;
    }
};

struct RBYSubstitute : public MM
{
    RBYSubstitute() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (fpoke(b,s).substitute()) {
            b.failSilently(s);
            b.sendMoveMessage(128, 0, s,0,s);
            return;
        }
        if (b.poke(s).lifePoints() < b.poke(s).totalLifePoints()/4) {
            b.failSilently(s);
            b.sendMoveMessage(8,0,s);
            return;
        }
    }

    static void uas(int s, int, BS &b) {
        int newHp = b.poke(s).lifePoints() - std::max(b.poke(s).totalLifePoints()/4, 1);
        if (newHp < 0) {
            newHp = 0;
        }
        b.changeHp(s, newHp);
        if (b.koed(s)) {
            b.koPoke(s, s);
        } else {
            fpoke(b,s).add(BS::BasicPokeInfo::Substitute);
            fpoke(b,s).substituteLife = b.poke(s).totalLifePoints()/4+1;
            b.sendMoveMessage(128,4,s);
            b.notifySub(s,true);
        }
    }
};

struct RBYSuperFang : public MM
{
    RBYSuperFang() {
        functions["CustomAttackingDamage"] = &cad;
    }

    static void cad(int s, int t, BS &b) {
        turn(b,s)["CustomDamage"] = std::max(int(b.poke(t).lifePoints()/2), 1);
    }
};
struct RBYConversion : public MM
{
    RBYConversion() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        fpoke(b,s).type1 = fpoke(b,t).type1;
        fpoke(b,s).type2 = fpoke(b,t).type2;
        fpoke(b,s).types = fpoke(b,t).types;
    }
};

struct RBYTransform : public MM
{
    RBYTransform() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        /* Give new values to what needed */
        Pokemon::uniqueId num = b.pokenum(t);

        b.sendMoveMessage(137,0,s,0,s,num.pokenum);

        BS::BasicPokeInfo &po = fpoke(b,s);
        BS::BasicPokeInfo &pt = fpoke(b,t);

        po.id = num;
        po.weight = PokemonInfo::Weight(num);
        //For Type changing moves
        po.types = b.getTypes(t);
        //po.type1 = PokemonInfo::Type1(num, b.gen());
        //po.type2 = PokemonInfo::Type2(num, b.gen());

        b.changeSprite(s, num);

        for (int i = 0; i < 4; i++) {
            b.changeTempMove(s,i,b.move(t,i));
            if (b.move(t,i) == Move::NoMove) { continue; }
            b.changePP(s, i, 5);
        }

        for (int i = 1; i < 6; i++)
            po.stats[i] = pt.stats[i];

        for (int i = 0; i < 8; i++) {
            po.boosts[i] = pt.boosts[i];
        }
    }
};

#define REGISTER_MOVE(num, name) mechanics[num] = RBY##name(); names[num] = #name; nums[#name] = num;

void RBYMoveEffect::init()
{
    REGISTER_MOVE(9, Bide);
    REGISTER_MOVE(10, Bind);
    REGISTER_MOVE(11, HyperBeam);
    REGISTER_MOVE(19, Conversion);
    REGISTER_MOVE(22, Counter);
    REGISTER_MOVE(13, Dig);
    REGISTER_MOVE(28, Disable);
    REGISTER_MOVE(30, DragonRage);
    REGISTER_MOVE(31, DreamEater);
    REGISTER_MOVE(37, Explosion);
    REGISTER_MOVE(46, FocusEnergy);
    REGISTER_MOVE(64, HiJumpKick);
    REGISTER_MOVE(72, LeechSeed);
    REGISTER_MOVE(73, LightScreen);
    REGISTER_MOVE(80, Metronome);
    REGISTER_MOVE(81, Mimic);
    REGISTER_MOVE(85, MirrorMove);
    REGISTER_MOVE(86, Mist);
    REGISTER_MOVE(91, NightShade);
    REGISTER_MOVE(93, PetalDance);
    REGISTER_MOVE(99, Psywave);
    REGISTER_MOVE(102, Rage);
    REGISTER_MOVE(104, RazorWind);
    REGISTER_MOVE(106, Rest);
    REGISTER_MOVE(128, Substitute);
    REGISTER_MOVE(130, SuperFang);
    REGISTER_MOVE(137, Transform);
    REGISTER_MOVE(149, Haze);
}

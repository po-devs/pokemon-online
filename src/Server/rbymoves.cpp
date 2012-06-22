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
        functions["UponAttackSuccessful"] = &uas;
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
        if (poke(b,s).value("LastBind").toInt() == b.turn()-1 && poke(b,s).value("BindCount").toInt() > 0) {
            fturn(b,s).add(TM::KeepAttack);
            addFunction(turn(b,s), "UponAttackSuccessful", "Bind", &uas2);
            addFunction(turn(b,s), "EvenWhenCantMove", "Bind", &ewcm);
            /* Bind does the same damage every turn */
            addFunction(turn(b,s), "CustomAttackingDamage", "Bind", &cad);
        }
    }

    static void cad(int s, int t, BS &b) {
        if (poke(b,t).contains("Bound")) {
            turn(b,s)["CustomDamage"] = poke(b,s)["BindDamage"];
        }
    }

    static void ewcm(int s, int, BS &b) {
        int t = b.opponent(s);

        if (!poke(b,t).contains("Bound")) {
            fturn(b,s).add(TM::UsePP); //If the opponent switched out, we use an additional PP
        }
    }

    static void mp(int s, int t, BS &b) {
        t = b.opponent(s);
        /* Either Bind was used last turn and is ongoing, or was used this turn (and may have finished) */
        if (( (poke(b,s).contains("Bound") || poke(b,t).contains("BindCount")) && poke(b,t).value("LastBind").toInt() >= b.turn()-1) || poke(b,t).value("LastBind").toInt() == b.turn()) {
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
        inc(poke(b,t)["BindCount"], -1);

        int count = poke(b,t)["BindCount"].toInt();

        if (count == 0) {
            poke(b,s).remove("BindCount");
            poke(b,t).remove("Bound");
            removeFunction(poke(b,s), "TurnSettings", "Bind");
            removeFunction(poke(b,t), "MovePossible", "Bind");
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
        if (move(b, t) == move(b, s)) {
            tmove(b,s).accuracy = -1;
        }
    }

    static void daf(int s, int t, BS &b) {
        int type = MoveInfo::Type(fpoke(b,t).lastMoveUsed, b.gen());

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
        functions["UponAttackSucessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        fturn(b, s).add(TM::BuildUp);
    }

    static void uas(int s, int t, BS &b) {
        int arg = turn(b,s)["Dig_Arg"].toInt();

        b.sendMoveMessage(13,arg,s,type(b,s), t);

        poke(b,s)["DigChargeTurn"] = b.turn();
        poke(b,s)["Invulnerable"] = true;
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
        addFunction(turn(b,s), "AttackSomehowFailed", "Dig", &asf);
        addFunction(turn(b,s), "UponAttackSucessful", "Dig", &uas2);
    }

    static void asf(int s, int, BS &b) {
        poke(b,s).remove("Invulnerable");
        b.changeSprite(s, 0);
    }

    static void uas2(int s, int t, BS &b) {
        asf(s, t, b);
    }
};

struct RBYDisable : public MM
{
    RBYDisable() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
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
    }

    static void mp(int s, int , BS &b) {
        inc(poke(b,s)["DisableCount"], -1);
        int slot = poke(b,s).value("DisableSlot").toInt();

        if (poke(b,s).value("DisableCount").toInt() <= 0) {
            poke(b,s).remove("DisableCount");
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

#define REGISTER_MOVE(num, name) mechanics[num] = RBY##name(); names[num] = #name; nums[#name] = num;

void RBYMoveEffect::init()
{
    REGISTER_MOVE(9, Bide);
    REGISTER_MOVE(10, Bind);
    REGISTER_MOVE(22, Counter);
    REGISTER_MOVE(13, Dig);
    REGISTER_MOVE(28, Disable);
    REGISTER_MOVE(30, DragonRage);
    REGISTER_MOVE(31, DreamEater);
}

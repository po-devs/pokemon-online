#include "../Shared/battlecommands.h"
#include <PokemonInfo/pokemoninfo.h>
#include "moves.h"
#include "miscabilities.h"
#include "items.h"
#include "battlecounterindex.h"
#include "miscmoves.h"

QHash<int, MoveMechanics> MoveEffect::mechanics;
QHash<int, QString> MoveEffect::names;
QHash<QString, int> MoveEffect::nums;
typedef BS::priorityBracket bracket;

Q_DECLARE_METATYPE(QList<int>)

using namespace Move;
typedef BattleCounterIndex BC;
typedef BS::TurnMemory TM;

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

void MoveEffect::setup(int num, int source, int target, BattleBase &b)
{
    /* first the basic info */
    MM::initMove(num, b.gen(), MM::tmove(b,source));

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
                    Mechanics::addFunction(MM::turn(b,source), i.key(), n, i.value());
                }
            }
        }
    }
}

/* Used by moves like Metronome that may use moves like U-Turn. Then AfterAttackSuccessful would be called twice, and that would
    not be nice because U-Turning twice :s*/
void MoveEffect::unsetup(int num, int source, BattleBase &b)
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
                    Mechanics::removeFunction(MM::turn(b,source), i.key(), n);
                }
            }
        }
    }

    MM::tmove(b,source).classification = Move::StandardMove;
}

/* List of events:
    *UponDamageInflicted -- turn: just after inflicting damage
    *DetermineAttackFailure -- turn, poke: set fturn(b,s).add(TM::Failed) to true to make the attack fail
    *DetermineGeneralAttackFailure -- battle: this is a battlefield effect, same as above
    *EndTurn -- poke: Called at the end of the turn
    *UponOffensiveDamageReceived -- turn: when the player received damage (not the substitute) from an attack
    *OnSetup -- none: when the move is setup
    *TurnSettings -- poke: Will be called at the beginning of the turn before even chosing moves.
    *EvenWhenCantMove -- turn: Will be called before even status check, useful for attacks like fly etc
    *BeforeTakingDamage -- turn: explicit
    *UponSwitchIn -- turn: When a new poke is switched in, like for Baton Pass/U-turn
    *MoveSettings -- turn: Just after losing PP, and before chosing the target
    *BeforeTargetList -- turn: Before processing the attack for each target
    *BeforeCalculatingDamage -- turn: The right moment to change the base power of the attack if needed
    *CustomAttackingDamage -- turn: If the attack does a certain amount of damage, without regard to things like base power, inflict it here
    *UponAttackSuccessful -- turn: after inflicting damage (and damage effects called) / just after succeeding the move if the move has 0 BP
    *AfterAttackSuccessful -- turn: at the very end of the attack for that target
    *BeforeHitting -- turn: for things that have 0 BP, this is called instead of BeforeCalculatingDamage & stuff
    *DetermineAttackPossible -- poke: just say if the poke is supposed to be able to attack, regarless of the the move used (like attracted pokes won't attack)
    *MovePossible -- turn: before attacking, say if the move is possible or not (like when a move just got blocked by encore, taunt,disable)
    *MovesPossible -- poke: at the beginning of the turn, tells if each move is possible or not
    *AfterKoedByStraightAttack -- poke: when koed by an attack
    *BlockTurnEffects -- poke: Called before calling effects for a turn event, to see if it's blocked. Used by Substitute
*/

struct MMAquaRing : public MM
{
    MMAquaRing() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).contains("AquaRinged"))
            fturn(b,s).add(TM::Failed);
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 1) : makeBracket(6, 0) ;
    }

    static void uas(int s, int, BS &b) {
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "AquaRing", &et);
        poke(b,s)["AquaRinged"] = true;
        b.sendMoveMessage(2, 0, s, type(b,s));
    }

    static void et(int s, int, BS &b) {
        if (!b.canHeal(s,BS::HealByEffect,0))
            return;

        int healing = b.poke(s).totalLifePoints()/16;
        if (b.hasWorkingItem(s, Item::BigRoot)) {
            healing = healing * 13 / 10;
        }

        b.sendMoveMessage(2, 1, s, Pokemon::Water);
        b.healLife(s, healing);
    }
};

struct MMAssurance : public MM
{
    MMAssurance() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (turn(b,t).contains("DamageTaken") || (team(b, b.player(t)).contains("LastKoedTurn") && team(b, b.player(t))["LastKoedTurn"].toInt() == b.turn() - 1)) {
                    tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMBatonPass : public MM
{
    MMBatonPass() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
        // the function has to be in that list when called by metronome, so it can be unsetup
        functions["AfterAttackFinished"] = &aaf;
    }

    static void daf(int s, int, BS &b) {
        if (b.countBackUp(b.player(s)) == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b)
    {
        turn(b,s)["BatonPassSuccess"] = true;
    }

    static void aaf(int s, int, BS &b) {
        if (!turn(b,s).contains("BatonPassSuccess"))
            return;
        /* first we copy the temp effects, then put them to the next poke */
        BS::context c = poke(b, s);
        c.remove("YawnCount");
        c.remove("Minimize");
        c.remove("DefenseCurl");
        c.remove("AbilityArg");
        c.remove("ItemArg");
        c.remove("Illusioned");
        /* removing last resort memory */
        c.remove("Move0Used");
        c.remove("Move1Used");
        c.remove("Move2Used");
        c.remove("HadItem");
        c.remove("Move3Used");
        c.remove("HasMovedOnce");
        /* Removing attract */
        c.remove("AttractBy");
        foreach( int opp, b.revs(s)) {
            if (b.linked(opp, "Attract"))
                poke(b, opp).remove("AttractBy");
        }

        QList<int> boosts;

        for(int i = 0; i < 8; i++) {
            boosts.push_back(fpoke(b,s).boosts[i]);
        }
        turn(b,s)["BatonPassBoosts"] = QVariant::fromValue(boosts);
        turn(b,s)["BatonPassFlags"] = fpoke(b,s).flags & BS::BasicPokeInfo::Substitute;
        turn(b,s)["BatonPassLife"] = fpoke(b,s).substituteLife;

        /* choice band etc. would force the same move
            if on both the passed & the passer */
        c.remove("ChoiceMemory");

        turn(b,s)["BatonPassData"] = c;
        turn(b,s)["BatonPassed"] = true;

        addFunction(turn(b,s), "UponSwitchIn", "BatonPass", &usi);
        b.requestSwitch(s);
    }

    static void usi(int s, int, BS &b) {
        if (turn(b,s)["BatonPassed"].toBool() == false) {
            return;
        }
        turn(b,s)["BatonPassed"] = false;
        merge(poke(b,s), turn(b,s)["BatonPassData"].value<BS::context>());

        /* If the poke before is confused, carry on that status */
        if (poke(b,s)["ConfusedCount"].toInt() > 0) {
            if (!b.poke(s).hasStatus(Pokemon::Confused))
                b.poke(s).addStatus(Pokemon::Confused);
        }

        QList<int> boosts = turn(b,s)["BatonPassBoosts"].value<QList<int> >();
        for (int i = 0; i < 8; i++) {
            fpoke(b,s).boosts[i] += boosts[i];
        }

        fpoke(b, s).flags |= turn(b,s)["BatonPassFlags"].toInt();
        fpoke(b, s).substituteLife = turn(b,s)["BatonPassLife"].toInt();

        if (b.gen() <= 4) {
            //and we decrease the switch count associated, so mean look & co still work
            inc(slot(b,s)["SwitchCount"], -1);
        }

        if (fpoke(b,s).substitute()) {
            b.notifySub(s,true);
        }
    }
};

struct MMBugBite : public MM
{
    MMBugBite() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        int item = b.poke(t).item();
        if (!ItemInfo::isBerry(item))
            return;
        if (b.hasWorkingAbility(t, Ability::StickyHold)) {
            b.sendAbMessage(121,1,t);
            return;
        }
        b.sendMoveMessage(16,0,s,type(b,s),t,item);
        b.devourBerry(s, item, s);
        if (b.gen() >= 5) {
            b.loseItem(t);
        } else {
            //Gen 4 BugBite/Pluck allows restoration of the berry
            b.disposeItem(t);
        }
    }
};

struct MMCamouflage : public MM {
    MMCamouflage() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        int type = Pokemon::Normal;
        if (b.gen().num == 5) {
            type = Pokemon::Ground;
        }
        if (b.terrain != 0) {
            type = std::abs(b.terrain);
        }
        //Move fails if the type you're transforming to is EXACTLY your typing.
        //Includes type changing moves like Soak in this check
        foreach (int check, fpoke(b,s).types) {
            /*Ignore Curse*/
            if (check == Pokemon::Curse)
                continue;
            /*Check if a user has a type other than the type you're converting into*/
            if (check != type) {
                turn(b,s)["CamouflageType"] = type;
                return;
            }
        }
        fturn(b, s).add(TM::Failed);
    }

    static void uas (int s, int, BS &b) {
        int type = turn(b,s)["CamouflageType"].toInt();
        b.setType(s, type);
        b.sendMoveMessage(17,0,s,type);
    }
};

struct MMBlastBurn : public MM
{
    MMBlastBurn() {
        functions["UponAttackSuccessful"] = &uas;
        functions["AttackSomehowFailed"] = &asf;
    }

    // Hyper Beam requires a recharge in Gen 1 Stadium even if it misses.
    static void asf(int s, int, BS &b) {
        if (b.gen() >= 2) {
            return;
        }
        addFunction(poke(b, s), "TurnSettings", "BlastBurn", &ts);
        poke(b, s)["BlastBurnTurn"] = b.turn();
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b, s), "TurnSettings", "BlastBurn", &ts);
        poke(b, s)["BlastBurnTurn"] = b.turn();
    }

    static void ts(int s, int, BS &b) {
        if (poke(b, s)["BlastBurnTurn"].toInt() != b.turn() - 1) {
            return;
        }

        fturn(b, s).add(TM::NoChoice);
        turn(b,s)["AutomaticMove"] = 0;//So that confusion won't be inflicted on recharge

        addFunction(turn(b,s), "MoveSettings", "BlastBurn", &ms);
    }

    static void ms(int s, int, BS &b) {
        turn(b, s)["TellPlayers"] = false;
        tmove(b, s).targets = Move::User;
        addFunction(turn(b,s), "UponAttackSuccessful", "BlastBurn", &aas);
    }

    static void aas(int s, int, BS &b) {
        b.sendMoveMessage(11, 0, s);
    }
};

struct MMBrine : public MM
{
    MMBrine() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (b.poke(t).lifePercent() <= 50) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMCharge : public MM
{
    MMCharge() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        poke(b, s)["Charged"] = true;
        addFunction(poke(b,s), "BeforeCalculatingDamage", "Charge", &bcd);
        b.sendMoveMessage(18, 0, s, type(b,s));
    }

    static void bcd(int s, int, BS &b) {
        if (poke(b,s)["Charged"] == true && tmove(b,s).type == Type::Electric) {
            if (tmove(b, s).power > 0) {
                tmove(b, s).power = tmove(b, s).power * 2;
                poke(b, s)["Charged"] = false;
            }
        }
    }
};

struct MMConversion : public MM
{
    MMConversion() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        /* Conversion doesn't fail in Gen 1 */
        if (b.gen().num == 1) {
            return;
        }
        /* First check if there's even 1 move available */
        for (int i = 0; i < 4; i++) {
            if (MoveInfo::Type(b.move(s,i), b.gen()) != Type::Curse) {
                break;
            }
            if (i == 3) {
                fturn(b, s).add(TM::Failed);
                return;
            }
        }
        if (fpoke(b,s).type2 != Pokemon::Curse) {
            /* It means the pokemon has two types, i.e. conversion always works */
            QList<int> poss;
            for (int i = 0; i < 4; i++) {
                if (MoveInfo::Type(b.move(s,i), b.gen()) != Type::Curse) {
                    poss.push_back(b.move(s,i));
                }
            }
            turn(b,s)["ConversionType"] = MoveInfo::Type(poss[b.randint(poss.size())], b.gen());
        } else {
            QList<int> poss;
            for (int i = 0; i < 4; i++) {
                if (MoveInfo::Type(b.move(s,i), b.gen()) != Type::Curse && MoveInfo::Type(b.move(s,i), b.gen()) != fpoke(b,s).type1) {
                    poss.push_back(b.move(s,i));
                }
            }
            if (poss.size() == 0) {
                fturn(b, s).add(TM::Failed);
            } else {
                turn(b,s)["ConversionType"] = MoveInfo::Type(poss[b.randint(poss.size())], b.gen());
            }
        }
    }

    static void uas(int s, int t, BS &b) {
        /* Conversion changes the user's types to the opponent's types in Gen 1*/
        if (b.gen().num == 1) {
            b.sendMoveMessage(172,0,s,type(b,s),t);
            fpoke(b,s).type1 = fpoke(b,t).type1;
            fpoke(b,s).type2 = fpoke(b,t).type2;
        }
        else {
            int type = turn(b,s)["ConversionType"].toInt();
            b.setType(s, type);
            b.sendMoveMessage(19, 0, s, type, s);
        }
    }
};

struct MMConversion2 : public MM
{
    MMConversion2() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        int attackType;
        if (b.gen() <= 4) {
            if (!poke(b,s).contains("LastAttackToHit"))
            {
                fturn(b,s).add(TM::Failed);
                return;
            }

            attackType = MoveInfo::Type(poke(b,s)["LastAttackToHit"].toInt(), b.gen());
        } else {
            if (fpoke(b,t).lastMoveUsed == 0) {
                fturn(b,s).add(TM::Failed);
                return;
            }

            attackType = MoveInfo::Type(fpoke(b,t).lastMoveUsed, b.gen());
        }

        if (attackType == Type::Curse) {
            fturn(b,s).add(TM::Failed);
            return;
        }

        /* Gets types available */
        QList<int> poss;
        for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
            QVector<int> types = b.getTypes(s);
            if (!(types[0] == i && types.count() == 1) && TypeInfo::Eff(attackType, i) < Type::Effective) {
                poss.push_back(i);
            }
        }
        if (poss.size() == 0) {
            fturn(b,s).add(TM::Failed);
        } else {
            turn(b,s)["Conversion2Type"] = poss[b.randint(poss.size())];
        }
    }

    static void uas(int s, int, BS &b) {
        int type = turn(b,s)["Conversion2Type"].toInt();
        b.setType(s, type);
        b.sendMoveMessage(20, 0, s, type, s);
    }
};

struct MMCrushGrip : public MM
{
    MMCrushGrip() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        tmove(b, s).power = tmove(b, s).power * std::max(1, 120 * b.poke(t).lifePoints()/b.poke(t).totalLifePoints());
    }
};

struct MMCurse : public MM
{
    MMCurse() {
        functions["DetermineAttackFailure"] = &daf;
        functions["MoveSettings"] = &ms;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (turn(b,s).value("CurseGhost").toBool() && poke(b,t).value("Cursed").toBool()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void ms(int s, int, BS &b) {
        if (!b.hasType(s, Pokemon::Ghost)) {
            tmove(b,s).targets = Move::User; // so that curse works even when there is no enemy.
            tmove(b,s).classification = Move::StatChangingMove;
            tmove(b,s).statAffected = (Attack << 16) + (Defense << 8) + Speed;
            tmove(b,s).boostOfStat = (1 << 16) + (1 << 8) + (uchar(-1));
        } else {
            tmove(b,s).classification = Move::SpecialMove;
            turn(b,s)["CurseGhost"] = true;
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 8) : makeBracket(10, 0) ;
    }

    static void uas(int s, int t, BS &b) {
        if (turn(b,s)["CurseGhost"].toBool() == true) {
            b.inflictPercentDamage(s, 50, s);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Cursed", &et);
            poke(b,t)["Cursed"] = true;
            b.sendMoveMessage(25, 0, s, Pokemon::Curse, t);
        }
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s) || b.hasWorkingAbility(s, Ability::MagicGuard))
            return;
        b.sendMoveMessage(25, 1, s, Pokemon::Curse);
        b.inflictPercentDamage(s, 25, s);
    }
};

struct MMDestinyBond : public MM
{
    MMDestinyBond() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["DestinyBondTurn"] = b.turn();
        addFunction(poke(b,s), "AfterKoedByStraightAttack", "DestinyBond", &akbsa);
        b.sendMoveMessage(26, 1, s, Pokemon::Ghost);
    }

    static void akbsa(int s, int t, BS &b) {
        if (b.koed(t))
            return;

        int trn = poke(b,s)["DestinyBondTurn"].toInt();

        //Destiny Bond does not take out your own pokemon.
        if ((trn == b.turn() || (trn+1 == b.turn() && !fturn(b,s).contains(TM::HasMoved))) && !b.arePartners(t, s)) {
            b.sendMoveMessage(26, 0, s, Pokemon::Ghost, t);
            b.koPoke(t, s, false);

            /* Self KO clause! */
            b.selfKoer() = s;
        }
    }
};

struct MMDetect : public MM
{
    MMDetect() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static bool othersHaveMoved(int s, int, BS &b) {
        /* Detect / Protects fail if all others already moved */
        bool fail = true;
        for (int t = 0;  t < b.numberOfSlots() ; t++) {
            if (!b.hasMoved(t) && !b.koed(t) && s!=t) {
                fail = false;
                break;
            }
        }

        if (fail) {
            fturn(b,s).add(TM::Failed);
            return true;
        }

        return false;
    }

    static void daf(int s, int, BS &b) {
        /* Detect / Protects fail if all others already moved */
        if (othersHaveMoved(s, s, b)) {
            return;
        }

        if (poke(b,s).contains("ProtectiveMoveTurn") && poke(b,s)["ProtectiveMoveTurn"].toInt() == b.turn() - 1) {
            //Wide guard and Quick guard are not affected by ProtectMoveCount in gen 6
            if (!testSuccess(poke(b,s)["ProtectiveMoveCount"].toInt(), b) && !((b.gen() >= 6 && (move(b,s) == Move::QuickGuard || move(b,s) == Move::WideGuard)))) {
                fturn(b,s).add(TM::Failed);
            } else {
                poke(b,s)["ProtectiveMoveTurn"] = b.turn();
                inc(poke(b,s)["ProtectiveMoveCount"]);
            }
        } else {
            poke(b,s)["ProtectiveMoveTurn"] = b.turn();
            poke(b,s)["ProtectiveMoveCount"] = 1;
        }
    }

    static bool testSuccess(int protectCount, BS &b) {
        if (b.gen() <= 2) {
            unsigned x = 256 / (1 << (std::min(protectCount, 8))) - 1;

            return (b.randint() & 0xFF) < x;
        } else if (b.gen() <= 4) {
            int x = 1 << (std::min(protectCount, 3));

            return (b.randint() & (x-1)) == 0;
        } else {
            int x = 1 << (std::min(protectCount, 8));

            if (x >= 256) {
                return b.randint() == 0;
            } else {
                return (b.randint() & (x-1)) == 0;
            }
        }
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "Detect", &dgaf);
        turn(b,s)["DetectUsed"] = true;
        b.sendMoveMessage(27, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        if (!turn(b,t)["DetectUsed"].toBool()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        if (turn(b,s).contains("TellPlayers")) { /* if the move was secret and cancelled, disclose it (like free fall) */
            b.notify(BS::All, BattleCommands::UseAttack, s, qint16(move(b,s)), false);
        }
        b.fail(s, 27, 0, Pokemon::Normal, t);
    }
};

struct MMEruption : public MM
{
    MMEruption() {
        functions["BeforeTargetList"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        tmove(b, s).power = std::max(10, tmove(b, s).power*b.poke(s).lifePoints()/b.poke(s).totalLifePoints());
    }
};

struct MMFacade : public MM
{
    MMFacade() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        int status = b.poke(s).status();
        if (status == Pokemon::Burnt || status == Pokemon::Poisoned || status == Pokemon::Paralysed) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMFakeOut : public MM
{
    MMFakeOut() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).value("HasMovedOnce").toInt() < b.turn()) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMDreamingTarget : public MM
{
    MMDreamingTarget() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (b.poke(t).status() != Pokemon::Asleep || (tmove(b,s).power == 0 && b.hasSubstitute(t))) {
            b.fail(s, 31, 0, type(b,s), t);
        }
    }
};

struct MMFaintUser : public MM
{
    MMFaintUser() {
        functions["AfterTellingPlayers"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (move(b,s) == Move::Explosion || move(b,s) == Move::Selfdestruct) {
            //Damp
            foreach (int i, b.sortedBySpeed()) {
                if (b.hasWorkingAbility(i, Ability::Damp) && !b.koed(i)) {
                    b.sendMoveMessage(114,0,i);
                    tmove(b, s).power = 0;
                    tmove(b,s).targets = Move::User;
                    return;
                }
            }
        }

        //Burn handling for Self-KO moves
        if (b.poke(s).status() == Pokemon::Burnt) {
            turn(b,s)["WasBurned"] = true;
        }

        b.selfKoer() = s;
        b.koPoke(s, s);
    }
};

struct MMFeint : public MM
{
    MMFeint() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        const char *shields[] = {"DetectUsed", "KingsShieldUsed", "SpikyShieldUsed", "CraftyShieldUsed", "MatBlockUsed", "WideGuardUsed", "QuickGuardUsed"};
        bool remove = false;

        for (int i = 0; i < 7; i++) {
            //Single Target: Protect/Detect, KingShield, SpikyShield
            if (i < 3) {
                if (turn(b,t).value(shields[i]).toBool()) {
                    turn(b,t).remove(shields[i]);
                    b.sendMoveMessage(42,i,t);
                    remove = true;
                }
            } else {
                //Team Target: CraftyShield, Mat Block, WideGuard, QuickGuard
                if (team(b,b.player(t)).contains(shields[i])) {
                    team(b,b.player(t)).remove(shields[i]);
                    b.sendMoveMessage(42,i,t);
                    remove = true;
                }

            }
        }

        if (b.gen() <= 4 && !remove) {
            fturn(b, s).add(TM::Failed);
        }
    }
};

struct MMOHKO : public MM
{
    MMOHKO() {
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void cad(int s, int t, BS &b) {
        turn(b,s)["CustomDamage"] = b.poke(t).totalLifePoints();
    }

    static void uas(int s, int t, BS &b) {
        if (b.koed(t)) {
            b.sendMoveMessage(43,1,s,type(b,s));
        }
    }

    static void daf(int s, int t, BS &b) {
        if ( (b.gen() > 1 && b.poke(s).level() < b.poke(t).level()) || (b.gen().num == 1 && b.getStat(s, Speed) < b.getStat(t, Speed)) ) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        if (b.hasWorkingAbility(t, Ability::Sturdy)) {
            b.fail(s,43,0,type(b,s),t);
        }
    }
};

struct MMFlail : public MM
{
    MMFlail() {
        functions["BeforeTargetList"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        int n = 64 * b.poke(s).lifePoints() / b.poke(s).totalLifePoints();
        int mult = 20;
        if (n <= 1) {
            mult = 200;
        } else if (n <= 5) {
            mult = 150;
        } else if (n <= 12) {
            mult = 100;
        } else if (n <= 21) {
            mult = 80;
        } else if (n <= 42) {
            mult = 40;
        }

        tmove(b, s).power = tmove(b, s).power * mult;
    }
};

struct MMFrustration : public MM
{
    MMFrustration() {
        functions["BeforeTargetList"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        tmove(b, s).power = tmove(b, s).power *
                std::max((move(b,s) == Move::Frustration ? (255-b.poke(s).happiness()) : b.poke(s).happiness()) * 2
                         / 5, 2);
    }
};

struct MMBellyDrum : public MM
{
    MMBellyDrum() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        if (b.poke(s).lifePoints() <= std::max(b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100,1)) {
            b.fail(s, 8);

            /* Odd bug with gold, silver, crystal versions in gen 2 */
            if ((b.gen() == Pokemon::gen(Gen::GoldSilver) || b.gen() == Pokemon::gen(Gen::Crystal)) && move(b,s) == Move::BellyDrum) {
                b.inflictStatMod(s, Attack, 2, s);
            }
        } else if (move(b,s) == Move::BellyDrum) {
            if (b.hasMaximalStatMod(s, Attack)) {
                fturn(b,s).add(TM::Failed);
            } else if (b.gen() <= 2) {
                if (b.getStat(s, Attack) == 999) {
                    fturn(b,s).add(TM::Failed);
                }
            }
        }
    }
    static void uas(int s, int, BS &b) {
        if (move(b,s) == Move::BellyDrum) {
            b.sendMoveMessage(8,1,s,type(b,s));
            b.inflictStatMod(s,Attack,12, s, false);

            if (b.gen().num == 2) {
                while (b.getStat(s, Attack) == 999) {
                    b.inflictStatMod(s,Attack,-1,s,false);
                }
                b.inflictStatMod(s,Attack,1,s,false);
            }
        }
        if (move(b,s) != Move::Substitute) {
            b.changeHp(s, b.poke(s).lifePoints() - std::max(b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100,1));
        }
    }
};

struct MMAromaTherapy : public MM
{
    MMAromaTherapy() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        int move = MM::move(b,s);
        int player = b.player(s);
        b.sendMoveMessage(3, (move == Aromatherapy) ? 0 : 1, s, type(b,s));
        for (int i = 0; i < 6; i++) {
            //SoundProof blocks healbell but not aromatherapy
            //In gen 5 heal bell cures Pokemon with SoundProof
            if (!b.poke(player,i).ko() && (move == Aromatherapy || (b.gen().num == 5 && move == HealBell) || b.poke(player,i).ability() != Ability::Soundproof)) {
                b.changeStatus(player,i,Pokemon::Fine);
            }
        }
    }
};

struct MMBlock : public MM
{
    MMBlock() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf (int s, int t, BS &b) {
        if (b.linked(t, "Blocked"))
            fturn(b,s).add(TM::Failed);
    }

    static void uas (int s, int t, BS &b) {
        b.link(s, t, "Blocked");
        b.sendMoveMessage(12, 0, s, type(b,s), t);
    }
};

struct MMFocusPunch : public MM
{
    MMFocusPunch() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnSetup"] = &os;
    }

    static void daf(int s, int, BS &b)
    {
        if (turn(b,s).contains("DamageTakenBy")) {
            b.fail(s,47,0,Pokemon::Fighting);
        }
    }

    static void os(int s, int, BS &b) {
        //Pokemon who are Frozen, Asleep and Paralyzed will display the message showing it "is tightening its focus"
        //DAF and Status effects will handle the second message correctly. Commenting out code just in case it changes for Gen 6
        //if (b.poke(s).status() != Pokemon::Frozen && b.poke(s).status() != Pokemon::Paralysed && b.poke(s).status() != Pokemon::Asleep)
        b.sendMoveMessage(47,1,s,Pokemon::Fighting);
    }
};

struct MMCovet : public MM
{
    MMCovet() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s,int t,BS &b)
    {
        /* Thief & Covet steal item even if target koed, at least in gen 5 */
        int i2 = b.poke(t).item();
        if (b.poke(s).item() == 0 && b.canLoseItem(t, s) && b.canPassMStone(s, i2)) {
            b.sendMoveMessage(23,(move(b,s)==Covet)?0:1,s,type(b,s),t,b.poke(t).item());
            b.acqItem(s, b.poke(t).item());
            b.loseItem(t);
        } else if (b.hasWorkingAbility(t, Ability::StickyHold)) {
            b.sendAbMessage(121,1,t);
        }
    }
};

struct MMDragonRage : public MM
{
    MMDragonRage() {
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] =  turn(b,s)["DragonRage_Arg"];
    }
};


struct MMCopycat : public MM
{
    MMCopycat() {
        functions["EvenWhenCantMove"] = &ewcm;
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ewcm(int s, int, BS &b) {
        QString key = b.gen() <= 4 ? "LastMoveUsed" : "AnyLastMoveUsed";
        turn(b,s)["CopycatMove"] = b.battleMemory().value(key).toInt();
    }

    static void daf(int s, int, BS &b) {
        /* First check if there's even 1 move available */
        int move = turn(b,s)["CopycatMove"].toInt();
        if (move == 0 || move == Copycat || move == Move::DragonTail || move == Move::CircleThrow || move == Move::Struggle || (b.gen() > 5 && (move == Roar || move == Whirlwind))
                || move == Belch) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "Copycat");
        removeFunction(turn(b,s), "DetermineAttackFailure", "Copycat");
        int attack = turn(b,s)["CopycatMove"].toInt();
        BS::BasicMoveInfo info = tmove(b,s);
        MoveEffect::setup(attack, s, t, b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s, attack, true);
        MoveEffect::unsetup(attack, s, b);
        tmove(b,s) = info;
    }
};

struct MMAssist : public MM
{
    MMAssist() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    struct FM : public QSet<int>
    {
        FM() {
            (*this) << NoMove << Assist << Chatter << Copycat << Counter << Covet << CraftyShield << DestinyBond << Detect
                              << Endure << Feint << FocusPunch << FollowMe << HelpingHand << KingsShield << MatBlock << MeFirst
                              << Metronome << Mimic << MirrorCoat << MirrorMove << Protect << Belch << SpikyShield
                              << Sketch << SleepTalk << Snatch << Struggle << Switcheroo << Thief << Trick << Transform;
        }

        bool contains(int move, Pokemon::gen gen) const {
            //Assist's Forbidden Moves will be inherited later for Metronome so we break out the moves that are only blocked with Assist
            //Metronome will inherit a block on Assist in Gen III despite it being a valid move
            if (move == Transform || move == NaturePower || move == CircleThrow || move == DragonTail) {
                return gen >= 5;
            } else if (move == PhantomForce || move == Geomancy || move == Belch || move == StickyWeb || move == Dive || move == Dig ||
                       move == Fly || move == Bounce || move == FreezeShock || move == IceBurn || move == RazorWind || move == SkullBash ||
                       move == SkyDrop || move == SolarBeam || move == SkyAttack || move == ShadowForce || move == Roar || move == Whirlwind){
                return gen >= 6;
            } else {
                return QSet<int>::contains(move);
            }
        }
    };
    static FM forbidden_moves;

    static void uas(int s, int, BS &b)
    {
        removeFunction(turn(b,s), "UponAttackSuccessful", "Assist");
        removeFunction(turn(b,s), "DetermineAttackFailure", "Assist");
        int attack = turn(b,s)["AssistMove"].toInt();
        BS::BasicMoveInfo info = tmove(b,s);
        MoveEffect::setup(attack, s, s, b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s, turn(b,s)["AssistMove"].toInt(), true);
        MoveEffect::unsetup(attack, s, b);
        tmove(b,s) = info;
    }

    static void daf(int s, int, BS &b)
    {
        int player = b.player(s);
        QList<int> possible_moves;
        for (int i = 0; i < 6; i++) {
            if (!b.isOut(player, i) && b.poke(player, i).num() != 0) {
                PokeBattle &p = b.poke(player,i);
                for(int j = 0; j < 4; j++) {
                    int m = p.move(j);
                    if (!forbidden_moves.contains(m, b.gen()))
                        possible_moves.push_back(m);
                }
            }
        }
        if (!possible_moves.empty()) {
            turn(b,s)["AssistMove"] = *(possible_moves.begin() + (b.randint(possible_moves.size())));
        } else {
            fturn(b,s).add(TM::Failed);
        }
    }
};

//Declaring static class variable
MMAssist::FM MMAssist::forbidden_moves;

struct MMBide : public MM
{
    MMBide() {
        functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static void uas(int s, int , BS &b) {
        addFunction(poke(b,s), "TurnSettings", "Bide", &ts);
        addFunction(turn(b,s), "UponOffensiveDamageReceived", "Bide", &udi);
        poke(b,s)["BideDamageCount"] = 0;
        poke(b,s)["BideTurn"] = b.turn();
    }

    static void udi(int s, int, BS &b) {
        inc(poke(b,s)["BideDamageCount"],poke(b,s)["DamageTakenByAttack"].toInt());
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s)["BideDamageCount"].toInt() == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void btl(int s, int, BS &b) {
        b.sendMoveMessage(9,1,s,type(b,s));
    }

    static void uas2(int s, int, BS &b) {
        b.sendMoveMessage(9,0,s,type(b,s));
    }
    static void ms(int s, int, BS &b) {
        tmove(b, s).power = 0;
        if (!poke(b,s).contains("BideTurn"))
            return;
        int _turn = poke(b,s)["BideTurn"].toInt();
        if (_turn + 2 < b.turn()) {
            return;
        }

        turn(b,s)["TellPlayers"] = false;
        if (_turn +1 == b.turn()) {
            tmove(b, s).targets = Move::User;
            tmove(b, s).power = 0;
            addFunction(turn(b,s), "UponAttackSuccessful", "Bide", &uas2);
        } else {
            tmove(b, s).targets = Move::ChosenTarget;
            tmove(b, s).power = 1;
            tmove(b, s).type = Pokemon::Curse;
            addFunction(turn(b,s), "BeforeTargetList", "Bide", &btl);
            addFunction(turn(b,s), "CustomAttackingDamage", "Bide", &ccd);
            addFunction(turn(b,s), "DetermineAttackFailure", "Bide",&daf);
            removeFunction(poke(b,s), "TurnSettings", "Bide");
            removeFunction(turn(b,s), "UponAttackSuccessful", "Bide");
        }
    }

    static void ts(int s, int, BS &b) {
        int _turn = poke(b,s)["BideTurn"].toInt();
        if (_turn + 2 < b.turn()) {
            return;
        }

        addFunction(turn(b,s),"UponOffensiveDamageReceived", "Bide", &udi);
        MoveEffect::setup(Move::Bide, s, s, b);
        fturn(b, s).add(TM::NoChoice);
    }

    static void ccd(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = 2*poke(b,s)["BideDamageCount"].toInt();
    }
};

struct MMBind : public MM
{
    MMBind() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(3,0) : gen <= 4 ? makeBracket(6, 9) : makeBracket(11, 0) ;
    }

    static void uas (int s, int t, BS &b) {
        if (!b.linked(t, "Trapped")) {
            b.link(s, t, "Trapped");
            BS::BasicMoveInfo &fm = tmove(b,s);
            poke(b,t)["TrappedRemainingTurns"] = b.poke(s).item() == Item::GripClaw ?
                        fm.maxTurns : (b.randint(fm.maxTurns+1-fm.minTurns)) + fm.minTurns; /* Grip claw = max turns */
            poke(b,t)["TrappedMove"] = move(b,s);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Bind", &et);
        }
    }

    static void ms (int s, int, BS &b) {
        turn(b,s)["TellPlayers"] = false;
        tmove(b,s).reset(); // Cancel the move
    }

    static void et (int s, int, BS &b) {
        int count = poke(b,s)["TrappedRemainingTurns"].toInt() - 1;
        int move = poke(b,s)["TrappedMove"].toInt();

        if (!b.koed(s)) {
            if (!b.linked(s, "Trapped")) {
                poke(b,s).remove("TrappedBy");
                b.removeEndTurnEffect(BS::PokeEffect, s, "Bind");
                removeFunction(poke(b,s), "TurnSettings", "Bind");
                return;
            }
            if (count <= 0) {
                poke(b,s).remove("TrappedBy");
                b.removeEndTurnEffect(BS::PokeEffect, s, "Bind");
                removeFunction(poke(b,s), "TurnSettings", "Bind");
                if (count == 0)
                    b.sendMoveMessage(10,1,s,MoveInfo::Type(move, b.gen()),s,move);
            } else {
                poke(b,s)["TrappedRemainingTurns"] = count;

                if (!b.hasWorkingAbility(s, Ability::MagicGuard)) {
                    b.sendMoveMessage(10,0,s,MoveInfo::Type(move, b.gen()),s,move);

                    int trapper = b.linker(s, "Trapped");
                    if (b.gen() < 6) {
                        b.inflictDamage(s, b.poke(s).totalLifePoints()/(b.hasWorkingItem(trapper, Item::BindingBand) ? 8 : 16),s,false);
                    } else {
                        b.inflictDamage(s, b.poke(s).totalLifePoints()/(b.hasWorkingItem(trapper, Item::BindingBand) ? 6 : 8),s,false);
                    }
                }
            }
        }
    }
};

struct MMStomp : public MM
{
    MMStomp(){
        functions["BeforeTargetList"] = &btl;
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void btl(int s, int, BS &b) {
        if (b.targetList.size() > 0) {
            if (poke(b, b.targetList.front()).value("Minimize").toBool() && b.gen() > 5) {
                tmove(b, s).accuracy = 0;
            }
        }
    }

    static void bcd(int s, int t, BS &b) {
        if (poke(b,t).value("Minimize").toBool()) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMBounce : public MM
{
    MMBounce() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static void daf(int s, int t, BS &b) {
        if (!poke(b,s).value("Invulnerable").toBool()) {
            /* First part of the move */
            if (b.hasSubstitute(t) && move(b,s) == Move::SkyDrop) {
                b.notify(BS::All, BattleCommands::UseAttack, s, qint16(Move::SkyDrop));
                fturn(b,s).add(TM::Failed);
            }
        } else {
            /* Second part of the move */
            if (move(b,s) == Move::SkyDrop) {
                if (!b.linked(s, "FreeFalledPokemon")) {
                    fturn(b,s).add(TM::Failed);
                }
            }
        }
    }

    static void ms(int s, int, BS &b) {
        if (b.hasWorkingItem(s, Item::PowerHerb)) {
            QStringList args = turn(b,s)["Bounce_Arg"].toString().split('_');
            b.sendMoveMessage(13,args[0].toInt(),s,type(b,s),s,move(b,s));
            b.sendItemMessage(11,s);
            b.disposeItem(s);

            removeFunction(turn(b,s), "UponAttackSuccessful", "Bounce");
            if (move(b,s) == ShadowForce || move(b,s) == PhantomForce) {
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &MMFeint::daf);
                if (poke(b, b.targetList.front()).value("Minimize").toBool()) {
                    tmove(b, s).accuracy = 0;
                    tmove(b, s).power = tmove(b, s).power * 2;
                }
            }
        } else {
            tmove(b, s).power = 0;
            tmove(b, s).accuracy = 0;
            if (move(b,s) != Move::SkyDrop) {
                tmove(b, s).targets = Move::User;
                tmove(b, s).status = 0;
                tmove(b, s).statAffected = 0;
            }

            poke(b,s)["2TurnMove"] = move(b,s);
            turn(b,s)["TellPlayers"] = false;
        }
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s).value("Invulnerable").toBool()) {
            fturn(b, s).add(TM::NoChoice);

            int move = poke(b,s)["2TurnMove"].toInt();

            initMove(move, b.gen(),tmove(b,s));
            addFunction(turn(b,s), "EvenWhenCantMove", "Bounce", &ewc);

            if (move == ShadowForce || move == PhantomForce) {
                addFunction(turn(b,s), "BeforeTargetList", "Bounce", &MMStomp::btl);
                addFunction(turn(b,s), "BeforeCalculatingDamage", "Bounce", &MMStomp::bcd);
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &MMFeint::daf);
            } else if (move == SkyDrop) {
                /* FreeFall sure-hits the foe once it caught it... */
                tmove(b,s).accuracy = 0;
                addFunction(turn(b,s), "BeforeCalculatingDamage", "Bounce", &bcd);
            }
        }
        //In ADV, the turn can end if for exemple the foe explodes, in which case TurnSettings will be needed next turn too
        //removeFunction(poke(b,s), "TurnSettings", "Bounce");
    }

    /* Called with freefall */
    static void bcd (int s, int t, BS &b) {
        /* Airbourne targets don't receive damage */
        if (b.hasType(t, Type::Flying)) {
            tmove(b,s).power = 1;
        }
    }

    static void ewc(int s, int, BS &b) {
        poke(b,s)["Invulnerable"] = false;
        /* If the foe caught us while we were digging, we don't come back on earth
           when we fail to move. */
        if (b.linked(s, "FreeFalled")) {
            return;
        }
        b.changeSprite(s, 0);

        if (b.linked(s, "FreeFalledPokemon")) {
            int t = b.linker(s, "FreeFalledPokemon");;
            b.changeSprite(t, 0);
            poke(b,t).remove("FreeFalledBy");
            poke(b,s).remove("FreeFalledPokemonBy");
        }
    }

    static void groundStruck(int s, BS &b) {
        poke(b,s)["Invulnerable"] = false;
        b.changeSprite(s, 0);

        if (b.linked(s, "FreeFalledPokemon")) {
            int t = b.linker(s, "FreeFalledPokemon");;
            b.changeSprite(t, 0);
            poke(b,t).remove("FreeFalledBy");
            poke(b,s).remove("FreeFalledPokemonBy");
        }
    }

    static void uas(int s, int t, BS &b) {
        QStringList args = turn(b,s)["Bounce_Arg"].toString().split('_');

        b.sendMoveMessage(13,args[0].toInt(),s,type(b,s), t, move(b,s));

        QList<int> vuln_moves, vuln_mult;
        for (int i = 1; i < args.size(); i++)
        {
            QStringList b = args[i].split('*');
            vuln_moves.push_back(b.front().toInt());
            vuln_mult.push_back(b.size() == 1 ? 1 : b.back().toInt());
        }
        poke(b,s)["Invulnerable"] = true;
        poke(b,s)["VulnerableMoves"].setValue(vuln_moves);
        poke(b,s)["VulnerableMults"].setValue(vuln_mult);
        b.changeSprite(s, -1);
        addFunction(poke(b,s), "TestEvasion", "Bounce", &dgaf);
        addFunction(poke(b,s), "TurnSettings", "Bounce", &ts);

        int att = move(b,s);
        /* Those moves protect from weather when in the invulnerable state */
        if (att == Move::Dig || att == Move::Dive)
            turn(b,s)["WeatherSpecialed"] = true;
        if (att == Move::SkyDrop) {
            b.link(s, t, "FreeFalled");
            b.link(t, s, "FreeFalledPokemon");
            b.changeSprite(t, -1);
            addFunction(poke(b,t), "TestEvasion", "Bounce", &dgaf);
            addFunction(poke(b,t), "DetermineAttackPossible", "Bounce", &dap);
            addFunction(poke(b,s), "AfterBeingKoed", "Bounce", &ewc);
            poke(b,t)["VulnerableMoves"].setValue(vuln_moves);
            poke(b,t)["VulnerableMults"].setValue(vuln_mult);
        }
    }

    static void dgaf(int s, int t, BS &b) {
        if (b.linked(s, "FreeFalled")) {
            turn(b,s)["EvadeAttack"] = true;
            return;
        }

        if (s == t || t == -1) {
            return;
        }
        if (!poke(b,s).value("Invulnerable").toBool()) {
            return;
        }
        int attack = move(b,t);
        /* Lets see if the poke is vulnerable to that one attack in Gen 2 or later */
        QList<int> vuln_moves = poke(b,s)["VulnerableMoves"].value<QList<int> >();

        for (int i = 0; i < vuln_moves.size(); i++) {
            if (vuln_moves[i] == attack && b.gen() != 1) {
                return;
            }
        }

        /* All other moves fail */
        turn(b,s)["EvadeAttack"] = true;
    }

    static void dap(int s, int, BS &b) {
        if (b.linked(s, "FreeFalled")) {
            b.sendMoveMessage(13, 6, s);
            turn(b,s) ["ImpossibleToMove"] = true;
            return;
        }
    }
};

struct MMCounter : public MM
{
    MMCounter() {
        functions["MoveSettings"] = &ms;
        functions["UponOffensiveDamageReceived"] = &uodr;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void uodr(int s, int source, BS &b) {
        if (b.gen() >= 4 && tmove(b,source).category != turn(b,s)["Counter_Arg"].toInt()) {
            return;
        }
        /* In third gen, all hidden power are countered by counter but not by mirror coat */
        if (b.gen() <= 3 && TypeInfo::Category(MoveInfo::Type(move(b, source), b.gen())) != turn(b,s)["Counter_Arg"].toInt()) {
            return;
        }
        /* In gen 1, only Normal and Fighting moves are countered */
        if (b.gen().num == 1 && type(b,source) != Type::Fighting && type(b,source) != Type::Normal) {
            return;
        }

        if (fturn(b, s).damageTaken <= 0) {
            return;
        }

        turn(b,s)["CounterDamage"] = 2 * fturn(b,s).damageTaken;
        turn(b,s)["CounterTarget"] = source;
    }

    static void ms (int s, int, BS &b) {
        //In GSC Sleep Talk + Counter works
        if (!turn(b,s).contains("CounterDamage") && b.gen().num == 2) {
            int t = b.slot(b.opponent(b.player(s)));

            if (b.hasMoved(t) && TypeInfo::Category(MoveInfo::Type(move(b, t), 2)) == turn(b,s)["Counter_Arg"].toInt()
                    && fturn(b, s).damageTaken > 0) {
                turn(b,s)["CounterDamage"] = 2 * fturn(b,s).damageTaken;
                turn(b,s)["CounterTarget"] = t;
            }
        }
        turn(b,s)["Target"] = turn(b,s)["CounterTarget"];
        tmove(b,s).targets = Move::ChosenTarget;
    }

    static void daf (int s, int t, BS &b) {
        if (!turn(b,s).contains("CounterDamage")) {
             if ((b.gen() == Pokemon::gen(Gen::GoldSilver) || b.gen() == Pokemon::gen(Gen::Crystal))
                     && (fpoke(b,t).lastMoveUsed == Move::Fissure || fpoke(b,t).lastMoveUsed == Move::HornDrill)) {
                 turn(b,s)["CounterDamage"] = b.poke(t).totalLifePoints();
                 return;
             }
            fturn(b,s).add(TM::Failed);
        }
    }

    static void cad(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = turn(b,s)["CounterDamage"].toInt();
    }
};

struct MMDoomDesire : public MM
{
    MMDoomDesire() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void ms(int s, int, BS &b) {
        tmove(b, s).type = Pokemon::Curse;
        tmove(b, s).power = 1;
        tmove(b, s).accuracy = 0;
    }

    static void daf(int s, int t, BS &b) {
        if (slot(b,t).contains("DoomDesireTurn") && slot(b,t)["DoomDesireTurn"].toInt() >= b.turn()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(1, 0) : gen <= 4 ? makeBracket(7, 0) : makeBracket(3, 0) ;
    }

    static void cad(int s, int t, BS &b) {
        int move = MM::move(b,s);

        slot(b,t)["DoomDesireTurn"] = b.turn() + 2;
        slot(b,t)["DoomDesireMove"] = move;
        tmove(b, s).accuracy = MoveInfo::Acc(move, b.gen());
        slot(b,t)["DoomDesireFailed"] = !b.testAccuracy(s,t,true);
        if (b.gen() <= 4) {
            fturn(b,s).remove(TM::CriticalHit);
            tmove(b, s).power = tmove(b, s).power * MoveInfo::Power(move, b.gen());
            slot(b,t)["DoomDesireDamage"] = b.calculateDamage(s, t);
        } else {
            slot(b,t)["DoomDesireAttack"] = b.getStat(s, SpAttack);
            tmove(b, s).type = MoveInfo::Type(tmove(b,s).attack, b.gen());
            b.calculateTypeModStab();
            tmove(b, s).type = Pokemon::Curse;
            slot(b,t)["DoomDesireStab"] = fturn(b,s).stab;
            slot(b,t)["DoomDesireId"] = b.team(b.player(s)).internalId(b.poke(s));
        }
        b.addEndTurnEffect(BS::SlotEffect, bracket(b.gen()), t, "DoomDesire", &et);
        b.sendMoveMessage(29, move==DoomDesire?2:1, s, type(b,s));
    }

    static void et (int s, int, BS &b) {
        if (b.turn() == slot(b,s).value("DoomDesireTurn"))
        {
            b.removeEndTurnEffect(BS::SlotEffect, s, "DoomDesire");

            //Invulnerable Check needed to interact with DB files for proper move execution.
            if(poke(b,s).value("Invulnerable").toBool()) {
                slot(b,s)["DoomDesireFailed"] = true;
            }
            if (slot(b,s)["DoomDesireFailed"].toBool()) {
                int move = slot(b,s)["DoomDesireMove"].toInt();
                b.sendMoveMessage(29,0,s,MoveInfo::Type(move, b.gen()),s,move);
                b.notifyFail(s);
            } else if (!b.koed(s)) {
                int move = slot(b,s)["DoomDesireMove"].toInt();
                b.sendMoveMessage(29,0,s,MoveInfo::Type(move, b.gen()),s,move);

                if (b.gen() <= 4) {
                    b.inflictDamage(s,slot(b,s)["DoomDesireDamage"].toInt(), s, true, true);
                } else {
                    initMove(move, b.gen(), tmove(b,s));

                    b.calculateTypeModStab(s, s);

                    int typemod = fturn(b,s).typeMod;
                    if (typemod < -50) {
                        /* If it's ineffective we just say it */
                        b.notify(BS::All, BattleCommands::Effective, s, quint8(0));
                        return;
                    }
                    if ((b.rawTypeEff(MoveInfo::Type(slot(b,s)["DoomDesireMove"].toInt(), b.gen()), s) <= 0) && b.hasWorkingAbility(s, Ability::WonderGuard) && b.gen() >= 5) {
                        b.sendAbMessage(71,0,s);
                        return;
                    }
                    fturn(b,s).stab = slot(b,s)["DoomDesireStab"].toInt();
                    turn(b,s)["AttackStat"] = slot(b,s)["DoomDesireAttack"];
                    fturn(b,s).remove(TM::CriticalHit);
                    tmove(b,s).power = MoveInfo::Power(move, b.gen());

                    int t = b.opponent(b.player(s));
                    int doomuser = s;

                    for (int i = 0; i < b.numberPerSide(); i++) {
                        if (b.team(t).internalId(b.poke(t, i)) == slot(b,s).value("DoomDesireId").toInt()) {
                            doomuser = b.slot(t, i);
                            break;
                        }
                    }
                    tmove(b, doomuser).recoil = 0;

                    int damage = b.calculateDamage(s, s);
                    b.notify(BS::All, BattleCommands::Effective, s, quint8(typemod > 0 ? 8 : (typemod < 0 ? 2 : 4)));
                    b.inflictDamage(s, damage, doomuser, true, true);
                }
            }
        }
    }
};

struct MMEmbargo : public MM
{
    MMEmbargo() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.ability(t) == Ability::Multitype)
            fturn(b,s).add(TM::Failed);
        else if (poke(b,t).contains("EmbargoEnd") && poke(b,t)["EmbargoEnd"].toInt() >= b.turn()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 17) : makeBracket(18, 0) ;
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(32,0,s,type(b,s),t);
        poke(b,t)["Embargoed"] = true;
        poke(b,t)["EmbargoEnd"] = b.turn() + 4;
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Embargo", &et);
    }

    static void et(int s, int , BS &b) {
        if (poke(b,s).value("Embargoed").toBool() && poke(b,s)["EmbargoEnd"].toInt() <= b.turn()) {
            b.sendMoveMessage(32,1,s,0);
            poke(b,s)["Embargoed"] = false;
            b.removeEndTurnEffect(BS::PokeEffect, s, "Embargo");
            b.callieffects(s, s, "UponReactivation");
        }
    }
};

struct MMEncore : public MM
{
    MMEncore() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    struct FM : public QSet<int>
    {
        FM() {
            /* Encore , Mimic Mirror Move, Sketch,  Struggle Transform, ,  */
            (*this)  << Encore << Mimic << MirrorMove << Sketch << Struggle << Transform;
        }
    };
    static FM forbidden_moves;

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(9, 0) : gen <= 4 ? makeBracket(6, 13) : makeBracket(13, 0) ;
    }

    static void daf(int s, int t, BS &b)
    {
        if (b.counters(t).hasCounter(BC::Encore))
        {
            fturn(b,s).add(TM::Failed);
            return;
        }
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        if (b.gen() > 2) {
            int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
            if (tu + 1 < b.turn() || (tu + 1 == b.turn() && fturn(b,t).contains(TM::HasMoved))) {
                fturn(b,s).add(TM::Failed);
                return;
            }
            /* Encore & 2-turn moves fail before gen 6 */
            if (fturn(b,t).contains(TM::NoChoice) && b.gen() < 6) {
                fturn(b,s).add(TM::Failed);
                return;
            }
        }
        int move = poke(b,t)["LastMoveUsed"].toInt();
        bool cont = forbidden_moves.contains(move);

        if (cont) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int sl = -1;
        for (int i = 0; i < 4; i++) {
            if (b.move(t, i) == move) {
                sl = i;
            }
        }
        if (sl == -1 || b.PP(t,sl) == 0 ) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void uas (int s, int t, BS &b) {
        if (b.hasWorkingTeamAbility(t, Ability::AromaVeil)) {
            b.sendAbMessage(112,1,t);
            return;
        }
        b.sendMoveMessage(33,1,s,0,t);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            if (b.gen() <=3)
                b.counters(t).addCounter(BC::Encore, 2 + (b.randint(4)));
            else if (b.gen().num == 4)
                b.counters(t).addCounter(BC::Encore, 3 + (b.randint(5)));
            else
                b.counters(t).addCounter(BC::Encore, 2);

            int mv =  poke(b,t)["LastMoveUsed"].toInt();
            poke(b,t)["EncoresMove"] = mv;

            /*Changes the encored move, if no choice is off (otherwise recharging moves like blast burn would attack again,
                and i bet something strange would also happen with charging move) */
            if (!fturn(b,t).contains(TM::NoChoice) && b.choice(t).attackingChoice()) {
                for (int i = 0; i < 4; i ++) {
                    if (b.move(t, i) == mv) {
                        MoveEffect::unsetup(move(b,t), t, b);
                        b.choice(t).setAttackSlot(i);
                        b.choice(t).setTarget(b.randomValidOpponent(t));
                        MoveEffect::setup(mv, t, s, b);
                        break;
                    }
                }
            }
            addFunction(poke(b,t), "MovesPossible", "Encore", &msp);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Encore", &et);
        }
    }

    static void et (int s, int, BS &b)
    {
        if (b.koed(s))
            return;
        for (int i = 0; i <= 4; i++) {
            if (i == 4) {
                b.counters(s).removeCounter(BC::Encore);
            } else {
                if (b.move(s, i) == poke(b,s)["EncoresMove"].toInt()) {
                    if (b.PP(s, i) <= 0) {
                        b.sendMoveMessage(33,0,s);
                        b.counters(s).removeCounter(BC::Encore);
                    }
                    break;
                }
            }
        }
        if (b.counters(s).count(BC::Encore) < 0) {
            removeFunction(poke(b,s), "MovesPossible", "Encore");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Encore");

            if (b.counters(s).hasCounter(BC::Encore)) {
                b.sendMoveMessage(33,0,s);
                b.counters(s).removeCounter(BC::Encore);
            }
        }
    }

    static void msp(int s, int, BS &b) {
        for (int i = 0; i < 4; i++) {
            if (b.move(s,i) != poke(b,s)["EncoresMove"].toInt()) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }
};

//Declaring static class variable
MMEncore::FM MMEncore::forbidden_moves;

struct MMEndeavor : public MM
{
    MMEndeavor() {
        functions["CustomAttackingDamage"] = &cad;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (b.poke(s).lifePoints() >= b.poke(t).lifePoints()) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void cad(int s, int t, BS &b) {
        turn(b,s)["CustomDamage"] = b.poke(t).lifePoints()-b.poke(s).lifePoints();
    }
};

struct MMEndure : public MM
{
    MMEndure() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        turn(b,s)["CannotBeKoed"] = true;
        addFunction(turn(b,s), "UponSelfSurvival", "Endure", &uodr);
        b.sendMoveMessage(35,1,s);
    }

    static void uodr(int s, int, BS &b) {
        turn(b,s)["SurviveReason"] = true;
        b.sendMoveMessage(35,0,s);
    }
};

struct MMFalseSwipe : public MM
{
    MMFalseSwipe() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        turn(b,t)["CannotBeKoedBy"] = s;
        addFunction(turn(b,t), "UponSelfSurvival", "FalseSwipe", &uss);
    }

    static void uss(int s, int, BS &b) {
        turn(b,s)["SurviveReason"] = true;
    }
};

struct MMFocusEnergy : public MM
{
    MMFocusEnergy() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "TurnSettings", "FocusEnergy", &ts);
        b.sendMoveMessage(46,0,s);
    }
    static void ts(int s, int, BS &b) {
        addFunction(turn(b,s), "BeforeTargetList", "FocusEnergy", &btl);
    }
    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power > 0) {
            tmove(b,s).critRaise += 2;
        }
    }
};

struct MMFuryCutter : public MM
{
    MMFuryCutter() {
        functions["OnSetup"] = &os;
        functions["AttackSomehowFailed"] = &ma;
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void os(int s, int, BS &b) {
        if (b.gen() >= 5 && poke(b,s)["AnyLastMoveUsed"].toInt() != FuryCutter) {
            poke(b,s)["FuryCutterCount"] = 0;
        }
    }

    static void ma(int s, int, BS &b) {
        poke(b,s)["FuryCutterCount"] = 0;
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["FuryCutterCount"] = std::min(poke(b,s)["FuryCutterCount"].toInt() * 2 + 1,b.gen().num == 4 ? 15 : 7);
    }

    static void bcd(int s, int, BS &b) {
        tmove(b, s).power = tmove(b, s).power * (poke(b,s)["FuryCutterCount"].toInt()+1);
    }
};

struct MMBeatUp : public MM {
    MMBeatUp() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
        functions["BeforeHitting"] = &bh;
    }

    static void ms(int s, int, BS &b) {
        if (b.gen() <= 4) {
            tmove(b,s).type = Pokemon::Curse;
            tmove(b,s).repeatMin = 0;
            tmove(b,s).repeatMax = 0;
        } else {
            tmove(b,s).power = 10;
        }
    }

    static void daf(int s,int, BS&b) {
        int source = b.player(s);
        for (int i = 0; i < 6; i++) {
            if (b.poke(source, i).status() == Pokemon::Fine) {
                return;
            }
        }
        fturn(b,s).add(TM::Failed);
    }

    static void cad(int s, int t, BS &b) {
        int source = b.player(s);
        int def = PokemonInfo::Stat(b.poke(t).num(), b.gen(), Defense,b.poke(t).level(),0,0);
        for (int i = 0; i < 6; i++) {
            PokeBattle &p = b.poke(source,i);
            if (p.status() == Pokemon::Fine || i == b.slotNum(s)) {
                int att = PokemonInfo::Stat(p.num(), b.gen(), Attack,p.level(),0,0);
                int damage = (((((p.level() * 2 / 5) + 2) * 10 * att / 50) / def) + 2) * (b.randint(255-217) + 217)*100/255/100;
                b.sendMoveMessage(7,0,s,Pokemon::Dark,t,0,p.nick());
                if (b.hasSubstitute(t))
                    b.inflictSubDamage(t,damage,t);
                else
                    b.inflictDamage(t,damage,t,true);
                if (!fpoke(b, t).substitute()) {
                    fpoke(b, t).remove(BS::BasicPokeInfo::HadSubstitute);
                }
            }
            if (b.koed(t))
                return;
        }
    }

    static void bh(int s, int, BS &b) {
        if (b.poke(b.player(s), b.repeatCount()).status() != Pokemon::Fine) {
            turn(b,s)["HitCancelled"] = true;
        } else {
            tmove(b,s).power = 5 + (PokemonInfo::BaseStats(fpoke(b,s).id, b.gen()).baseAttack()/10);
            //turn(b,s)["UnboostedAttackStat"] = b.poke(s, b.repeatCount()).normalStat(Attack);
        }
    }
};

struct MMBlizzard : public MM
{
    MMBlizzard() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (b.gen() >= 4 && b.isWeatherWorking(BattleSituation::Hail)) {
            tmove(b, s).accuracy = 0;
        }
    }
};

struct MMBrickBreak : public MM
{
    MMBrickBreak() {
        functions["BeforeCalculatingDamage"] = &bh;
        functions["BeforeHitting"] = &uas;
    }

    static void bh(int s, int t, BS &b) {
        if (b.gen() <= 4) {
            int opp = b.player(t);
            if (team(b,opp).value("Barrier1Count").toInt() > 0 || team(b,opp).value("Barrier2Count").toInt() > 0) {
                b.sendMoveMessage(14,0,s,Pokemon::Fighting);
                team(b,opp)["Barrier1Count"] = 0;
                team(b,opp)["Barrier2Count"] = 0;
            }
        }
    }

    static void uas(int s, int t, BS &b) {
        if (b.gen() >= 5) {
            int opp = b.player(t);
            if (team(b,opp).value("Barrier1Count").toInt() > 0 || team(b,opp).value("Barrier2Count").toInt() > 0) {
                b.sendMoveMessage(14,0,s,Pokemon::Fighting);
                team(b,opp)["Barrier1Count"] = 0;
                team(b,opp)["Barrier2Count"] = 0;
            }
        }
    }
};

struct MMFling : public MM
{
    MMFling() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
        functions["BeforeTargetList"] = &btl;
        functions["AttackSomehowFailed"] = &asf;
    }

    static void daf(int s, int, BS &b) {
        if (!turn(b,s).contains("FlingItem")) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void btl(int s, int, BS &b) {
        if (b.canLoseItem(s,s) && ItemInfo::Power(b.poke(s).item()) > 0) {
            if (b.gen() >= 5 && b.hasWorkingAbility(s, Ability::Klutz)) {
                return;
            } else if (poke(b,s).value("Embargoed").toBool()) {
                return;
            } else if (b.battleMemory().value("MagicRoomCount").toInt() > 0) {
                return;
            } else if (ItemInfo::isGem(b.poke(s).item())) {
                return;
            }
            tmove(b, s).power = tmove(b, s).power * ItemInfo::Power(b.poke(s).item());
            int t = b.targetList.front();
            b.sendMoveMessage(45, 0, s, type(b,s), t, b.poke(s).item());
            turn(b,s)["FlingItem"] = b.poke(s).item();//needed for failure check
        }
    }

    static void uas (int s, int t, BS &b) {
        int item = b.poke(s).item();
        if (b.hasSubstitute(t) && !b.canBypassSub(t)) {
            b.disposeItem(s);
            return;
        }
        if (!b.koed(t)) {
            bool isEmbargoed = poke(b, t).value("Embargoed").toBool();
            if (!ItemInfo::isBerry(item)) {
                if (item == Item::WhiteHerb || item == Item::MentalHerb) {
                    //Gen 4 doesn't allow Embargoed Targets to use the flung item
                    if (b.gen() > 4 || !isEmbargoed) {
                        int oppitem = b.poke(t).item();
                        ItemEffect::activate("UponSetup", item, t,s,b);
                        b.poke(t).item() = oppitem; /* the effect of mental herb / white herb may have disposed of the foes item */
                    }
                } else if (item == Item::RazorFang || item == Item::KingsRock) {
                    fturn(b,t).add(TM::Flinched); /* king rock, razor fang */
                } else if (!team(b, b.player(t)).contains("SafeGuardCount"))  {
                    switch (item) {
                    case Item::FlameOrb: b.inflictStatus(t, Pokemon::Burnt, s); break; /*flame orb*/
                    case Item::ToxicOrb: b.inflictStatus(t, Pokemon::Poisoned, s, 16, 16); break; /*toxic orb*/
                    case Item::LightBall: b.inflictStatus(t, Pokemon::Paralysed, s); break; /* light ball */
                    case Item::PoisonBarb: b.inflictStatus(t, Pokemon::Poisoned, s); break; /* poison barb */
                    }
                }
            } else {
                if (!b.opponentsHaveWorkingAbility(t, Ability::Unnerve)) {
                    //Gen 4 doesn't allow Embargoed Targets to use the flung item
                    if (b.gen() > 4 || !isEmbargoed) {
                        b.sendMoveMessage(16,0,t,type(b,s),s,item);
                        b.devourBerry(t, item, t);
                    }
                }
            }
        }
        /* Can't be in btl, because LO needs to boost Fling's power */
        /* Dispose item is used to allow Recycle to properly recover the item */
        b.disposeItem(s);
    }

    static void asf(int s, int, BS &b) {
        //If Fling misses due to acc, semi-invuln moves, protect moves, etc. the item is still discarded
        //The item's identity should still be revealed (2nd to last line of btl handles this)
        if (turn(b,s).contains("FlingItem")) {
            b.disposeItem(s);
        }
    }
};

struct MMFollowMe : public MM
{
    MMFollowMe() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(48,0,s);

        int source = b.player(s);

        team(b, source)["FollowMeTurn"] = b.turn();
        team(b, source)["FollowMePlayer"] = s;
        /* Imagine one foe used assist + roar, then follow me wouldn't work anymore. That's
            why we need a switch count, or that */
        poke(b,s)["FollowMe"] = true;

        addFunction(b.battleMemory(), "GeneralTargetChange", "FollowMe", &gtc);
    }

    struct FM : public QSet<int> {
        FM() {
            //Follow Me prevents attraction by all 2-Turn moves
            (*this) << NoMove << Bounce << Dig << Dive << Fly << FocusPunch << Geomancy << IceBurn
                              << ShadowForce << SkullBash << SkyAttack << SkyDrop << SolarBeam << RazorWind
                              << FreezeShock << PhantomForce;

        }
    };
    static FM forbidden_moves;

    static void gtc(int s, int, BS &b) {
        /* Explanation of Variables
         * s = Pokemon attacking
         * tar = Target initially selected
         * target = User of Follow Me/Rage Powder
         */
        int tar = b.opponent(b.player(s));
        if (turn(b,s).value("TargetChanged").toBool()) {
            return;
        }

        if (team(b, tar)["FollowMeTurn"] != b.turn())
            return;
        int target = team(b, tar)["FollowMePlayer"].toInt();
        if (b.koed(target) || !poke(b, target).contains("FollowMe") || b.player(s) == b.player(target) || forbidden_moves.contains(move(b,s)))
            return;

        int tarChoice = tmove(b,s).targets;
        bool muliTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

        if (muliTar) {
            return;
        }

        if (!b.canTarget(move(b,s), s, target)) {
            return;
        }

        //Sky Drop clears the redirect
        if (b.linked(tar, "FreeFalled")) {
            return;
        }

        int move = MM::move(b,target);
        if (move == Move::RagePowder && b.gen() >= 6) {
            if (b.hasType(s, Type::Grass) || b.hasWorkingAbility(s, Ability::Overcoat) || b.hasWorkingItem(s, Item::SafetyGoggles)) {
                return; //Rage powder doesn't affect them
            }
        }

        turn(b,s)["TargetChanged"] = true;
        turn(b,s)["Target"] = target;
    }
};
//Declaring satic class variable
MMFollowMe::FM MMFollowMe::forbidden_moves;

struct MMGravity : public MM
{
    MMGravity() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.battleMemory().value("Gravity").toBool()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(5, 0) : makeBracket(22, 0) ;
    }

    static void uas(int s, int, BS &b) {
        b.battleMemory()["Gravity"] = true;
        b.battleMemory()["GravityCount"] = 5;
        b.sendMoveMessage(53,0,s,type(b,s));

        std::vector<int> list = b.sortedBySpeed();

        foreach(int p, list) {
            if (b.koed(p))
                continue;
            if (b.isFlying(p)) {
                b.sendMoveMessage(53,2,p,Type::Psychic);
            }
            if(poke(b,p).value("Invulnerable").toBool()) {
                int move = poke(b,p)["2TurnMove"].toInt();
                if (move == Fly || move == Bounce || move == SkyDrop) {
                    MMBounce::groundStruck(p, b);
                    b.sendMoveMessage(53,3, p, Type::Psychic, s, poke(b,p)["2TurnMove"].toInt());
                }
            }
        }

        b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "Gravity", &et);
        addFunction(b.battleMemory(), "MovesPossible", "Gravity", &msp);
        addFunction(b.battleMemory(), "MovePossible", "Gravity", &mp);
    }

    static void et(int s, int, BS &b) {
        if (b.battleMemory().value("Gravity").toBool()) {
            int count = b.battleMemory()["GravityCount"].toInt() - 1;
            if (count <= 0) {
                b.sendMoveMessage(53,1,s,Pokemon::Psychic);
                b.removeEndTurnEffect(BS::FieldEffect, 0, "Gravity");
                removeFunction(b.battleMemory(), "MovesPossible", "Gravity");
                b.battleMemory()["Gravity"] = false;
            } else {
                b.battleMemory()["GravityCount"] = count;
            }
        }
    }

    struct FM : public QSet<int> {
        FM() {
            (*this) << Bounce << Fly << SkyDrop << JumpKick << HiJumpKick << Splash << MagnetRise << FlyingPress;
        }
    };
    static FM forbidden_moves;

    static void mp (int s, int, BS &b) {
        if (!b.battleMemory().value("Gravity").toBool()) {
            return;
        }

        int mv = move(b,s);
        if(forbidden_moves.contains(mv)) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(53,4,s,Type::Psychic,s,mv);
        }
    }

    static void msp (int s, int , BS &b) {
        if (!b.battleMemory().value("Gravity").toBool()) {
            return;
        }

        for (int i = 0; i < 4; i++) {
            if (forbidden_moves.contains(b.move(s, i))) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }
};

//Declaring static class variable
MMGravity::FM MMGravity::forbidden_moves;

struct MMSmackDown : public MM
{
    MMSmackDown() {
        functions["OnFoeOnAttack"] = &ofoa;
    }

    static void ofoa(int s, int t, BS &b) {
        if (b.isFlying(t)) {
            b.sendMoveMessage(175, 0, s, type(b,s), t);
            poke(b,t)["SmackedDown"] = true;
        }

        if (b.koed(t)) {
            return;
        }

        if(poke(b,t).value("Invulnerable").toBool()) {
            int move = poke(b,t)["2TurnMove"].toInt();
            if (move == Fly || move == Bounce || move == SkyDrop) {
                MMBounce::groundStruck(t, b);
            }
        }
    }
};

struct MMMetronome : public MM
{
    MMMetronome() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "Metronome");

        while (1) {
            int move = b.randint(MoveInfo::NumberOfMoves()-1)+1;

            if (!MoveInfo::Exists(move, b.gen())) {
                continue;
            }

            if (!forbidden.contains(move, b.gen())) {
                if (!b.hasMove(s, move) || b.gen() == 5 || b.gen() == 3) {
                    BS::BasicMoveInfo info = tmove(b,s);
                    MoveEffect::setup(move,s,t,b);
                    turn(b,s)["Target"] = b.randomValidOpponent(s);
                    b.useAttack(s,move,true,true);
                    MoveEffect::unsetup(move, s, b);
                    tmove(b,s) = info;
                    break;
                }
            }
        }
    }

    struct MMMetroSet : public QSet<int> {
        MMMetroSet() {
            (*this).unite(MMAssist::forbidden_moves );
            //The inheritance is missing a few moves
            (*this) << Move::IceBurn << Move::FreezeShock << Move::Quash << Move::QuickGuard << Move::RelicSong << Move::SecretSword
                                     << Move::TechnoBlast << Move::V_create << Move::WideGuard << Move::Snarl << Move::RagePowder << Move::AfterYou
                                     << Move::Bestow << Move::NaturePower << Move::Snore << Move::HyperspaceHole << Move::SteamEruption << Move::LightOfRuin
                                     << Move::DiamondStorm << Move::ThousandArrows << Move::ThousandWaves << Move::HoldHands << Move::Celebrate << Move::HappyHour;
        }
        bool contains(int move, Pokemon::gen gen) const {
            if (gen <= 4) {
                return MMAssist::forbidden_moves.contains(move, gen);
            } else {
                return QSet<int>::contains(move);
            }
        }
    };

    static MMMetroSet forbidden;
};

//Declaring static class variable
MMMetronome::MMMetroSet MMMetronome::forbidden;

struct MMWideGuard : public MM
{
    MMWideGuard() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "WideGuard", &dgaf);
        team(b,b.player(s))["WideGuardUsed"] = b.turn();
        b.sendMoveMessage(169, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        int target = b.player(t);
        if (!team(b,target).contains("WideGuardUsed") || team(b,target)["WideGuardUsed"].toInt() != b.turn()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        if (tmove(b,s).targets != Move::Opponents && tmove(b,s).targets != Move::All && tmove(b,s).targets != Move::AllButSelf) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        b.fail(s, 169, 0, Pokemon::Normal, t);
    }
};

struct MMQuickGuard : public MM
{
    MMQuickGuard() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "FastGuard", &dgaf);
        team(b,b.player(s))["QuickGuardUsed"] = b.turn();
        b.sendMoveMessage(170, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        int target = b.player(t);
        if (!team(b,target).contains("QuickGuardUsed") || team(b,target)["QuickGuardUsed"].toInt() != b.turn()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) && tmove(b,s).attack != Move::Feint) {
            return;
        }

        /* Quick Guard looks at the priority of the move, the raw one.
           If the priority was altered by Prankster or whatever,
           that doesn't matter in gen 5

           In gen 6, it does!
        */
        if (b.gen() <= 5) {
            if (MoveInfo::SpeedPriority(tmove(b,s).attack, b.gen()) <= 0) {
                return;
            }
        } else {
            if (tmove(b,s).priority <= 0) {
                return;
            }
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        b.fail(s, 170, 0, Pokemon::Normal, t);
    }
};

struct MMAvalanche : public MM
{
    MMAvalanche() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (turn(b,s).contains("DamageTakenBy") && turn(b,s)["DamageTakenBy"].toInt() == t) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMUTurn : public MM
{
    MMUTurn() {
        functions["UponAttackSuccessful"] = &uas;
        functions["AfterAttackFinished"] = &aas;
    }

    static void uas(int s, int, BS &b) {
        turn(b,s)["UTurnSuccess"] = true;
        turn(b,s)["UTurnCount"] = slot(b,s)["SwitchCount"];
    }

    static void aas(int s, int, BS &b) {
        if (!turn(b,s).contains("UTurnSuccess") || slot(b,s)["SwitchCount"] != turn(b,s)["UTurnCount"]) {
            return;
        }
        if (b.countAlive(b.player(s)) <= 1) {
            return;
        }
        if (b.koed(s)) {
            return;
        }
        b.requestSwitch(s);
    }
};

struct MMHiddenPower : public MM
{
    MMHiddenPower() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        quint8 *dvs = fpoke(b,s).dvs;

        int type = HiddenPowerInfo::Type(b.gen(), dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);
        tmove(b, s).type = type;
        if (b.gen() < 6) {
            tmove(b, s).power = HiddenPowerInfo::Power(b.gen(), dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);
        }

        /* In 3rd gen, hidden powers can be physical! */
        if (b.gen() <= 3) {
            tmove(b, s).category = TypeInfo::Category(type);
        }
    }
};

struct MMTrumpCard : public MM
{
    MMTrumpCard() {
        functions["BeforeTargetList"] = &bcd;
    }

    static void bcd(int s, int, BS &b)
    {
        int n = b.PP(s,fpoke(b,s).lastMoveSlot);
        int mult;
        switch(n) {
        case 0: mult = 200; break;
        case 1: mult = 80; break;
        case 2: mult = 60; break;
        case 3: mult = 50; break;
        default: mult = 40;
        }
        tmove(b, s).power = tmove(b, s).power * mult;
    }
};

struct MMSuperFang : public MM
{
    MMSuperFang() {
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        turn(b,s)["CustomDamage"] = b.poke(t).lifePoints()/2;
    }
};

struct MMPainSplit : public MM
{
    MMPainSplit() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        if (b.koed(t) || b.koed(s)) {
            return;
        }
        int sum = b.poke(s).lifePoints() + b.poke(t).lifePoints();
        b.changeHp(s, sum/2);
        b.changeHp(t, sum/2);
        b.sendMoveMessage(94, 0, s, type(b,s),t);
    }
};

struct MMPerishSong : public MM
{
    MMPerishSong() {
        functions["UponAttackSuccessful"] = &uas;
        functions["BeforeTargetList"] = &btl;
    }

    /* Perish Song is a move that affects all, and is affected by pressure.
       So we keep it an all target move until the execution,
       where we handle this differently. */
    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            b.targetList.clear();
            b.targetList.push_back(s);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(4,0) : gen <= 4 ? makeBracket(8, 0) : makeBracket(20, 0) ;
    }

    static void uas(int s, int, BS &b) {
        for (int t = 0; t < b.numberOfSlots(); t++) {
            if (poke(b,t).contains("PerishSongCount") || b.koed(t)) {
                continue;
            }
            if (b.hasWorkingAbility(t, Ability::Soundproof)) {
                b.sendAbMessage(57,0,t);
                continue;
            }
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "PerishSong", &et);
            poke(b, t)["PerishSongCount"] = tmove(b,s).minTurns + b.randint(tmove(b,s).maxTurns+1-tmove(b,s).maxTurns) - 1;
            poke(b, t)["PerishSonger"] = s;
        }
        b.sendMoveMessage(95);
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;

        int count = poke(b,s)["PerishSongCount"].toInt();

        b.sendMoveMessage(95,1,s,0,0,count);
        if (count > 0) {
            poke(b,s)["PerishSongCount"] = count - 1;
        } else {
            b.koPoke(s,s,false);
            b.selfKoer() = poke(b,s)["PerishSonger"].toInt();
        }
    }
};

struct MMHaze : public MM
{
    MMHaze() {
        functions["BeforeTargetList"] = &btl;
        functions["OnFoeOnAttack"] = &uas;
    }

    /* Haze is a move that affects all, and is affected by pressure.
       So we keep it an all target move until the execution,
       where we handle this differently. */
    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            b.targetList.clear();
            b.targetList.push_back(s);
        }
    }

    static void uas(int s, int t, BS &b) {
        if (b.gen() > 1) {
            if (tmove(b,s).power == 0) {
                b.sendMoveMessage(149);

                foreach (int p, b.sortedBySpeed())
                {
                    for (int i = 1; i <= 7; i++) {
                        fpoke(b,p).boosts[i] = 0;
                    }
                }
            }
            else {
                b.sendMoveMessage(149, 1, s, type(b,s), t);
                for (int i = 1; i <= 7; i++) {
                    fpoke(b,t).boosts[i] = 0;
                }
            }
        } else {
            /* In gen 1, haze just clears status */
            b.sendMoveMessage(149, 2);

            foreach (int p, b.sortedBySpeed())
            {
                b.healStatus(p, 0);
            }
        }
    }
};

struct MMLeechSeed : public MM
{
    MMLeechSeed() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasType(t, Pokemon::Grass) || (poke(b,t).contains("SeedSource"))) {
            b.fail(s, 72,0,Pokemon::Grass,t);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 4) : makeBracket(8, 0) ;
    }

    static void uas(int s, int t, BS &b) {
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "LeechSeed", &et);
        poke(b,t)["SeedSource"] = s;
        b.sendMoveMessage(72, 1, s, Pokemon::Grass, t);
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s) || b.hasWorkingAbility(s, Ability::MagicGuard))
            return;
        int s2 = poke(b,s)["SeedSource"].toInt();
        if (b.koed(s2))
            return;

        int denumerator = b.gen().num == 1 ? 16 : 8;
        int damage = std::min(int(b.poke(s).lifePoints()), std::max(b.poke(s).totalLifePoints() / denumerator, 1));

        b.sendMoveMessage(72, 2, s, Pokemon::Grass);
        b.inflictDamage(s, damage, s, false);

        if (b.koed(s2))
            return;

        if (b.hasWorkingItem(s2, Item::BigRoot)) {
            damage = damage * 13 / 10;
        }
        if (!b.hasWorkingAbility(s, Ability::LiquidOoze)) {
            if (b.canHeal(s2,BS::HealByEffect,0)) {
                b.sendMoveMessage(60, 0, s2);
                b.healLife(s2, damage);
            }
        } else {
            b.sendMoveMessage(1,2,s2,Pokemon::Poison,s);
            b.inflictDamage(s2, damage,s2,false);

            if (b.gen() >= 5 && b.koed(s2)) {
                b.selfKoer() = s2;
            }
        }
    }
};

struct MMRoost : public MM
{
    MMRoost() {
        functions["DetermineAttackFailure"] =  &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf (int s, int, BS &b) {
        if (b.poke(s).isFull())
            fturn(b,s).add(TM::Failed);
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(40, 0);
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(150,0,s,Pokemon::Flying);

        poke(b,s)["Roosted"] = true;
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "Roost", &et);
    }

    static void et(int s, int, BS &b) {
        poke(b,s)["Roosted"] = false;
    }
};

struct MMRest : public MM
{
    MMRest() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        // Insomnia, Vital Spirit, Uproar
        if ( (b.gen() >= 3 && b.poke(s).status() == Pokemon::Asleep) || !b.canGetStatus(s, Pokemon::Asleep) || b.poke(s).isFull()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        b.healLife(s, b.poke(s).totalLifePoints());
        b.sendMoveMessage(106,0,s,type(b,s));
        b.changeStatus(s, Pokemon::Asleep,false);
        b.poke(s).statusCount() =  2;
        b.poke(s).oriStatusCount() = 2;
        poke(b,s)["Rested"] = true;

        /* In GSC, when you rest when asleep thanks to sleep talk,
           sleep clause resets */
        if (b.currentForcedSleepPoke[b.player(s)] == b.currentInternalId(s)) {
            b.currentForcedSleepPoke[b.player(s)] = -1;
        }
    }
};

struct MMWish : public MM
{
    MMWish() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (slot(b,s).contains("WishTurn") && slot(b,s)["WishTurn"].toInt() >= b.turn()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(2, 0) : makeBracket(4, 0) ;
    }

    static void uas(int s, int, BS &b) {
        slot(b,s)["WishTurn"] = b.turn() + 1;
        slot(b,s)["Wisher"] = b.poke(s).nick();
        if (b.gen() >= 5)
            slot(b,s)["WishHeal"] = std::max(b.poke(s).totalLifePoints()/2, 1);
        b.addEndTurnEffect(BS::SlotEffect, bracket(b.gen()), s, "Wish", &et);
    }

    static void et(int s, int, BS &b) {
        int turn = slot(b,s)["WishTurn"].toInt();
        if (turn != b.turn()) {
            return;
        }
        if (!b.koed(s)) {
            b.sendMoveMessage(142, 0, 0, 0, 0, 0, slot(b,s)["Wisher"].toString());

            int life = b.gen() >= 5 ? slot(b, s)["WishHeal"].toInt() : b.poke(s).totalLifePoints()/2;
            b.healLife(s, life);
        }
        b.removeEndTurnEffect(BS::SlotEffect, s, "Wish");
    }
};

struct MMIngrain : public MM
{
    MMIngrain() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 0) : makeBracket(7, 0) ;
    }

    static void daf(int s, int , BS &b) {
        if (poke(b,s)["Rooted"].toBool() == true) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["Rooted"] = true;
        b.sendMoveMessage(151,0,s,Pokemon::Grass);
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "Ingrain", &et);
    }

    static void et(int s, int, BS &b) {
        if (b.canHeal(s,BS::HealByEffect,0) && poke(b,s)["Rooted"].toBool() == true) {
            int healing = b.poke(s).totalLifePoints()/16;

            if (b.hasWorkingItem(s, Item::BigRoot)) {
                healing = healing * 13 / 10;
            }

            b.sendMoveMessage(151,1,s,Pokemon::Grass);
            b.healLife(s, healing);
        }
    }
};

struct MMRoar : public MM
{
    MMRoar() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
        functions["AfterAttackFinished"] = &aaf;
    }

    static void daf(int s, int t, BS &b) {
        /* Roar, Whirlwind test phazing here */
        if (tmove(b,s).power == 0)
            testPhazing(s, t, b, true);
    }

    static bool testPhazing(int s, int t, BS &b, bool verbose) {
        if (b.gen().num == 1) {
            fturn(b,s).add(TM::Failed);
            return false;
        }

        if (b.gen().num == 2 && !b.hasMoved(t)) {
            fturn(b,s).add(TM::Failed);
            return false;
        }

        int target = b.player(t);

        /* ingrain & suction cups */
        if (poke(b,t).value("Rooted").toBool()) {
            if (verbose)
                b.fail(s, 107, 1, Pokemon::Grass,t);
            return false;
        } else if (b.hasWorkingAbility(t,Ability::SuctionCups)) {
            if (verbose)
                b.fail(s, 107, 0, 0, t);
            return false;
        } else{
            if (b.countBackUp(target) == 0) {
                if (verbose)
                    fturn(b,s).add(TM::Failed);
                return false;
            }
        }
        return true;
    }

    static void uas(int s, int t, BS &b) {
        /* Dragon tail, Judo Throw only test phazing here */
        if (tmove(b,s).power > 0 && !testPhazing(s, t, b, false))
            return;

        turn(b,s)["RoarSuccess"] = true;
        turn(b,s)["RoarTarget"] = t;
        turn(b,s)["RoarSwitchCount"] = slot(b,t)["SwitchCount"].toInt();
        return;
    }

    static void aaf(int s, int, BS &b) {
        if (!turn(b,s).contains("RoarSuccess") || b.koed(s))
            return;

        int t = turn(b,s)["RoarTarget"].toInt();

        if (turn(b,s)["RoarSwitchCount"] != slot(b,t)["SwitchCount"] || b.koed(t))
            return;

        QList<int> switches;
        int target = b.player(t);
        for (int i = 0; i < 6; i++) {
            if (!b.isOut(target, i) && !b.poke(target,i).ko()) {
                switches.push_back(i);
            }
        }
        b.sendBack(t, true);
        b.sendPoke(t, switches[b.randint(switches.size())], true);
        b.sendMoveMessage(107,2,s,type(b,s),t);
        b.callEntryEffects(t);
    }
};

struct MMSpikes : public MM
{
    MMSpikes() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (team(b,b.opponent(b.player(s))).value("Spikes").toInt() >= 1 + 2*(b.gen() >= 3)) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        team(b,t)["Spikes"] = std::min(3, team(b,t).value("Spikes").toInt()+1);
        addFunction(team(b,t), "UponSwitchIn", "Spikes", &usi);
        b.sendMoveMessage(121, 0, s, 0, t);
    }

    static void usi(int p, int slot, BS &b) {
        int spikeslevel = team(b,p).value("Spikes").toInt();
        if (spikeslevel <= 0 || b.koed(slot) || b.isFlying(slot) || b.hasWorkingAbility(slot, Ability::MagicGuard)) {
            return;
        }
        int n = 0;
        switch (spikeslevel) {
        case 1:
            n = 3; break;
        case 2:
            n = 4; break;
        case 3:
            n = 6; break;
        }

        b.sendMoveMessage(121,1,slot);
        b.inflictDamage(slot, b.poke(slot).totalLifePoints()*n/(8*3), slot);
    }
};

struct MMStealthRock : public MM
{
    MMStealthRock() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        if (team(b,t).value("StealthRock").toBool() == true) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        team(b,t)["StealthRock"] = true;
        addFunction(team(b,t), "UponSwitchIn", "StealthRock", &usi);
        b.sendMoveMessage(124,0,s,Pokemon::Rock,t);
    }

    static void usi(int source, int s, BS &b) {
        if (!b.koed(s) && team(b,source).value("StealthRock").toBool() == true && !b.hasWorkingAbility(s, Ability::MagicGuard))
        {
            b.sendMoveMessage(124,1,s,Pokemon::Rock);
            int n = b.rawTypeEff(Pokemon::Rock, s);
            b.inflictDamage(s, b.poke(s).totalLifePoints()*b.effFraction(n)/8, s);
        }
    }
};

struct MMToxicSpikes : public MM
{
    MMToxicSpikes() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        if (team(b,t).value("ToxicSpikes").toInt() >= 2) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        team(b,t)["ToxicSpikes"] = team(b,t)["ToxicSpikes"].toInt()+1;
        b.sendMoveMessage(136, 0, s, Pokemon::Poison, t);
        addFunction(team(b,t), "UponSwitchIn", "ToxicSpikes", &usi);
    }

    static void usi(int source, int s, BS &b) {
        if (!b.koed(s) && b.hasType(s, Pokemon::Poison) && !b.isFlying(s) && team(b,source).value("ToxicSpikes").toInt() > 0) {
            team(b,source).remove("ToxicSpikes");
            removeFunction(team(b,source), "UponSwitchIn", "ToxicSpikes");
            b.sendMoveMessage(136, 1, s, Pokemon::Poison);
            return;
        }
        if ((b.gen().num != 5 && b.hasSubstitute(s)) || b.isFlying(s)) {
            return;
        }
        if (team(b,source).value("SafeGuardCount").toInt() > 0) {
            return;
        }
        if (b.ability(source) == Ability::MagicGuard && b.gen() <= 4) {
            return;
        }

        int spikeslevel = team(b,source).value("ToxicSpikes").toInt();

        switch (spikeslevel) {
        case 0: return;
        case 1: b.inflictStatus(s, Pokemon::Poisoned, s); break;
        default: b.inflictStatus(s, Pokemon::Poisoned, s, 16, 16); break;
        }
    }
};

struct MMRapidSpin : public MM
{
    MMRapidSpin() {
        functions["UponAttackSuccessful"] = &uas;
        functions["AfterAttackSuccessful"] = &aas;
    }

    static void uas(int s, int, BS &b) {
        if (b.gen() > 4)
            return;
        exc(s,b);
    }

    static void aas(int s, int, BS &b) {
        if (b.gen() < 5)
            return;
        exc(s,b);
    }

    static void exc(int s, BS &b) {
        if (poke(b,s).contains("SeedSource")) {
            b.sendMoveMessage(103,1,s);
            poke(b,s).remove("SeedSource");
            b.removeEndTurnEffect(BS::PokeEffect, s, "LeechSeed");
        }
        int source = b.player(s);
        if (team(b,source).contains("Spikes")) {
            b.sendMoveMessage(103,2,source);
            team(b,source).remove("Spikes");
        }
        if (team(b,source).contains("ToxicSpikes")) {
            b.sendMoveMessage(103,3,source);
            team(b,source).remove("ToxicSpikes");
        }
        if (team(b,source).contains("StealthRock")) {
            b.sendMoveMessage(103,4,source);
            team(b,source).remove("StealthRock");
        }
        if (team(b,source).contains("StickyWeb")) {
            b.sendMoveMessage(103,5,source);
            team(b,source).remove("StickyWeb");
        }
        if (poke(b,s).contains("TrappedBy")) {
            b.sendMoveMessage(103,0,s,0,poke(b,s)["TrappedBy"].toInt(),poke(b,s)["TrappedMove"].toInt());
            poke(b,s).remove("TrappedBy");
            poke(b,s).remove("TrappedCount");
        }
    }
};

struct MMSubstitute : public MM
{
    MMSubstitute() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (fpoke(b,s).substitute()) {
            b.fail(s, 128);
        }
    }

    static void uas(int s, int, BS &b) {
        b.changeHp(s, b.poke(s).lifePoints() - std::max(b.poke(s).totalLifePoints()*25/100,1));
        fpoke(b,s).add(BS::BasicPokeInfo::Substitute);
        fpoke(b,s).substituteLife = b.poke(s).totalLifePoints()/4;
        b.sendMoveMessage(128,4,s);
        b.notifySub(s,true);
        //addFunction(poke(b,s), "BlockTurnEffects", "Substitute", &bte);
    }
};

struct MMNightShade : public MM
{
    MMNightShade() {
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = fpoke(b,s).level;
    }
};

struct MMAttract : public MM
{
    MMAttract() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (!b.isSeductionPossible(s,t) || b.linked(t, "Attract")){
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas (int s, int t, BS &b) {
        if (b.hasWorkingTeamAbility(t, Ability::AromaVeil)) {
            b.sendAbMessage(112,1,t);
            return;
        }
        b.sendMoveMessage(58,1,s,0,t);
        if (b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            b.link(s, t, "Attract");
            addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);

            if (b.hasWorkingItem(t, Item::DestinyKnot) && b.isSeductionPossible(t, s) && !b.linked(s, "Attract")) {
                b.link(t, s, "Attract");
                addFunction(poke(b,s), "DetermineAttackPossible", "Attract", &pda);
                b.sendItemMessage(41,t,0,s);
            }
        }
    }

    static void pda(int s, int, BS &b) {
        if (fturn(b,s).contains(TM::HasPassedStatus))
            return;
        if (b.linked(s, "Attract")) {
            int seducer = b.linker(s, "Attract");

            b.sendMoveMessage(58,0,s,0,seducer);
            if (b.coinflip(1, 2)) {
                turn(b,s)["ImpossibleToMove"] = true;
                b.sendMoveMessage(58, 2,s);
            }
        }
    }
};

struct MMKnockOff : public MM
{
    MMKnockOff() {
        functions["BasePowerModifier"] = &bh;
        functions["KnockOff"] = &aas;
    }
    static void bh(int s, int t, BS &b) {
        if ((b.canLoseItem(t,s) || (b.hasWorkingAbility(t, Ability::StickyHold) && b.poke(t).item() != 0))&& b.gen() > 5) {
            b.chainBp(s, 10);
        }
    }

    static void aas(int s,int t,BS &b) {
        //Knock off if target is dead, but not if attacker
        if (!b.koed(s)) {
            if (b.canLoseItem(t,s)) {
                b.sendMoveMessage(70,0,s,type(b,s),t,b.poke(t).item());
                b.loseItem(t);
                b.battleMemory()[QString("KnockedOff%1%2").arg(b.player(t)).arg(b.currentInternalId(t))] = true;
            } else if (b.hasWorkingAbility(t, Ability::StickyHold) && b.poke(t).item() != 0) {
                b.sendAbMessage(121,1,t);
            }
        }
    }
};

struct MMSwitcheroo : public MM
{
    MMSwitcheroo() {
        functions["OnFoeOnAttack"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        int i1 = b.poke(s).item();
        int i2 = b.poke(t).item();
        if (b.koed(t) || b.koed(s) || (i1 == 0 && i2 == 0) || (i2 != 0 && !b.canLoseItem(t, s)) || (i1 != 0 && !b.canLoseItem(s, s)) ||
                !b.canPassMStone(t, i1) || !b.canPassMStone(s, i2)) {
            if (b.hasWorkingAbility(t, Ability::StickyHold)) {
                b.sendAbMessage(121,0,t);
            }
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b)
    {
        b.sendMoveMessage(132,0,s,type(b,s),t);
        int i1(b.poke(s).item()), i2(b.poke(t).item());
        if (i2)
            b.sendMoveMessage(132,1,s,type(b,s),t,i2);
        b.acqItem(s, i2);
        if (i1)
            b.sendMoveMessage(132,1,t,type(b,s),s,i1);
        b.acqItem(t, i1);
    }
};

struct MMMetalBurst : public MM
{
    MMMetalBurst() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void ms (int s, int, BS &b) {
        tmove(b,s).targets = Move::ChosenTarget;
        turn(b,s)["Target"] = turn(b,s).value("DamageTakenBy").toInt();
    }

    static void daf (int s, int, BS &b) {
        int dam = fturn(b,s).damageTaken;
        if (dam == 0) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        turn(b,s)["CounterDamage"] = dam * 3 / 2;
    }

    static void cad(int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = turn(b,s)["CounterDamage"];
    }
};

struct MMTaunt : public MM
{
    MMTaunt() {
        functions["OnFoeOnAttack"] = &uas;
        functions["DetermineAttackFailure"]=  &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (b.counters(t).hasCounter(BC::Taunt) || (b.gen() > 5 && b.hasWorkingAbility(t, Ability::Oblivious)))
            fturn(b,s).add(TM::Failed);
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 14) : makeBracket(12, 0) ;
    }

    static void uas (int s, int t, BS &b) {
        if (b.hasWorkingTeamAbility(t, Ability::AromaVeil)) {
            b.sendAbMessage(112,1,t);
            return;
        }
        b.sendMoveMessage(134,1,s,Pokemon::Dark,t);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            addFunction(poke(b,t), "MovesPossible", "Taunt", &msp);
            addFunction(poke(b,t), "MovePossible", "Taunt", &mp);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Taunt", &et);

            if (b.gen() <= 3) {
                b.counters(t).addCounter(BC::Taunt, 1);
            } else if (b.gen().num == 4) {
                b.counters(t).addCounter(BC::Taunt, 2 + (b.randint(3)));
            } else {
                b.counters(t).addCounter(BC::Taunt, 2);
            }
        }
    }

    static void et(int s, int, BS &b)
    {
        if (b.koed(s))
            return;

        if (b.counters(s).count(BC::Taunt) < 0) {
            removeFunction(poke(b,s), "MovesPossible", "Taunt");
            removeFunction(poke(b,s), "MovePossible", "Taunt");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Taunt");
            if (b.gen() >= 4)
                b.sendMoveMessage(134,2,s,Pokemon::Dark);
            b.counters(s).removeCounter(BC::Taunt);
        }
    }

    static void msp(int s, int, BS &b) {
        if (!b.counters(s).hasCounter(BC::Taunt)) {
            return;
        }
        for (int i = 0; i < 4; i++) {
            if (MoveInfo::Power(b.move(s,i), b.gen()) == 0) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }

    static void mp(int s, int, BS &b) {
        /* Taunt doesn't count when bouncing a move */
        if (!b.counters(s).hasCounter(BC::Taunt) || b.battleMemory().value("CoatingAttackNow").toBool()) {
            return;
        }

        int move = turn(b,s)["MoveChosen"].toInt();
        if (move != NoMove && MoveInfo::Power(move, b.gen()) == 0) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(134,0,s,Pokemon::Dark,s,move);
        }
    }
};

struct MMGastroAcid : public MM
{
    MMGastroAcid() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.ability(t) == Ability::Multitype || poke(b,t).value("AbilityNullified").toBool() || b.ability(t) == Ability::StanceChange) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(51,0,s,type(b,s),t,b.ability(t));
        b.loseAbility(t);
        poke(b,t)["AbilityNullified"] = true;
    }
};

struct MMGrassKnot : public MM
{
    MMGrassKnot() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (b.gen() <= 2)
            return;

        int weight = b.weight(t);
        int bp;
        /* I had to make some hacks due to the floating point precision, so this is a '<' here and not
    a '<='. Will be fixed if someone wants to do it */
        if (weight <= 100) {
            bp = 20;
        } else if (weight <= 250) {
            bp = 40;
        } else if (weight <= 500) {
            bp = 60;
        } else if (weight <= 1000) {
            bp = 80;
        } else if (weight <= 2000) {
            bp = 100;
        } else {
            bp = 120;
        }
        tmove(b, s).power = tmove(b,s).power * bp;
    }
};

struct MMGrudge : public MM
{
    MMGrudge() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["GrudgeTurn"] = b.turn();
        addFunction(poke(b,s), "AfterKoedByStraightAttack", "Grudge", &akbst);
    }

    static void akbst(int s, int t, BS &b) {
        int trn = poke(b,s)["GrudgeTurn"].toInt();

        if (trn == b.turn() || (trn+1 == b.turn() && !fturn(b,s).contains(TM::HasMoved))) {
            if (!b.koed(t)) {
                int slot = fpoke(b, t).lastMoveSlot;
                b.sendMoveMessage(54,0,s,Pokemon::Ghost,t,b.move(t,slot));
                b.losePP(t, slot, 64);
            }
        }
    }
};

struct MMBoostSwap : public MM
{
    MMBoostSwap() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        QStringList args = turn(b,s)["BoostSwap_Arg"].toString().split('_');
        foreach(QString str, args) {
            std::swap(fpoke(b,s).boosts[str.toInt()], fpoke(b,t).boosts[str.toInt()]);
        }
        b.sendMoveMessage(55,0,s,type(b,s),t);
    }
};

struct MMGyroBall : public MM
{
    MMGyroBall() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd (int s, int t, BS &b) {
        bool speed = turn(b,s)["GyroBall_Arg"].toInt() == 1;

        int bp = 1 + 25 * b.getStat(speed ? s : t,Speed) / b.getStat(speed ? t : s,Speed);
        bp = std::max(2,std::min(bp,150));

        tmove(b, s).power = tmove(b, s).power * bp;
    }
};

struct MMWeather : public MM
{
    MMWeather() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    struct WI : public QMap<int,int> {
        WI() {
            insert(BS::SandStorm, Item::SmoothRock); /* Soft Rock */
            insert(BS::Hail, Item::IcyRock); /* Icy Rock */
            insert(BS::Rain, Item::DampRock); /* Damp Rock */
            insert(BS::Sunny, Item::HeatRock); /* Heat Rock */
        }
    };
    static WI weather_items;

    static void daf(int s, int, BS &b) {
        if (b.weather == turn(b,s)["Weather_Arg"].toInt())
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int, BS &b) {
        int weather = turn(b,s)["Weather_Arg"].toInt();

        b.sendMoveMessage(57,weather-1,s,type(b,s));
        if (weather_items.contains(weather) && b.hasWorkingItem(s,weather_items[weather])) {
            b.callForth(weather,8);
        } else {
            b.callForth(weather,5);
        }
    }
};

//Declaring static class variable
MMWeather::WI MMWeather::weather_items;

struct MMThunder : public MM
{
    MMThunder() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Rain)) {
            tmove(b, s).accuracy = 0;
        } else if (b.isWeatherWorking(BattleSituation::Sunny)) {
            tmove(b, s).accuracy = tmove(b, s).accuracy * 5 / 7;
        }
    }
};

struct MMWeatherBall : public MM
{
    MMWeatherBall() {
        functions["MoveSettings"] = &ms;
    }

    static void ms (int s, int, BS &b) {
        int weather = b.weather;

        if (weather != BattleSituation::NormalWeather && b.isWeatherWorking(weather)) {
            tmove(b, s).power = tmove(b, s).power * 2;
            tmove(b,s).type = TypeInfo::TypeForWeather(weather);
        }
    }
};

struct MMHealingWish : public MM
{
    MMHealingWish() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &btl;
        /* Has to be put there so it can be unsetup by calling moves */
        functions["AfterAttackFinished"] = &aaf;
    }

    static void daf(int s, int, BS &b) {
        if (b.countBackUp(b.player(s)) == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void btl(int s, int, BS &b) {
        turn(b,s)["HealingWishSuccess"] = true;
        b.koPoke(s,s,false); /*...*/
    }

    static void aaf(int s, int, BS &b) {
        if (!turn(b,s).contains("HealingWishSuccess"))
            return;
        /* In gen 5, it triggers before entry hazards */
        addFunction(turn(b,s), b.gen().num == 4 ? "AfterSwitchIn" : "UponSwitchIn", "HealingWish", &asi);

        /* On gen 5 and further, the pokemon is switched at the end of the turn! */
        if (b.gen() <= 4)
            b.requestSwitch(s);
    }

    static void asi(int s, int, BS &b) {
        if (!b.koed(s)) {
            int t = type(b,s);
            b.sendMoveMessage(61,move(b,s) == HealingWish ? 1 : 2,s,t);
            b.sendMoveMessage(61,0,s,t);
            b.healLife(s,b.poke(s).totalLifePoints());
            b.changeStatus(s, Pokemon::Fine);
            if (move(b,s) == LunarDance) {
                for(int i = 0; i < 4; i++) {
                    b.gainPP(s, i, 100);
                }
            }
            removeFunction(turn(b,s), "AfterSwitchIn", "HealingWish");
        }
    }
};

struct MMPowerTrick : public MM
{
    MMPowerTrick() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["PowerTricked"] = true;
        b.sendMoveMessage(62,0,s,type(b,s));
    }
};

/* Heal block:
   For 5 turns, the target cannot select or execute any of the following moves:

If Pokémon under the effect of Heal Block receives the effects of Wish, Wish will fail to heal. If a Pokemon uses Wish, is hit by Heal Block, and then switches out to another Pokemon, Wish will heal that Pokemon.

Aqua Ring and Ingrain do not heal their user while under the effects of Heal Block.

Leech Seed can be used and will still damage its target, but will not heal the user. Absorb, Drain Punch, Dream Eater, Giga Drain, Leech Life, and Mega Drain will also damage their target, but will not heal the user
*/

struct MMHealBlock: public MM
{
    MMHealBlock() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }
    static void daf(int s, int t, BS &b) {
        if (poke(b,t).value("HealBlockCount").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 16) : makeBracket(17, 0) ;
    }

    static void uas(int s, int t, BS &b) {
        if (b.hasWorkingTeamAbility(t, Ability::AromaVeil)) {
            b.sendAbMessage(112,1,t);
            return;
        }
        b.sendMoveMessage(59,0,s,type(b,s),t);
        if (b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            poke(b,t)["HealBlockCount"] = 5;
            poke(b,t)["HealBlocked"] = true;
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "HealBlock", &et);
            addFunction(poke(b,t), "MovePossible", "HealBlock", &mp);
            addFunction(poke(b,t), "MovesPossible", "HealBlock", &msp);
        }
    }
    static void et(int s, int , BS &b) {
        inc(poke(b,s)["HealBlockCount"], -1);
        int count = poke(b,s)["HealBlockCount"].toInt();

        if (count == 0) {
            b.sendMoveMessage(59,1,s,Type::Psychic);
            b.removeEndTurnEffect(BS::PokeEffect, s, "HealBlock");
            removeFunction(poke(b,s), "MovesPossible", "HealBlock");
            removeFunction(poke(b,s), "MovePossible", "HealBlock");
            poke(b,s).remove("HealBlocked");
        }
    }

    static void msp(int s, int, BS &b) {
        for (int i = 0; i < 4; i++) {
            if ((MoveInfo::Flags(b.move(s, i), b.gen()) & Move::HealingFlag
                    || (b.gen() >= 6 && MoveInfo::Recoil(b.move(s,i), b.gen()) > 0 )) && b.move(s,i) != Move::HealPulse) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }

    static void mp(int s, int, BS &b) {
        int mv = move(b,s);
        if((tmove(b,s).flags & Move::HealingFlag
                || (b.gen() >= 6 && tmove(b,s).recoil > 0)) && mv != Move::HealPulse) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.notify(BS::All, BattleCommands::UseAttack, s, qint16(move(b,s)), false);
            b.sendMoveMessage(59,BS::HealByMove,s,Type::Psychic,s,mv);
        }
    }
};

struct MMJumpKick : public MM
{
    MMJumpKick() {
        functions["AttackSomehowFailed"] = &asf;
    }

    static void asf(int s, int t, BS &b) {
        int damage;
        if (b.gen() >= 5)
            damage = b.poke(s).totalLifePoints()/2;
        else {
            int typemod;
            int typeadv[] = {b.getType(t, 1), b.getType(t, 2)};
            int type = MM::type(b,s);
            if (typeadv[0] == Type::Ghost) {
                if (b.gen() <= 3)
                    return;
                typemod = b.convertTypeEff(TypeInfo::Eff(type, typeadv[1]));
            } else if (typeadv[1] == Type::Ghost) {
                if (b.gen() <= 3)
                    return;
                typemod = b.convertTypeEff(TypeInfo::Eff(type, typeadv[0]));
            } else {
                typemod = b.convertTypeEff(TypeInfo::Eff(type, typeadv[0])) + b.convertTypeEff(TypeInfo::Eff(type, typeadv[1]));
            }

            fturn(b,s).typeMod = typemod;
            fturn(b,s).stab = b.hasType(s, Type::Fighting) ? 3 : 2;
            if (b.gen().num == 4)
                damage = std::min(b.calculateDamage(s,t)/2, b.poke(t).totalLifePoints()/2);
            else
                damage = std::min(b.calculateDamage(s,t)/8, b.poke(t).totalLifePoints()/2);
        }
        b.sendMoveMessage(64,0,s,Type::Fighting);
        b.inflictDamage(s, damage, s, true);
    }
};

struct MMDefenseCurl : public MM
{
    MMDefenseCurl() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        poke(b,s)["DefenseCurl"] = true;
    }
};

struct MMIceBall : public MM
{
    MMIceBall() {
        functions["UponAttackSuccessful"] = &uas;
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        if (! (poke(b,s).contains("LastBallTurn") && poke(b,s).value("LastBallTurn").toInt() + 1 == b.turn()) ) {
            poke(b,s)["IceBallCount"] = 0;
        }
        if (poke(b,s).contains("DefenseCurl")) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
        tmove(b, s).power = tmove(b, s).power * (1+poke(b,s)["IceBallCount"].toInt());
    }

    static void uas(int s, int, BS &b) {
        if (b.poke(s).status() == Pokemon::Asleep)
            return;

        int count = poke(b,s)["IceBallCount"].toInt();
        if (b.turn() - 1 != poke(b,s)["LastBallTurn"].toInt()) {
            count = 0;
        }
        if (count >= 15) {
            poke(b,s)["IceBallCount"] = 0;
        } else {
            poke(b,s)["IceBallCount"] = count*2+1;
        }
        poke(b,s)["LastBallTurn"] = b.turn();
        addFunction(poke(b,s), "TurnSettings", "IceBall", &ts);
    }

    static void ts(int s, int t, BS &b) {
        if (poke(b,s).contains("LastBallTurn") && poke(b,s)["LastBallTurn"].toInt() + 1 == b.turn() && poke(b,s)["IceBallCount"].toInt() > 0) {
            fturn(b,s).add(TM::NoChoice);
            MoveEffect::setup(fpoke(b,s).lastMoveUsed,s,t,b);
        }
    }
};

struct MMImprison : public MM
{
    MMImprison() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.gen() >= 5)
            return;

        /* let's just see if there are moves to imprison */
        QList<int> foes = b.revs(s);

        bool success = false;

        foreach(int foe, foes) {
            for (int i = 0; i < 4; i++)
                if (b.move(s,i) != 0)
                    for (int j = 0; j < 4; j++)
                        if (b.move(foe,j) == b.move(s,i))
                            success = true;
        }

        if (!success) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "MovePossible", "Imprison", &mp);
        addFunction(b.battleMemory(), "MovesPossible", "Imprison", &msp);
        poke(b,s)["Imprisoner"] = true;
        b.sendMoveMessage(67,0,s,type(b,s));
    }

    static void mp(int s, int, BS &b) {
        QList<int> foes = b.revs(s);

        int attack = move(b,s);

        foreach(int foe, foes) {
            if (!poke(b,foe).value("Imprisoner").toBool()) {
                continue;
            }

            for (int i = 0; i < 4; i++) {
                if (b.move(foe,i) == attack) {
                    turn(b,s)["ImpossibleToMove"] = true;
                    b.sendMoveMessage(67,1,s,Pokemon::Psychic,foe,attack);
                    return;
                }
            }
        }
    }

    static void msp(int s, int, BS &b) {
        /* let's just see if there are moves to imprison */
        QList<int> foes = b.revs(s);

        foreach(int foe, foes) {
            if (!poke(b,foe).value("Imprisoner").toBool()) {
                continue;
            }

            for (int i = 0; i < 4; i++)
                if (b.move(s,i) != 0)
                    for (int j = 0; j < 4; j++)
                        if (b.move(foe,j) == b.move(s,i))
                            turn(b,s)["Move"+QString::number(i) + "Blocked"] = true;
        }
    }
};

struct MMMagnetRise : public MM
{
    MMMagnetRise() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.hasWorkingAbility(s,Ability::Levitate) || poke(b,s).value("Rooted").toBool() || poke(b,s).value("MagnetRise").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 15) : makeBracket(15, 0) ;
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(68,0,s,Pokemon::Electric);
        poke(b,s)["MagnetRiseCount"] = 5;
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "MagnetRise", &et);
    }

    static void et(int s, int, BS &b) {
        inc(poke(b,s)["MagnetRiseCount"], -1);
        int count = poke(b,s)["MagnetRiseCount"].toInt();

        if (count == 0 && !b.koed(s)) {
            b.sendMoveMessage(68,1,s, Type::Electric);
            b.removeEndTurnEffect(BS::PokeEffect, s, "MagnetRise");
        }
    }
};

struct MMJudgment : public MM
{
    MMJudgment() {
        functions["MoveSettings"] = &ms;
    }

    static void ms (int s, int, BS &b) {
        int item = b.poke(s).item();
        if (ItemInfo::isPlate(item) && b.hasWorkingItem(s, item) && !turn(b,s).value("Symbiote").toBool()) {
            tmove(b,s).type = poke(b,s)["ItemArg"].toInt();
        }
    }
};

struct MMLastResort : public MM
{
    MMLastResort() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        if (b.move(s, 1) == 0) {
            /* The user only has 1 move */
            fturn(b,s).add(TM::Failed);
            return;
        }
        bool succ = true;
        int slot = fpoke(b,s).lastMoveSlot;
        for (int i = 0; i < 4; i++) {
            if (i != slot && b.move(s,i) != 0 && !poke(b,s).value(QString("Move%1Used").arg(i)).toBool()) {
                succ= false;
            }
        }
        if (!succ) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMTeamBarrier : public MM
{
    MMTeamBarrier() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.gen().num == 1) { MMTeamBarrier::daf1(s,s,b); return; }

        int cat = turn(b,s)["TeamBarrier_Arg"].toInt();
        int source = b.player(s);

        if (team(b,source).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(7, 0) : gen <= 4 ? makeBracket(1, 0) : makeBracket(21, 0) ;
    }

    static void uas(int s, int, BS &b) {
        if (b.gen().num == 1) { MMTeamBarrier::uas1(s,s,b); return; }

        int source = b.player(s);

        int nturn;
        if (b.hasWorkingItem(s, Item::LightClay)) { /* light clay */
            nturn = 8;
        } else {
            nturn = 5;
        }
        int cat = turn(b,s)["TeamBarrier_Arg"].toInt();

        b.sendMoveMessage(73,(cat-1)+b.multiples()*2,s,type(b,s));
        team(b,source)["Barrier" + QString::number(cat) + "Count"] = nturn;

        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), source, "TeamBarrier", &et);
    }

    static void et(int s, int, BS &b) {
        int counts[] = {team(b,s).value("Barrier1Count").toInt(), team(b,s).value("Barrier2Count").toInt()};

        for (int i = 0; i < 2; i++) {
            if (counts[i] != 0) {
                team(b,s)["Barrier" + QString::number(i+1) + "Count"] = counts[i] - 1;
                if (counts[i] == 1) {
                    b.sendMoveMessage(73, 4+i,s,Pokemon::Psychic);
                }
            }
        }
    }

    static void daf1(int s, int, BS &b) {
        int cat = turn(b,s)["TeamBarrier_Arg"].toInt();
        if (poke(b,s).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas1(int s, int, BS &b) {
        int cat = turn(b,s)["TeamBarrier_Arg"].toInt();

        b.sendMoveMessage(73,(cat-1)+b.multiples()*2,s,type(b,s));
        poke(b,s)["Barrier" + QString::number(cat) + "Count"] = 1;
    }
};

struct MMLockOn : public MM
{
    MMLockOn() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        poke(b,s)["LockedOnEnd"] = b.turn() + 1;
        poke(b,s)["LockedOn"] = t;
        poke(b,s)["LockedOnCount"] = slot(b,t).value("SwitchCount");

        b.sendMoveMessage(74,0,s,type(b,s),t);
    }
};

struct MMLuckyChant : public MM
{
    MMLuckyChant() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(1, 5) : makeBracket(21, 5) ;
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(75,0,s,type(b,s));

        int source = b.player(s);

        team(b,source)["LuckyChantCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), source, "LuckyChant", &et);
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["LuckyChantCount"], -1);
        int count = team(b,s)["LuckyChantCount"].toInt();

        if (count == 0) {
            b.sendMoveMessage(75,1,s);
            b.removeEndTurnEffect(BS::ZoneEffect, s, "LuckyChant");
        }
    }
};

struct MMMagicCoat : public MM
{
    MMMagicCoat() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure2", "MagicCoat", &dgaf);
        turn(b,s)["MagicCoated"] = true;
        b.sendMoveMessage(76,0,s,Pokemon::Psychic);
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

        if (t != s && turn(b,t).value("MagicCoated").toBool() ) {
            target = t;
        } else {
            /* Entry hazards */
            if (tmove(b,s).targets == Move::OpposingTeam) {
                foreach(int t, b.revs(s)) {
                    if (b.koed(t)) {
                        continue;
                    }
                    if (turn(b,t).value("MagicCoated").toBool()) {
                        target = t;
                        break;
                    }
                }
            }
        }

        if (target == -1)
            return;

        int move = MM::move(b,s);

        b.fail(s, 76, 1, Pokemon::Psychic);
        /* Now Bouncing back ... */
        BS::context ctx = turn(b,target);
        BS::BasicMoveInfo info = tmove(b,target);

        turn(b,target).clear();
        MoveEffect::setup(move,target,s,b);
        turn(b,target)["Target"] = s;
        b.battleMemory()["CoatingAttackNow"] = true;
        b.useAttack(target,move,true,false);
        b.battleMemory().remove("CoatingAttackNow");

        /* Restoring previous state. Only works because moves reflected don't store useful data in the turn memory,
            and don't cause any such data to be stored in that memory */
        turn(b,target) = ctx;
        tmove(b,target) = info;
    }
};

struct MMDefog : public MM
{
    MMDefog() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
        bool dropevade = false;
        //Evasion drop is blocked by sub. Flags prevent using canBypassSub.
        if (!b.hasMinimalStatMod(t, Evasion) && (!b.hasSubstitute(t) || (b.gen() >= 6 && b.hasWorkingAbility(s, Ability::Infiltrator)))) {
            b.inflictStatMod(t, Evasion, -1, s);
            dropevade = true;
        }

        bool clear = false;

        QVector<int> players = QVector<int>() << b.player(t);
        //Defog only clears Reflect/Lightscreen from opponent's side, even in 6th gen
        foreach (int p, players) {
            BS::context &c = team(b,p);

            if (c.remove("Barrier1Count") + c.remove("Barrier2Count")) {
                clear = true;
            }
        }
        if (b.gen() >= 6) {
            players.push_back(b.opponent(b.player(t)));
        }

        foreach (int p, players) {
            BS::context &c = team(b,p);

            if (c.remove("Spikes") + c.remove("ToxicSpikes") + c.remove("StealthRock") + c.remove("MistCount") + c.remove("SafeGuardCount") + c.remove("StickyWeb")) {
                clear = true;
            }
        }

        if (clear) {
            b.sendMoveMessage(77,b.gen() >= 6, s, type(b,s), t);
        } else if (!dropevade){
            //Let's give feedback if nothing occurs due to no hazards + evade drop blocked by Sub
            b.notifyFail(s);
        }
    }
};

struct MMMagnitude: public MM
{
    MMMagnitude() {
        functions["BeforeTargetList"] = &bcd;
        functions["BeforeCalculatingDamage"] = &bh;
    }

    static void bcd(int s, int, BS &b) {

        int pow, magn;

        if (b.gen() >= 3) {
            int randnum = b.randint(20);

            switch (randnum) {
            case 0: magn = 4; pow = 10; break;
            case 1: case 2: magn = 5; pow = 30; break;
            case 3: case 4: case 5: case 6: magn = 6; pow = 50; break;
            case 7: case 8: case 9: case 10: case 11: case 12: magn = 7; pow = 70; break;
            case 13: case 14: case 15: case 16: magn = 8; pow = 90; break;
            case 17: case 18: magn = 9; pow = 110; break;
            case 19: default: magn = 10; pow = 150; break;
            }
        }
        else { // gen 2 has slightly different probabilities, 14, 25, 51, 77, 51, 25, 13
            int randnum = b.randint(256);

            if (randnum < 14) {
                magn = 4; pow = 10;
            }
            else if (randnum < 39) {
                magn = 5; pow = 30;
            }
            else if (randnum < 90) {
                magn = 6; pow = 50;
            }
            else if (randnum < 167) {
                magn = 7; pow = 70;
            }
            else if (randnum < 218) {
                magn = 8; pow = 90;
            }
            else if (randnum < 243) {
                magn = 9; pow = 110;
            }
            else {
                magn = 10; pow = 150;
            }
        }

        turn(b,s)["MagnitudeLevel"] = magn;
        tmove(b, s).power = tmove(b, s).power * pow;
    }

    static void bh(int s, int t, BS &b) {
        b.sendMoveMessage(78, 0, s, type(b,s), t, turn(b,s)["MagnitudeLevel"].toInt());
    }
};

struct MMMeFirst : public MM
{
    MMMeFirst() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        tmove(b,s).power = 0;
    }

    static void daf(int s, int t, BS &b) {
        /* if has moved */
        if (b.koed(t) || fturn(b,t).contains(TM::HasMoved)) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int num = move(b,t);
        if (MoveInfo::Power(num, b.gen()) == 0 || num == Move::MeFirst) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        turn(b,s)["MeFirstAttack"] = num;
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "DetermineAttackFailure", "MeFirst");
        removeFunction(turn(b,s), "UponAttackSuccessful", "MeFirst");
        removeFunction(turn(b,s), "MoveSettings", "MeFirst");
        int move = turn(b,s)["MeFirstAttack"].toInt();
        MoveEffect::setup(move,s,t,b);
        tmove(b,s).power = tmove(b,s).power * 3 / 2;
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s,move,true,true);
        MoveEffect::unsetup(move,s,b);
    }
};

struct MMMimic : public MM
{
    MMMimic() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    struct FailedMoves : public QSet<int> {
        FailedMoves() {
            //Technically, Mimic and Sketch are capable of being copied in Gen II, but we ignore it because it is cyclical.
            //Metronome might be able to be copied in Generations 1, 2 and 5. However, it is unclear if it copies Metronome or the move used.
            //For now, we will just block it
            (*this) << NoMove << Struggle << Sketch << Mimic << Chatter << Metronome;
        }
        bool contains(int move, Pokemon::gen gen) const {
            //Transform blocked in Gen 5, possibly as a result of Transform+Mimic glitch, but this won't affect anything in a simulator
            if (move == Transform) {
                return gen >= 5;
            } else {
                return QSet<int>::contains(move);
            }
        }
    };

    static FailedMoves FM;

    static void daf(int s, int t, BS &b) {
        /* Mimic doesn't fail in Gen 1 */
        if (b.gen().num == 1) {
            return;
        }
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
        if (tu + 1 < b.turn() || (tu + 1 == b.turn() && fturn(b,t).contains(TM::HasMoved))) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int move = poke(b,t)["LastMoveUsed"].toInt();
        if (b.hasMove(s,move) || FM.contains(move, b.gen())) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void uas(int s, int t, BS &b) {
        int move = poke(b,t)["LastMoveUsed"].toInt();
        /* Mimic copies a random move in Gen 1 */
        if (b.gen().num == 1) {
            move = 0;
            while (move == 0) {
                move = b.move(t, b.randint(4));
            }
        }
        int slot = fpoke(b,s).lastMoveSlot;
        //Following check is needed to make sure "Mimic" is replaced, and not other moves, like Sleep Talk.
        for(int i = 0; i < 4; i++) {
           if (b.move(s,i) == Move::Mimic) {
               slot = i;
               break;
           }
        }

        //Gen 5+ Mimic gives a full PP count. We need to apply the 60% from PP ups
        int pp = b.gen() > 4 ? (MoveInfo::PP(move, b.gen()) * 8/5) : 5;
        b.changeTempMove(s, slot, move, pp);
        b.sendMoveMessage(81,0,s,type(b,s),t,move);
    }
};

//Declaring static class variable
MMMimic::FailedMoves MMMimic::FM;

struct MMMinimize : public MM
{
    MMMinimize() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int , BS &b) {
        poke(b,s)["Minimize"] = true;
    }
};

struct MMMiracleEye : public MM
{
    MMMiracleEye() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        poke(b,t)[turn(b,s)["MiracleEye_Arg"].toString() + "Sleuthed"] = true;
        poke(b,t)["Sleuthed"] = true;
        b.sendMoveMessage(84,0,s,type(b,s),t);
    }
};

struct MMMirrorMove : public MM
{
    MMMirrorMove() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int , BS &b) {
        if(!poke(b,s).contains("MirrorMoveMemory")) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        removeFunction(turn(b,s), "DetermineAttackFailure", "MirrorMove");
        removeFunction(turn(b,s), "UponAttackSuccessful", "MirrorMove");

        int move = poke(b,s)["MirrorMoveMemory"].toInt();
        BS::BasicMoveInfo info = tmove(b,s);
        MoveEffect::setup(move,s,s,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s,move,true,true);
        MoveEffect::unsetup(move,s,b);
        tmove(b,s) = info;
    }
};

struct MMMist : public MM
{
    MMMist() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"]=  &daf;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(7, 3) : gen <= 4 ? makeBracket(1, 2) : makeBracket(21, 3) ;
    }

    static void daf(int s, int, BS &b) {
        int count = team(b,b.player(s))["MistCount"].toInt();
        if (count > 0)
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(86,0,s,Pokemon::Ice);
        int source = b.player(s);

        team(b,source)["MistCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), source, "Mist", &et);
    }

    static void et(int s, int, BS &b) {
        int source = b.player(s);
        if (team(b,source).value("MistCount") == 0) {
            return;
        }

        inc(team(b,source)["MistCount"], -1);
        int count = team(b,source)["MistCount"].toInt();
        if (count == 0) {
            b.sendMoveMessage(86,1,s,Pokemon::Ice);
            b.removeEndTurnEffect(BS::ZoneEffect, source, "Mist");
        }
    }
};

struct MMMoonlight : public MM
{
    MMMoonlight() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        int weather = b.weather;

        if (weather == BattleSituation::NormalWeather || !b.isWeatherWorking(weather)) {
            tmove(b,s).healing = 50;
        } else if (b.isWeatherWorking(BattleSituation::Sunny)) {
            if (b.gen().num == 2) {
                tmove(b,s).healing = 100;
            } else {
                tmove(b,s).healing = 66;
            }
        } else {
            tmove(b,s).healing = 25;
        }
    }
};

struct MMMudSport : public MM
{
    MMMudSport() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        int type = turn(b,s)["MudSport_Arg"].toInt();
        QString sport = "Sported" + QString::number(type);
        QString sportend = "SportedEnd" + QString::number(type);
        if ((b.battleMemory().contains(sport) && b.pokeMemory(b.battleMemory()[sport].toInt()).value(sport).toBool()) ||
                (b.battleMemory().contains(sportend) && b.battleMemory().value(sportend).toInt() > b.turn())) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int move = MM::move(b,s);
        b.sendMoveMessage(88, move == MudSport ? 0 : 1, s, type(b,s));
        int type = turn(b,s)["MudSport_Arg"].toInt();

        if (b.gen() <= 5) {
            QString sport = "Sported" + QString::number(type);
            poke(b,s)[sport] = true;
            b.battleMemory()[sport] = s;
        } else {
            QString sport = "SportedEnd" + QString::number(type);
            b.battleMemory()[sport] = b.turn() + 5;
        }
    }
};

struct MMNightMare : public MM
{
    MMNightMare() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 6) : makeBracket(9, 1) ;
    }

    static void uas(int, int t, BS &b) {
        b.sendMoveMessage(92, 0, t, Pokemon::Ghost);
        poke(b,t)["HavingNightmares"] = true;
        addFunction(poke(b,t),"AfterStatusChange", "NightMare", &asc);
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "NightMare", &et);
    }

    static void asc(int s, int, BS &b) {
        if (b.poke(s).status() != Pokemon::Asleep) {
            removeFunction(poke(b,s),"AfterStatusChange", "NightMare");
            b.removeEndTurnEffect(BS::PokeEffect, s, "NightMare");
        }
    }

    static void et(int s, int, BS &b) {
        if (!b.koed(s) && b.poke(s).status() == Pokemon::Asleep && !b.hasWorkingAbility(s, Ability::MagicGuard)) {
            b.sendMoveMessage(92,1,s,Pokemon::Ghost);
            b.inflictPercentDamage(s, 25, s, false);
        }
    }
};

struct MMPresent : public MM
{
    MMPresent() {
        functions["BeforeTargetList"] = &btl;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void btl(int s, int, BS &b) {
        tmove(b, s).power = tmove(b, s).power * 40 * b.randint(4);
        if (tmove(b, s).power == 0) {
            tmove(b, s).power = 1;
        }
    }

    static void cad(int s, int t, BS &b) {
        if (b.gen() >= 5) {
            b.sendMoveMessage(96,0,s,type(b,s),t);
            b.healLife(t, b.poke(t).totalLifePoints()/4);
        }
        else {
            b.sendMoveMessage(96,0,s,type(b,s),t, 80);
            b.healLife(t, 80);
        }
    }
};

struct MMPsychup : public MM
{
    MMPsychup() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas (int s, int t, BS &b ) {
        b.sendMoveMessage(97,0,s,type(b,s),t);
        for (int i = 1; i <= 7; i++) {
            fpoke(b,s).boosts[i] = fpoke(b,t).boosts[i];
        }
    }
};

struct MMPsychoShift : public MM
{
    MMPsychoShift() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.poke(s).status() == Pokemon::Fine || b.poke(t).status() != Pokemon::Fine || !b.canGetStatus(t, b.poke(s).status()))
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(98,0,s,type(b,s),t);
        b.inflictStatus(t, b.poke(s).status(), s);
        b.healStatus(s, b.poke(s).status());
    }
};

struct MMPsywave : public MM
{
    MMPsywave() {
        functions["CustomAttackingDamage"] = &cad;
    }

    static void cad (int s, int, BS &b) {
        turn(b,s)["CustomDamage"] = fpoke(b,s).level * (5 + b.randint(11)) / 10;
    }
};

struct MMRazorWind : public MM
{
    MMRazorWind() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (!poke(b,s).contains("ReleaseTurn") || poke(b,s)["ReleaseTurn"].toInt() != b.turn()) {
            int mv = move(b,s);
            if (mv == SolarBeam && b.isWeatherWorking(BS::Sunny))
                return;

            b.sendMoveMessage(104, turn(b,s)["RazorWind_Arg"].toInt(), s, type(b,s), s, move(b,s));
            /* Skull bash */
            if (b.gen() > 1 && mv == SkullBash) {
                b.inflictStatMod(s,Defense,1, s);
            }

            if (b.hasWorkingItem(s, Item::PowerHerb)) {
                //Power Herb
                b.sendItemMessage(11,s);
                b.disposeItem(s);

                if (mv == SolarBeam && b.weather != BS::NormalWeather && b.weather != BS::Sunny && b.isWeatherWorking(b.weather)) {
                    tmove(b, s).power = tmove(b, s).power / 2;
                }
            } else {
                poke(b,s)["ChargingMove"] = mv;
                poke(b,s)["ReleaseTurn"] = b.turn() + 1;
                turn(b,s)["TellPlayers"] = false;
                tmove(b, s).power = 0;
                tmove(b, s).statAffected = 0;
                tmove(b, s).status = Pokemon::Fine;
                tmove(b, s).targets = Move::User;
                addFunction(poke(b,s), "TurnSettings", "RazorWind", &ts);
            }
        }
    }

    static void ts(int s, int, BS &b) {
        removeFunction(poke(b,s), "TurnSettings", "RazorWind");
        fturn(b,s).add(TM::NoChoice);
        int mv = poke(b,s)["ChargingMove"].toInt();
        MoveEffect::setup(mv,s,s,b);
        if (mv == SolarBeam && b.weather != BS::NormalWeather && b.weather != BS::Sunny && b.isWeatherWorking(b.weather)) {
            tmove(b, s).power = tmove(b, s).power / 2;
        }
    }
};

struct MMPunishment : public MM
{
    MMPunishment() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        int boostsum = 0;

        for (int i = 1; i <= 7; i++) {
            int temp = fpoke(b,t).boosts[i];
            if (temp > 0) {
                boostsum += temp;
            }
        }

        tmove(b, s).power = tmove(b, s).power * std::min(60 + 20 * boostsum, 200);
    }
};

struct MMRage : public MM
{
    MMRage() {
        functions["OnSetup"] = &os;
        functions["MoveSettings"] = &ms;
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
        functions["AttackSomehowFailed"] = &asf;
    }

    static void os(int s, int, BS &b) {
        if (b.gen() != 2)
            return;

        if (poke(b,s).value("AnyLastMoveUsed") != Move::Rage)
            poke(b,s)["RagePower"] = 0;
    }

    static void ms(int s, int, BS &b) {
        if (b.gen().num == 2) {
            tmove(b,s).power *= 1 + poke(b,s).value("RagePower").toInt();
        }
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "UponOffensiveDamageReceived", "Rage", &uodr);

        if (poke(b,s).contains("RageBuilt") && poke(b,s)["AnyLastMoveUsed"] == Move::Rage) {
            poke(b,s).remove("AttractBy");
            b.healConfused(s);
            poke(b,s).remove("Tormented");
        }
        poke(b,s).remove("RageBuilt");

        // In Gen 1 we are locked into Rage
        if (b.gen().num == 1) {
            addFunction(poke(b,s), "TurnSettings", "Rage", &ts);
            poke(b,s)["RageMissed"] = false;
        }

    }

    static void uodr(int s, int, BS &b) {
        if (!b.koed(s) && poke(b,s)["AnyLastMoveUsed"] == Move::Rage) {
            poke(b,s)["RageBuilt"] = true;
            if (b.gen() != 2) {
                if (!b.hasMaximalStatMod(s, Attack)) {
                    b.inflictStatMod(s, Attack, 1,false);
                    b.sendMoveMessage(102, 0, s);
                }
            } else {
                if (poke(b,s).value("RagePower").toInt() < 5) {
                    inc(poke(b,s)["RagePower"], 1);
                    b.sendMoveMessage(102, 0, s);
                }
            }
        }
    }

    static void ts(int s, int, BS &b) {
        fturn(b,s).add(TM::NoChoice);
        MoveEffect::setup(Move::Rage,s,s,b);
    }

    static void daf(int s, int, BS &b) {
        if (b.gen().num == 1 && poke(b,s)["RageMissed"].toBool()) {
            if (b.coinflip(1,256)) {
                tmove(b, s).accuracy = 0;
            } else {
                fturn(b,s).add(TM::Failed);
            }
        }
    }

    static void asf(int s, int, BS &b) {
        if (b.gen().num == 1) {
            poke(b,s)["RageMissed"] = true;
        }
    }

};

struct MMSafeGuard : public MM
{
    MMSafeGuard() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 2 ? makeBracket(7, 2) : gen <= 4 ? makeBracket(1, 3) : makeBracket(21, 2);
    }

    static void daf(int s, int, BS &b) {
        int source = b.player(s);
        if (team(b,source).value("SafeGuardCount").toInt() > 0)
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int, BS &b) {
        int source = b.player(s);
        b.sendMoveMessage(109,0,s,type(b,s));
        team(b,source)["SafeGuardCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), source, "SafeGuard", &et);
    }

    static void et(int s, int, BS &b) {
        int source = b.player(s);
        if (team(b,source).value("SafeGuardCount") == 0) {
            return;
        }

        inc(team(b,source)["SafeGuardCount"], -1);
        int count = team(b,source)["SafeGuardCount"].toInt();
        if (count == 0) {
            b.sendMoveMessage(109,1,s,Pokemon::Psychic);
            b.removeEndTurnEffect(BS::ZoneEffect, source, "SafeGuard");
        }
    }
};

struct MMSketch : public MM
{
    MMSketch() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        int move = poke(b,t)["LastMoveUsed"].toInt();
        /* Struggle, chatter */
        if (b.koed(t) || move == Struggle || move == Chatter || move == Sketch || move == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        int mv = poke(b,t)["LastMoveUsed"].toInt();
        b.sendMoveMessage(111,0,s,type(b,s),t,mv);
        int slot = fpoke(b,s).lastMoveSlot;
        b.changeDefMove(s, slot, mv);

    }
};

struct MMSleepingUser : public MM
{
    MMSleepingUser() {
        functions["EvenWhenCantMove"] = &ewcm;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void ewcm(int s, int, BS &b) {
        turn(b,s)["SleepingMove"] = true;
    }

    static void daf(int s, int, BS &b) {
        if (b.poke(s).status() != Pokemon::Asleep) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMSleepTalk : public MM
{
    MMSleepTalk() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    struct FM : public QSet<int> {
        FM() {
            //Sleep Talk prevents all 2-Turn moves
            //Fixme: Gen 4 and below Mimic should have the move replaced, not Sleep Talk
            (*this) << NoMove << Bide << Bounce << Chatter << Copycat << Dig << Dive << Fly
                              << FocusPunch << MeFirst << Metronome << MirrorMove << ShadowForce
                              << SkullBash << SkyAttack << SleepTalk << SolarBeam << Struggle << RazorWind
                              << Uproar << IceBurn << FreezeShock << Geomancy << PhantomForce << Belch;
        }
        bool contains(int move, Pokemon::gen gen) const {
            //Nature Power ruining things again, Sketch and Mimic are forbidden at least in Gen 5.
            if (move == NaturePower || move == Sketch || move == Mimic) {
                return gen >= 5;
            } else {
                return QSet<int>::contains(move);
            }
        }
    };

    static FM forbidden_moves;

    static void daf(int s, int, BS &b) {
        b.callpeffects(s, s, "MovesPossible");
        QList<int> mp;

        for (int i = 0; i < 4; i++) {
            /* Sleep talk can work on 0 PP moves but not on disabled moves*/
            /* On gen 5 it can work several times behind a choice band, so I allowed disabled moves, as
               choice band blocks moves the same way, but it needs to be cross checked. */
            if ( (b.gen() >= 5 || turn(b, s).value("Move" + QString::number(i) + "Blocked").toBool() == false)
                 && !forbidden_moves.contains(b.move(s,i), b.gen())) {
                mp.push_back(i);
            }
        }

        if (mp.size() == 0) {
            fturn(b,s).add(TM::Failed);
        } else {
            turn(b,s)["SleepTalkedMove"] = b.move(s, mp[b.randint(mp.size())]);
        }
    }

    static void uas(int s, int, BS &b) {
        removeFunction(turn(b,s), "DetermineAttackFailure", "SleepTalk");
        removeFunction(turn(b,s), "UponAttackSuccessful", "SleepTalk");
        int mv = turn(b,s)["SleepTalkedMove"].toInt();
        BS::BasicMoveInfo info = tmove(b,s);
        MoveEffect::unsetup(Move::SleepTalk, s, b);
        MoveEffect::setup(mv,s,s,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s, mv, true);
        MoveEffect::unsetup(mv,s,b);
        MoveEffect::setup(Move::SleepTalk, s, s, b);
        tmove(b,s) = info;
    }
};

//Declaring static class variable
MMSleepTalk::FM MMSleepTalk::forbidden_moves;

struct MMSmellingSalt : public MM
{
    MMSmellingSalt () {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["AfterAttackSuccessful"] = &aas;
    }

    static void bcd(int s, int t, BS &b) {
        if (b.hasSubstitute(t) && tmove(b,s).attack != Move::Hex)
            return;

        int st = turn(b,s)["SmellingSalt_Arg"].toInt();
        if ( (st == 0 && b.poke(t).status() != Pokemon::Fine) || (st != 0 && b.poke(t).status() == st)) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }

    static void aas(int s, int t, BS &b) {
        if (!b.koed(t)) {
            int status = turn(b,s)["SmellingSalt_Arg"].toInt();

            /* Venom Shock doesn't heal, as well as Evil Eye */
            if (status != Pokemon::Poisoned && status != 0)
                b.healStatus(t, status);
        }
    }
};

struct MMSnatch : public MM
{
    MMSnatch() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "Snatch", &dgaf);
        b.battleMemory()["Snatcher"] = s;
        turn(b,s)["Snatcher"] = true;
        b.sendMoveMessage(118,1,s,type(b,s));
    }

    static void dgaf(int s, int , BS &b) {
        if (b.battleMemory().contains("Snatcher")) {
            int snatcher = b.battleMemory()["Snatcher"].toInt();
            if (!turn(b,snatcher).value("Snatcher").toBool()) {
                return;
            }

            int move = MM::move(b,s);
            /* Typically, the moves that are snatched are moves that only induce status / boost mods and nothing else,
                therefore having no "SpecialEffect". Exceptions are stored in snatched_moves */
            bool snatched = tmove(b,s).flags & Move::SnatchableFlag;

            if (snatched) {
                b.fail(s,118,0,type(b,snatcher), snatcher);
                /* Now Snatching ... */
                removeFunction(turn(b,snatcher), "UponAttackSuccessful", "Snatch");
                turn(b,snatcher).remove("Snatcher");
                b.battleMemory().remove("Snatcher");
                MoveEffect::setup(move,snatcher,s,b);
                b.useAttack(snatcher,move,true);
                MoveEffect::unsetup(move,snatcher,b);
            }
        }
    }
};

struct MMSpite : public MM
{
    MMSpite(){
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf (int s, int t, BS &b) {
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
        if (tu + 1 < b.turn() || (tu + 1 == b.turn() && fturn(b,t).contains(TM::HasMoved))) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        int slot = fpoke(b,t).lastMoveSlot;
        if (b.PP(t,slot) == 0) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }
    static void uas(int s, int t, BS &b)
    {
        int slot = fpoke(b,t).lastMoveSlot;
        int pploss = 4;

        if (b.gen().num == 3)
            pploss = 2 + b.randint(4);
        else if (b.gen().num == 2)
            pploss = 1 + b.randint(5);

        b.losePP(t, slot, pploss);
        b.callieffects(t,t,"AfterPPLoss");
        b.sendMoveMessage(123,0,s,Pokemon::Ghost,t,b.move(t,slot),QString::number(pploss));
    }
};

struct MMSplash : public MM
{
    MMSplash(){
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int , BS & b) {
        b.sendMoveMessage(82,0,s);
    }
};

struct MMSuckerPunch : public MM
{
    MMSuckerPunch(){
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (fturn(b,t).contains(TM::HasMoved) || tmove(b, t).power == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMTailWind : public MM {
    MMTailWind(){
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(1, 4) : makeBracket(21, 4) ;
    }

    static void daf(int s, int , BS &b) {
        if (team(b,b.player(s)).contains("TailWindCount"))
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int, BS &b){
        b.sendMoveMessage(133,0,s,Pokemon::Flying);
        int source = b.player(s);
        team(b,source)["TailWindCount"] = b.gen() <= 4 ? 3 : 4;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), source, "TailWind", &et);
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["TailWindCount"], -1);
        if (team(b,s)["TailWindCount"].toInt() == 0) {
            b.removeEndTurnEffect(BS::ZoneEffect, s, "TailWind");
            team(b,s).remove("TailWindCount");
            b.sendMoveMessage(133,1,s,Pokemon::Flying);
        }
    }
};

struct MMTorment : public MM {
    MMTorment() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (poke(b,t).value("Tormented").toBool())
            fturn(b,s).add(TM::Failed);
    }

    static void uas (int s, int t, BS &b) {
        if (b.hasWorkingTeamAbility(t, Ability::AromaVeil)) {
            b.sendAbMessage(112,1,t);
            return;
        }
        b.sendMoveMessage(135,0,s,Pokemon::Dark,t);
        if (b.gen() >= 5 && b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
            b.sendItemMessage(7,t);
            b.disposeItem(t);
        } else {
            poke(b,t)["Tormented"] = true;
            addFunction(poke(b,t), "MovesPossible", "Torment", &msp);
        }
    }

    static void msp(int s, int, BS &b) {
        if (!poke(b,s).contains("Tormented") || poke(b,s).value("AnyLastMoveUsed") == Move::Struggle)
            return;
        for (int i = 0; i < 4; i++) {
            if (b.move(s,i) == poke(b,s)["LastMoveUsed"].toInt()) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }
};

struct MMTrickRoom : public MM {
    MMTrickRoom() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(9, 0) : makeBracket(23, 0) ;
    }

    static void uas(int s, int, BS &b) {
        if (b.battleMemory().value("TrickRoomCount").toInt() > 0) {
            b.sendMoveMessage(138,1,s,Pokemon::Psychic);
            b.battleMemory().remove("TrickRoomCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "TrickRoom");
        } else {
            b.sendMoveMessage(138,0,s,Pokemon::Psychic);
            b.battleMemory()["TrickRoomCount"] = 5;
            b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "TrickRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battleMemory()["TrickRoomCount"], -1);
        if (b.battleMemory()["TrickRoomCount"].toInt() == 0) {
            b.sendMoveMessage(138,1,s,Pokemon::Psychic);
            b.battleMemory().remove("TrickRoomCount");
        }
    }
};

struct MMTripleKick : public MM {
    MMTripleKick() {
        functions["BeforeCalculatingDamage"] = &bh;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void bh(int s, int t, BS &b) {
        int count = 1;
        if (b.testAccuracy(s, t, true)) {
            count += 1;
            if (b.testAccuracy(s, t, true)) {
                count += 1;
            }
        }
        turn(b,s)["RepeatCount"] = count;
    }

    static void uas(int s, int, BS &b) {
        inc(turn(b,s)["TripleKickCount"], 1);
        int tkc = turn(b,s)["TripleKickCount"].toInt();
        if (tkc == 1) {
            tmove(b, s).power = tmove(b, s).power * 2;
        } else if (tkc==2) {
            tmove(b, s).power = tmove(b, s).power * 3/2;
        }
    }
};

struct MMWorrySeed : public MM {
    MMWorrySeed() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Truant & multi-type */
        if (b.ability(t) == Ability::Truant || b.ability(t) == Ability::Multitype || b.ability(t) == Ability::StanceChange) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        int ab = turn(b,s)["WorrySeed_Arg"].toInt();
        b.loseAbility(t);
        b.acquireAbility(t, ab); //Insomnia
        b.sendMoveMessage(143,0,s,type(b,s),t,ab);
    }
};

struct MMYawn : public MM {
    MMYawn() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        int opp = b.player(t);

        if (b.poke(t).status() != Pokemon::Fine || team(b,opp).value("SafeGuardCount").toInt() > 0 || poke(b,t).value("YawnCount").toInt() > 0
                || (b.terrainCount > 0 && (std::abs(b.terrain) == Type::Electric || std::abs(b.terrain) == Type::Fairy) && !b.isFlying(t))) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        if (b.hasWorkingAbility(t, Ability::Insomnia) || b.hasWorkingAbility(t, Ability::VitalSpirit)) {
            b.fail(s, 144, 2, 0, t);
        }
        if (b.sleepClause() && b.currentForcedSleepPoke[b.player(t)] != -1) {
            b.notifyClause(ChallengeInfo::SleepClause);
            fturn(b,s).add(TM::Failed);
        }
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 18) : makeBracket(19, 0) ;
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(144,0,s,Pokemon::Normal,t);
        poke(b,t)["YawnCount"] = 2;
        b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Yawn", &et);
    }

    static void et(int s, int, BS &b) {
        inc(poke(b,s)["YawnCount"], -1);
        int count = poke(b,s)["YawnCount"].toInt();
        if (count != 0) {

        } else {
            if (b.poke(s).status() == Pokemon::Fine && b.canGetStatus(s, Pokemon::Asleep)) {
                if (b.sleepClause() && b.currentForcedSleepPoke[b.player(s)] != -1) {
                    b.notifyClause(ChallengeInfo::SleepClause);
                } else {
                    b.inflictStatus(s, Pokemon::Asleep, s);
                    if (b.sleepClause() && b.poke(s).status() == Pokemon::Asleep) {
                        b.currentForcedSleepPoke[b.player(s)] = b.currentInternalId(s);
                    }
                }
            }
            b.removeEndTurnEffect(BS::PokeEffect, s, "Yawn");
            poke(b,s).remove("YawnCount");
        }
    }
};

struct MMCaptivate : public MM {
    MMCaptivate() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (!b.isSeductionPossible(s,t)) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMNaturePower : public MM
{
    MMNaturePower() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "NaturePower");

        int type = Type::Normal;
        if (b.gen().num == 5) {
            type = Type::Ground;
        }
        if (b.terrain != 0) {
            type = std::abs(b.terrain);
        }

        int move;
        if (type == Type::Ground) {
            move = Earthquake;
        } else if (type == Type::Water) {
            move = HydroPump;
        } else if (type == Type::Grass) {
            move = EnergyBall;
        } else if (type == Type::Fairy) {
            move = Moonblast;
        } else if (type == Type::Electric) {
            move = Thunderbolt;
        } else {
            if (b.gen().num == 3) {
                move = Swift;
            } else {
                move = TriAttack;
            }
        }

        MoveEffect::setup(move,s,s,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
        b.useAttack(s,move,true,true);
        MoveEffect::unsetup(move,s,b);
    }
};

struct MMRolePlay : public MM {
    MMRolePlay() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Wonder Guard & multi-type */
        if (b.ability(t) == Ability::WonderGuard || b.ability(t) == Ability::Multitype || b.ability(t) == Ability::Illusion || b.ability(t) == Ability::FlowerGift || b.ability(t) == Ability::Forecast ||  b.ability(t) == Ability::ZenMode ||  b.ability(t) == Ability::Trace ||  b.ability(t) == Ability::Imposter || b.ability(t) == Ability::StanceChange ) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(108,0,s,Pokemon::Psychic,t,b.ability(t));
        b.loseAbility(s);
        b.acquireAbility(s, b.ability(t));
    }
};

struct MMSkillSwap : public MM {
    MMSkillSwap() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Wonder Guard & multi-type */
        if (b.ability(t) == Ability::Multitype || b.ability(t) == Ability::WonderGuard || b.ability(s) == Ability::Multitype
                || b.ability(s) == Ability::WonderGuard || b.ability(s) == Ability::Illusion || b.ability(t) == Ability::Illusion
                || b.ability(s) == Ability::StanceChange || b.ability(t) == Ability::StanceChange) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        int tab = b.ability(t);
        int sab = b.ability(s);

        b.sendMoveMessage(112,0,s,Pokemon::Psychic,t);

        b.loseAbility(s);
        b.acquireAbility(s, tab);
        b.loseAbility(t);
        b.acquireAbility(t, sab);

        if (b.gen() >= 5) {
            b.sendMoveMessage(143,0,s,0,t,sab);
            b.sendMoveMessage(143,0,t,0,s,tab);
        }
    }
};

struct MMSecretPower : public MM {
    MMSecretPower() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (b.gen().num == 5) {
                tmove(b,s).classification = Move::OffensiveStatChangingMove;
                tmove(b,s).rateOfStat = 30 << 16;
                tmove(b,s).statAffected = Accuracy << 16;
                tmove(b,s).boostOfStat = uchar(-1) << 16;
        } else {
            if (b.terrain != 0) {
                int type = std::abs(b.terrain);
                if (type == Type::Grass) {
                    tmove(b,s).classification = Move::OffensiveStatusInducingMove;
                    tmove(b,s).status = Pokemon::Asleep;
                    tmove(b,s).rate = 30;
                } else if (type == Type::Electric) {
                    tmove(b,s).classification = Move::OffensiveStatusInducingMove;
                    tmove(b,s).status = Pokemon::Paralysed;
                    tmove(b,s).rate = 30;
                } else if (type == Type::Fairy) {
                    tmove(b,s).classification = Move::OffensiveStatChangingMove;
                    tmove(b,s).rateOfStat = 30 << 16;
                    tmove(b,s).statAffected = SpAttack << 16;
                    tmove(b,s).boostOfStat = uchar(-1) << 16;
                }
            } else {
                tmove(b,s).classification = Move::OffensiveStatusInducingMove;
                tmove(b,s).status = Pokemon::Paralysed;
                tmove(b,s).rate = 30;
            }
        }
    }
};

struct MMOutrage : public MM
{
    MMOutrage() {
        functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(6, 11);
    }

    static void uas(int s, int, BS &b) {
        // Asleep is for Sleep Talk
        if ( (!turn(b,s)["OutrageBefore"].toBool() || poke(b,s).value("OutrageUntil").toInt() < b.turn())
             && b.poke(s).status() != Pokemon::Asleep) {
            poke(b,s)["OutrageUntil"] = b.turn() +  1 + b.randint(2);
            addFunction(poke(b,s), "TurnSettings", "Outrage", &ts);
            addFunction(poke(b,s), "MoveSettings", "Outrage", &ms);

            if (b.gen() <= 4) {
                b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "Outrage", &aas);
            }

            poke(b,s)["OutrageMove"] = move(b,s);
        }
        /* In gen 5, even a miss causes outrage to stop, so we update the outrage count only after the attack is successful */
        if (b.gen() >= 5)
            poke(b,s)["LastOutrage"] = b.turn();
    }

    static void aas(int s, int, BS &b) {
        if (poke(b,s).contains("OutrageUntil") && b.turn() == poke(b,s)["OutrageUntil"].toInt()) {
            removeFunction(poke(b,s), "TurnSettings", "Outrage");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Outrage");
            poke(b,s).remove("OutrageUntil");
            poke(b,s).remove("OutrageMove");
            poke(b,s).remove("LastOutrage");
            b.sendMoveMessage(93,0,s,type(b,s));
            b.inflictConfused(s, s, b.gen() >= 2);
        }
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s).value("OutrageUntil").toInt() >= b.turn() && poke(b,s)["LastOutrage"].toInt() == b.turn()-1) {
            fturn(b,s).add(TM::NoChoice);
            MoveEffect::setup(poke(b,s)["OutrageMove"].toInt(),s,s,b);

            if (b.gen() >= 5) {
                addFunction(turn(b, s), "AfterAttackFinished", "Outrage", &aas);
            }
        }
    }

    static void ms(int s, int, BS &b) {
        turn(b,s)["OutrageBefore"] = poke(b,s).contains("LastOutrage") && poke(b,s)["LastOutrage"].toInt() == b.turn() - 1;
        if (b.gen() <= 4)
            poke(b,s)["LastOutrage"] = b.turn();
    }
};

struct MMUproar : public MM {
    MMUproar() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen gen) {
        return gen <= 4 ? makeBracket(6, 11) : makeBracket(26, 0) ;
    }

    static void uas(int s,int, BS &b) {
        if (poke(b,s).value("UproarUntil").toInt() < b.turn() || !turn(b,s).value("UproarBefore").toBool()) {
            if (b.gen() <= 4)
                poke(b,s)["UproarUntil"] = b.turn() + 1 + b.randint(4);
            else
                poke(b,s)["UproarUntil"] = b.turn() + 2;

            b.sendMoveMessage(141,0,s);
            foreach (int i, b.sortedBySpeed()) {
                if (b.poke(i).status() == Pokemon::Asleep) {
                    b.sendMoveMessage(141,3,i);
                    b.changeStatus(i, Pokemon::Normal);
                }
            }
            b.addUproarer(s);
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), s, "Uproar", &et);
            addFunction(poke(b,s), "TurnSettings", "Uproar", &ts);
            poke(b,s)["UproarMove"] = move(b,s);
        }
        poke(b,s)["LastUproar"] = b.turn();
    }


    static void ts(int s, int, BS &b) {
        fturn(b,s).add(TM::NoChoice);
        turn(b,s)["UproarBefore"] = true;
        MoveEffect::setup(poke(b,s)["UproarMove"].toInt(),s,s,b);
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;
        if (poke(b,s).value("UproarUntil").toInt() > b.turn() && poke(b,s).value("LastUproar").toInt() == b.turn()) {
            b.sendMoveMessage(141,1,s);

            foreach (int i, b.sortedBySpeed()) {
                if (b.poke(i).status() == Pokemon::Asleep) {
                    b.sendMoveMessage(141,3,i);
                    b.changeStatus(i, Pokemon::Normal);
                }
            }
        } else {
            removeFunction(poke(b,s), "TurnSettings", "Uproar");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Uproar");
            poke(b,s).remove("UproarUntil");
            poke(b,s).remove("LastUproar");
            b.removeUproarer(s);
            b.sendMoveMessage(141,2,s,type(b,s));
        }
    }
};

struct MMStockPile : public MM
{
    MMStockPile() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).value("StockPileCount").toInt() >= 3) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        inc(poke(b,s)["StockPileCount"], 1);
        b.sendMoveMessage(125,0,s,0,s,poke(b,s)["StockPileCount"].toInt());
        if (b.gen() >= 4) {
            if (!b.hasMaximalStatMod(s, Defense)) {
                inc(poke(b,s)["StockPileDef"],1);
                b.inflictStatMod(s, Defense, 1, s);
            }
            if (!b.hasMaximalStatMod(s, SpDefense)) {
                inc(poke(b,s)["StockPileSDef"], 1);
                b.inflictStatMod(s, SpDefense, 1, s);
            }
        }
    }
};

struct MMSwagger : public MM {
    MMSwagger() {
        functions["DetermineAttackFailure"] = &daf;
    }
    /* Swagger fails against max attack foes in statium 2 */
    static void daf(int s, int t, BS &b) {
        if (b.gen() <= 2 && (b.getStat(t, Attack) == 999 || b.hasMaximalStatMod(t, Attack))) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMSwallow: public MM
{
    MMSwallow() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).value("StockPileCount").toInt() == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        if (b.gen() >= 4) {
            b.changeStatMod(s,Defense,fpoke(b,s).boosts[Defense] - poke(b,s)["StockPileDef"].toInt());
            b.changeStatMod(s,SpDefense,fpoke(b,s).boosts[SpDefense] - poke(b,s)["StockPileSDef"].toInt());
            poke(b,s).remove("StockPileDef");
            poke(b,s).remove("StockPileSDef");
        }
        switch (poke(b,s)["StockPileCount"].toInt()) {
        case 1: tmove(b,s).healing = 25; break;
        case 2: tmove(b,s).healing = 50; break;
        case 3: default: tmove(b,s).healing = 100; break;
        }
        poke(b,s).remove("StockPileCount");
        b.sendMoveMessage(131,0,s);
    }
};

struct MMSpitUp : public MM
{
    MMSpitUp() {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = & uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).value("StockPileCount").toInt() == 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void bcd(int s, int, BS &b) {
        tmove(b, s).power = tmove(b, s).power * poke(b,s)["StockPileCount"].toInt() * 100;
    }

    static void uas(int s, int, BS &b) {
        if (b.gen() >= 4) {
            b.changeStatMod(s,Defense,fpoke(b,s).boosts[Defense] - poke(b,s)["StockPileDef"].toInt());
            b.changeStatMod(s,SpDefense,fpoke(b,s).boosts[SpDefense] - poke(b,s)["StockPileSDef"].toInt());
            poke(b,s).remove("StockPileDef");
            poke(b,s).remove("StockPileSDef");
        }
        poke(b,s).remove("StockPileCount");
        b.sendMoveMessage(122,0,s);
    }
};

struct MMNaturalGift :  public MM
{
    MMNaturalGift() {
        functions["DetermineAttackFailure"] = &daf;
        functions["BeforeTargetList"] = &btl;
    }

    static void daf(int s, int, BS &b) {
        if (!turn(b,s).value("NaturalGiftOk").toBool()) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void btl(int s, int, BS &b) {
        int berry = b.poke(s).item();

        if (!b.hasWorkingItem(s, berry) || !ItemInfo::isBerry(berry)) {
            return;
        }

        //False is used to prevent Cheek Pouch and Belch from seeing that a berry was eaten but still allow Harvest to work
        b.eatBerry(s, false);
        //Natural gift gets a 20 BP boost in Gen 6
        tmove(b, s).power = tmove(b, s).power * (ItemInfo::BerryPower(berry) + (b.gen() >= 6 ? 20 : 0));
        tmove(b,s).type = ItemInfo::BerryType(berry);
        turn(b,s)["NaturalGiftOk"] = true;
    }
};

struct MMRecycle : public MM {
    MMRecycle() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf (int s, int, BS &b) {
        if (b.poke(s).itemUsed() == 0 || b.poke(s).item() != 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas (int s, int, BS &b) {
        int item = b.poke(s).itemUsed();
        b.sendMoveMessage(105,0,s,0,s,item);
        b.poke(s).itemUsed() = 0;
        b.acqItem(s, item);
    }
};

struct MMTransform : public MM {
    MMTransform() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if ( fpoke(b,t).flags & BS::BasicPokeInfo::Transformed || (b.hasWorkingAbility(t, Ability::Illusion) && poke(b,t).contains("IllusionTarget"))) {
            fturn(b,s).add(TM::Failed);
            return;
        }
    }

    static void uas(int s, int t, BS &b) {
        /* Give new values to what needed */
        Pokemon::uniqueId num = b.pokenum(t);

        if (b.gen() <= 4) {
            if (num.toPokeRef() == Pokemon::Giratina_O && b.poke(s).item() != Item::GriseousOrb)
                num = Pokemon::Giratina;
            if (PokemonInfo::OriginalForme(num) == Pokemon::Arceus) {
                num.subnum = ItemInfo::PlateType(b.poke(s).item());
            }
            if (PokemonInfo::OriginalForme(num) == Pokemon::Genesect)
                num.subnum = ItemInfo::DriveForme(b.poke(s).item());
        }

        b.sendMoveMessage(137,0,s,0,s,num.pokenum);

        BS::BasicPokeInfo &po = fpoke(b,s);
        BS::BasicPokeInfo &pt = fpoke(b,t);

        po.id = num;
        po.weight = PokemonInfo::Weight(num);
        //For Type changing moves
        po.types = QVector<int> () << b.getTypes(t);
        //po.type1 = PokemonInfo::Type1(num, b.gen());
        //po.type2 = PokemonInfo::Type2(num, b.gen());
        //po.types = QVector<int>() << po.type1 << po.type2;


        b.changeSprite(s, num);

        for (int i = 0; i < 4; i++) {
            b.changeTempMove(s,i,b.move(t,i));
        }

        for (int i = 1; i < 6; i++)
            po.stats[i] = pt.stats[i];

        //        for (int i = 0; i < 6; i++) {
        //            po.dvs[i] = pt.dvs[i];
        //        }

        for (int i = 0; i < 8; i++) {
            po.boosts[i] = pt.boosts[i];
        }

        if (b.gen() >= 3) {
            b.loseAbility(s);
            b.acquireAbility(s, b.ability(t));
        }

        fpoke(b,s).flags |= BS::BasicPokeInfo::Transformed;
    }
};

struct MMPayback : public MM
{
    MMPayback() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        //Attack / Switch --> power *= 2
        //In gen 5, switch doesn't increase the power
        if ( (b.gen() <= 4 && b.hasMoved(t)) || (b.gen() >= 5 && fturn(b,t).contains(TM::HasMoved))) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMAcupressure : public MM
{
    MMAcupressure() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasSubstitute(t) && (t!=s || b.gen() <= 4)) {
            b.failSilently(s);
            b.sendMoveMessage(128, 2, t, 0, s, Move::Acupressure);
        }
        for (int i = Attack; i <= Evasion; i++) {
            if (fpoke(b,t).boosts[i] < 6) {
                return;
            }
        }
        fturn(b,s).add(TM::Failed);
    }

    static void uas(int , int t, BS &b) {
        QVector<int> stats;
        for (int i = Attack; i <= Evasion; i++) {
            if (fpoke(b,t).boosts[i] < 6) {
                stats.push_back(i);
            }
        }
        if (stats.empty())
            return;
        b.inflictStatMod(t, stats[b.randint(stats.size())], 2, t);
    }
};

struct MMHelpingHand : public MM
{
    MMHelpingHand() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int p, BS &b) {
        b.sendMoveMessage(63, 0, s, type(b,s), p);
        turn(b,p)["HelpingHanded"] = true;
    }
};

struct MMGuardSplit : public MM
{
    MMGuardSplit() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        QStringList stats = turn(b,s)["GuardSplit_Arg"].toString().split('_');

        b.sendMoveMessage(155, move(b,s) == Move::PowerSplit ? 0 : 1, s, type(b,s), t);
        foreach(QString statS, stats) {
            int stat = statS.toInt();
            int avstat = (fpoke(b, s).stats[stat] + fpoke(b, t).stats[stat]) / 2;
            fpoke(b,s).stats[stat] = avstat;
            fpoke(b,t).stats[stat] = avstat;
        }
    }
};

struct MMMagicRoom : public MM {
    MMMagicRoom() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(25, 0) ;
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        if (b.battleMemory().value("MagicRoomCount").toInt() > 0) {
            b.sendMoveMessage(156,1,s,Pokemon::Psychic);
            b.battleMemory().remove("MagicRoomCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "MagicRoom");
            reactivate(b);
        } else {
            b.sendMoveMessage(156,0,s,Pokemon::Psychic);
            b.battleMemory()["MagicRoomCount"] = 5;
            b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "MagicRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battleMemory()["MagicRoomCount"], -1);
        if (b.battleMemory()["MagicRoomCount"].toInt() == 0) {
            b.sendMoveMessage(156,1,s,Pokemon::Psychic);
            b.battleMemory().remove("MagicRoomCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "MagicRoom");
            reactivate(b);
        }
    }

    static void reactivate (BS &b) {
        foreach (int p, b.sortedBySpeed()) {
            b.callieffects(p, p, "UponReactivation");
        }
    }
};

struct MMSoak : public MM {
    MMSoak() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        //Soak works on Water Pokemon, ForestCurse/TrickOrTreat don't work on Grass/Ghost
        int type = turn(b,s)["Soak_Arg"].toInt();
        if (b.pokenum(t).pokenum == Pokemon::Arceus || (move(b,s) != Move::Soak && b.hasType(t,type)))
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int s, int t, BS &b) {
        int type = turn(b,s)["Soak_Arg"].toInt();
        if (move(b,s) == Move::Soak) {
            b.setType(t, type);
            b.sendMoveMessage(157, 0, t, type, t);
        } else {
            b.addType(t, type);
            b.sendMoveMessage(157, 1, t, type, t);
        }
    }
};

struct MMEntrainment : public MM {
    MMEntrainment() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.ability(t) == Ability::Multitype || b.ability(t) == Ability::Truant || b.ability(t) == Ability::StanceChange) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        b.loseAbility(t);
        b.acquireAbility(t, b.ability(s));
        b.sendMoveMessage(158,0,s,type(b,s),t,b.ability(s));
    }
};

struct MMShellSmash : public MM {
    MMShellSmash() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        /* So that white herbs restore both negative boosts,
           the boolean is introduced and item effect called later */
        b.applyingMoveStatMods = true;
        b.inflictStatMod(s, Defense, -1, s);
        b.inflictStatMod(s, SpDefense, -1, s);
        b.inflictStatMod(s, Attack, 2, s);
        b.inflictStatMod(s, SpAttack, 2, s);
        b.inflictStatMod(s, Speed, 2, s);
        b.applyingMoveStatMods = false;
        b.callieffects(s, s, "AfterStatChange");
    }
};

struct MMIncinerate : public MM {
    MMIncinerate() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        if (ItemInfo::isBerry(b.poke(t).item()) || (ItemInfo::isGem(b.poke(t).item()) && b.gen() >= 6)) {
            b.sendMoveMessage(160, 0, s, Type::Fire, t, b.poke(t).item());
            b.loseItem(t);
        }
    }
};

struct MMFinalGambit : public MM {
    MMFinalGambit() {
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        int hp = b.poke(s).lifePoints();
        b.selfKoer() = s;
        b.koPoke(s, s);
        b.inflictDamage(t, hp, s, true);
    }
};

struct MMBestow : public MM {
    MMBestow() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
        int i1 = b.poke(s).item();
        if (b.koed(t) || !b.canLoseItem(s,s) || b.poke(t).item() != 0
                || (b.ability(t) == Ability::Multitype && ItemInfo::isPlate(b.poke(s).item()))
                || (b.pokenum(t).pokenum == Pokemon::Giratina && b.poke(s).item() == Item::GriseousOrb)
                || (b.pokenum(t).pokenum == Pokemon::Genesect && ItemInfo::isDrive(b.poke(s).item()))
                || (!b.canPassMStone(t, i1)))
        {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b)
    {
        b.sendMoveMessage(162,0,s,type(b,s),t,b.poke(s).item());
        b.acqItem(t, b.poke(s).item());
        b.loseItem(s);
        b.symbiosisPass(s);
    }
};

struct MMRefresh : public MM {
    MMRefresh() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.poke(s).status() == Pokemon::Fine) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        b.healStatus(s, 0);
        b.sendMoveMessage(164, 0, s);
    }
};

struct MMMemento : public MM {
    MMMemento() {
        functions["BeforeTargetList"] = &btl;
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    /* In gen 3, fails if can't inflict negative boosts.
      In gen 4, doesn't fail (so the Ko is in MMFaintUser)
      In gen 5, fails if miss or substitute
    From upokecenter*/
    static void btl(int s, int, BS &b) {
        if (b.gen() > 3) {
            return;
        }
        if (b.targetList.size() > 0) {
            int t= b.targetList.front();

            if (!(b.hasMinimalStatMod(t, Attack) && b.hasMinimalStatMod(t, SpAttack))) {
                b.koPoke(s,s);
            }
        }
    }

    static void daf(int s, int t, BS &b) {
        if (b.gen() >= 5) {
            if (b.hasSubstitute(t)) {
                fturn(b,s).add(TM::Failed);
            } else {
                b.koPoke(s, s);
            }
        } else if (b.gen() <= 3) {
            if (b.hasMinimalStatMod(t, Attack) && b.hasMinimalStatMod(t, SpAttack)) {
                fturn(b,s).add(TM::Failed);
            }
        }
    }

    static void uas(int s, int t, BS &b) {
        /* In case of White Herb, apply the stat changes and then call the item effects afterwards */
        b.applyingMoveStatMods = true;
        b.inflictStatMod(t, Attack, -2, s);
        b.inflictStatMod(t, SpAttack, -2, s);
        b.applyingMoveStatMods = false;
        b.callieffects(t, t, "AfterStatChange");
    }
};

struct MMRelicSong : public MM
{
    MMRelicSong() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        if (!turn(b,s).contains("ChangedForm")) {
            if (fpoke(b,s).id == Pokemon::Meloetta)
                b.changePokeForme(s, Pokemon::Meloetta_P);
            else if (fpoke(b,s).id == Pokemon::Meloetta_P)
                b.changePokeForme(s, Pokemon::Meloetta);
            turn(b,s)["ChangedForm"] = true;
        }
    }
};

struct MMHeavySlam : public MM
{
    MMHeavySlam() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        int ratio = b.weight(s) / b.weight(t);

        int bp = 0;
        if (ratio >= 5) {
            bp = 120;
        } else if (ratio == 4) {
            bp = 100;
        } else if (ratio == 3) {
            bp = 80;
        } else if (ratio == 2) {
            bp = 60;
        } else {
            bp = 40;
        }

        tmove(b,s).power *= bp;
    }
};

struct MMWonderRoom : public MM {
    MMWonderRoom() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(24, 0) ;
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        if (b.battleMemory().value("WonderRoomCount").toInt() > 0) {
            b.sendMoveMessage(168,1,s,Pokemon::Psychic);
            b.battleMemory().remove("WonderRoomCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "WonderRoom");
        } else {
            b.sendMoveMessage(168,0,s,Pokemon::Psychic);
            b.battleMemory()["WonderRoomCount"] = 5;
            b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "WonderRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battleMemory()["WonderRoomCount"], -1);
        if (b.battleMemory()["WonderRoomCount"].toInt() <= 0) {
            b.sendMoveMessage(168,1,s,Pokemon::Psychic);
            b.battleMemory().remove("WonderRoomCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "WonderRoom");
        }
    }
};

struct MMReflectType : public MM
{
    MMReflectType() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(172,0,s,type(b,s),t);
        fpoke(b,s).type1 = fpoke(b,t).type1;
        fpoke(b,s).type2 = fpoke(b,t).type2;
        fpoke(b,s).types = fpoke(b,t).types;
    }
};

/** struct MMAcrobatics : public MM
{
    MMAcrobatics() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        if (b.poke(s).item() == 0) {
            tmove(b,s).power *= 2;
        }
    }
}; */

struct MMTelekinesis : public MM
{
    MMTelekinesis() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(16, 0) ;
    }

    static void daf(int s, int t, BS &b) {
        if (poke(b,t).contains("LevitatedCount")) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int t, BS &b) {
        if (poke(b,t).value("LevitatedCount").toInt() > 0) {
            b.sendMoveMessage(174, 1, t);
            poke(b,t).remove("LevitatedCount");
        } else {
            b.sendMoveMessage(174, 0, s, type(b,s), t);
            poke(b,t)["LevitatedCount"] = 3;
            b.addEndTurnEffect(BS::PokeEffect, bracket(b.gen()), t, "Telekinesis", &et);
        }
    }

    static void et(int s, int , BS &b) {
        inc(poke(b,s)["LevitatedCount"], -1);
        if (poke(b,s).value("LevitatedCount").toInt() == 0) {
            poke(b,s).remove("LevitatedCount");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Telekinesis");
            b.sendMoveMessage(174, 1, s);
        };
    }
};

struct MMAfterYou : public MM
{
    MMAfterYou() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasMoved(t))
            fturn(b,s).add(TM::Failed);
    }

    static void uas(int, int t, BS &b) {
        /* Possible crash cause... If t is in the wrong place in the list. If DetermineAttackFailure didn't do its job correctly. */
        b.makePokemonNext(t);
    }
};

struct MMQuash : public MM
{
    MMQuash() {
        functions["DetermineAttackFailure"] = &MMAfterYou::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int, int t, BS &b) {
        /* Possible crash cause... If t is in the wrong place in the list. If DetermineAttackFailure didn't do its job correctly. */
        b.makePokemonLast(t);
    }
};

struct MMFirePledge : public MM
{
    MMFirePledge() {
        functions["MoveSettings"] = &ms;
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (turn(b,i).contains("MadeAPledge") && turn(b,i)["MadeAPledge"] != Pokemon::Fire) {
                /* Here you go with the special effect */
                turn(b,s)["PledgeEffectActivater"] = i;//ref here
                return;
            }
        }
        /* No one made a combo for us, sad! Let's see if someone WILL make a combo for us */
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (!b.hasMoved(i) && (tmove(b,i).attack == Move::WaterPledge || tmove(b,i).attack == Move::GrassPledge) ){
                /* Here we make our pledge */
                turn(b,s)["MadeAPledge"] = Pokemon::Fire;
                b.sendMoveMessage(178, 0, s, Pokemon::Fire, 0, move(b,s));
                tmove(b,s).power = 0;
                tmove(b,s).targets = Move::User;
                turn(b,s)["TellPlayers"] = false;
                return;
            }
        }

        /* Otherwise it's just the standard pledge... */
    }

    static void bcd(int s, int, BS &b) {
        if (!turn(b,s).contains("PledgeEffectActivater"))
            return;

        //ref here
        int i = turn(b,s)["PledgeEffectActivater"].toInt();

        b.sendMoveMessage(178, 1, s, 0, i);
        turn(b,s)["AttackStat"] = b.getStat(s, SpAttack) + b.getStat(i, SpAttack, 1);
        tmove(b,s).power *= 2;
    }

    static void uas(int s, int t, BS &b);

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(5, 0) ;
    }

    static void makeABurningField(int t, BS &b) {
        if (team(b, t).value("BurningFieldCount").toInt() > 0)
            return;

        b.sendMoveMessage(178, 2, t, Pokemon::Fire);
        team(b,t)["BurningFieldCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), t, "FirePledge", &et);
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["BurningFieldCount"], -1);

        if (team(b,s).value("BurningFieldCount").toInt() <= 0) {
            team(b,s).remove("BurningFieldCount");
            b.removeEndTurnEffect(BS::ZoneEffect, s, "FirePledge");
            return;
        }

        std::vector<int> vect = b.sortedBySpeed();

        foreach(int t, vect) {
            if (b.player(t) != s && !b.koed(t))
                continue;
            b.sendMoveMessage(178, 3, t, Pokemon::Fire);
            b.inflictDamage(t, b.poke(t).totalLifePoints()/8, t);
        }
    }
};

struct MMGrassPledge : public MM
{
    MMGrassPledge() {
        functions["MoveSettings"] = &ms;
        functions["BeforeCalculatingDamage"] = &MMFirePledge::bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (turn(b,i).contains("MadeAPledge") && turn(b,i)["MadeAPledge"] != Pokemon::Grass) {
                /* Here you go with the special effect */
                turn(b,s)["PledgeEffectActivater"] = i;//ref here
                return;
            }
        }
        /* No one made a combo for us, sad! Let's see if someone WILL make a combo for us */
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (!b.hasMoved(i) && (tmove(b,i).attack == Move::FirePledge || tmove(b,i).attack == Move::WaterPledge) ){
                /* Here we make our pledge */
                turn(b,s)["MadeAPledge"] = Pokemon::Grass;
                b.sendMoveMessage(179, 0, s, Pokemon::Grass, 0, move(b,s));
                tmove(b,s).power = 0;
                tmove(b,s).targets = Move::User;
                turn(b,s)["TellPlayers"] = false;
                return;
            }
        }

        /* Otherwise it's just the standard pledge... */
    }

    static void uas(int s, int t, BS &b);

    static ::bracket bracket(Pokemon::gen) {
        return  makeBracket(21, 6) ;
    }

    static void makeASwamp(int t, BS &b) {
        if (team(b, t).value("SwampCount").toInt() > 0)
            return;

        b.sendMoveMessage(179, 2, t, Pokemon::Grass);
        team(b,t)["SwampCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), t, "GrassPledge", &et);
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["SwampCount"], -1);

        if (team(b,s).value("SwampCount").toInt() <= 0) {
            team(b,s).remove("SwampCount");
            b.removeEndTurnEffect(BS::ZoneEffect, s, "GrassPledge");
            return;
        }
    }
};

struct MMWaterPledge : public MM
{
    MMWaterPledge() {
        functions["MoveSettings"] = &ms;
        functions["BeforeCalculatingDamage"] = &MMFirePledge::bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (turn(b,i).contains("MadeAPledge") && turn(b,i)["MadeAPledge"] != Pokemon::Water) {
                /* Here you go with the special effect */
                turn(b,s)["PledgeEffectActivater"] = i;//ref here
                return;
            }
        }
        /* No one made a combo for us, sad! Let's see if someone WILL make a combo for us */
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.player(i) != b.player(s) || b.koed(i))
                continue;
            if (!b.hasMoved(i) && (tmove(b,i).attack == Move::FirePledge || tmove(b,i).attack == Move::GrassPledge) ){
                /* Here we make our pledge */
                turn(b,s)["MadeAPledge"] = Pokemon::Water;
                b.sendMoveMessage(180, 0, s, Pokemon::Water, 0, move(b,s));
                tmove(b,s).power = 0;
                tmove(b,s).targets = Move::User;
                turn(b,s)["TellPlayers"] = false;
                return;
            }
        }

        /* Otherwise it's just the standard pledge... */
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(21, 6) ;
    }

    static void makeARainbow(int t, BS &b) {
        if (team(b, t).value("RainbowCount").toInt() > 0)
            return;

        b.sendMoveMessage(180, 2, t, Pokemon::Water);
        team(b,t)["RainbowCount"] = 5;
        b.addEndTurnEffect(BS::ZoneEffect, bracket(b.gen()), t, "WaterPledge", &et);
    }

    static void uas(int s, int t, BS &b) {
        if (!turn(b,s).contains("PledgeEffectActivater"))
            return;

        //ref here
        int i = turn(b,s)["PledgeEffectActivater"].toInt();

        if (turn(b,i)["MadeAPledge"] == Pokemon::Fire) {
            makeARainbow(b.player(t), b);
        } else {
            MMGrassPledge::makeASwamp(b.player(t), b);
        }

        turn(b,i).remove("MadeAPledge");
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["RainbowCount"], -1);

        if (team(b,s).value("RainbowCount").toInt() <= 0) {
            team(b,s).remove("RainbowCount");
            b.removeEndTurnEffect(BS::ZoneEffect, s, "WaterPledge");
            return;
        }
    }
};

void MMFirePledge::uas(int s, int t, BS &b)
{
    if (!turn(b,s).contains("PledgeEffectActivater"))
        return;

    //ref here
    int i = turn(b,s)["PledgeEffectActivater"].toInt();

    if (turn(b,i)["MadeAPledge"] == Pokemon::Water) {
        MMWaterPledge::makeARainbow(b.player(t), b);
    } else {
        makeABurningField(b.player(t), b);
    }

    turn(b,i).remove("MadeAPledge");
}

void MMGrassPledge::uas(int s, int t, BS &b)
{
    if (!turn(b,s).contains("PledgeEffectActivater"))
        return;

    //ref here
    int i = turn(b,s)["PledgeEffectActivater"].toInt();

    if (turn(b,i)["MadeAPledge"] == Pokemon::Water) {
        makeASwamp(b.player(t), b);
    } else {
        MMFirePledge::makeABurningField(b.player(t), b);
    }

    turn(b,i).remove("MadeAPledge");
}

struct MMEchoedVoice : public MM
{
    MMEchoedVoice() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        int count = 1;
        if (b.battleMemory().contains("EchoVoiceTurn")) {
            if (b.battleMemory()["EchoVoiceTurn"].toInt() == b.turn() - 1) {
                count = b.battleMemory()["EchoVoiceCount"].toInt() + 1;
            } else if (b.battleMemory()["EchoVoiceTurn"].toInt() == b.turn()) {
                count = b.battleMemory()["EchoVoiceCount"].toInt();
            }
        }

        if (count > 5) {
            count = 5;
        }

        b.battleMemory()["EchoVoiceCount"] = count;
        b.battleMemory()["EchoVoiceTurn"] = b.turn();

        tmove(b,s).power *= count;
    }
};

struct MMElectroBall : public MM
{
    MMElectroBall() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        int ratio = b.getStat(s, Speed) / b.getStat(t, Speed);

        int bp = 0;
        if (ratio >= 5) {
            bp = 150;
        } else if (ratio == 4) {
            bp = 120;
        } else if (ratio == 3) {
            bp = 100;
        } else if (ratio == 2) {
            bp = 80;
        } else {
            bp = 60;
        }

        tmove(b,s).power *= bp;
    }
};

struct MMTechnoBlast : public MM
{
    MMTechnoBlast() {
        functions["MoveSettings"] = &ms;
    }

    static void ms (int s, int, BS &b) {
        int item = b.poke(s).item();
        if (!ItemInfo::isDrive(item) || turn(b,s).value("Symbiote").toBool())
            return;
        if (b.hasWorkingItem(s, item)) {
            tmove(b,s).type = poke(b,s)["ItemArg"].toInt();
        }
    }
};

struct MMRound : public MM
{
    MMRound() {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void bcd(int s, int, BS &b) {
        int source = b.player(s);

        if (!team(b,source).contains("CapellaTurn") || team(b,source)["CapellaTurn"].toInt() != b.turn()) {
            return;
        }

        tmove(b, s).power *= 2;
    }

    static void uas(int s, int, BS &b) {
        int source = b.player(s);

        team(b,source)["CapellaTurn"] = b.turn();

        for (int i = b.currentSlot + 1; i < signed(b.speedsVector.size()); i++) {
            int p = b.speedsVector[i];
            if (b.player(p) == b.player(s) && tmove(b,p).attack == Move::Round) {
                b.makePokemonNext(p);
                return;
            }
        }
    }
};

struct MMStoredPower : public  MM
{
    MMStoredPower() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        int boostsum = 0;

        for (int i = 1; i <= 7; i++) {
            int temp = fpoke(b,s).boosts[i];
            if (temp > 0) {
                boostsum += temp;
            }
        }

        tmove(b, s).power = tmove(b, s).power * (1 + boostsum);
    }
};

struct MMSynchronoise : public MM
{
    MMSynchronoise() {
        functions["BeforeCalculatingDamage"] = &btl;
    }

    static void btl(int s, int t, BS &b) {
        if (!b.getTypes(s).toList().toSet().intersect(b.getTypes(t).toList().toSet()).isEmpty()) {

        } else {
            fturn(b,s).typeMod = -100;
        }
    }
};

struct MMFoulPlay : public MM
{
    MMFoulPlay() {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["AfterAttackFinished"] = &aad;
    }

    static void bcd(int s, int t, BS &b) {
        turn(b,s)["CustomAttackStat"] = b.getBoostedStat(t, Attack);
    }

    static void aad(int s, int, BS &b) {
        turn(b,s).remove("CustomAttackStat");
    }
};

struct MMRetaliate : public MM
{
    MMRetaliate() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
        if (team(b, b.player(s)).contains("LastKoedTurn") && team(b, b.player(s))["LastKoedTurn"].toInt() == b.turn() - 1) {
            tmove(b,s).power *= 2;
        }
    }
};

struct MMFlameBurst : public MM
{
    MMFlameBurst() {
        functions["UponAttackSuccessful"] = &uas;
    }
    static void uas(int s, int t, BS &b) {
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (b.arePartners(i, t) && i!=t && b.areAdjacent(i, t) && !b.hasWorkingAbility(i, Ability::MagicGuard)) {
                b.inflictDamage(i, b.poke(i).totalLifePoints()/16, s, false);
            }
        }
    }
};

struct MMAllySwitch : public MM
{
    MMAllySwitch() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int , BS &b) {
        if (!b.multiples()) {
            fturn(b,s).add(TM::Failed);
            return;
        }
        if (b.slotNum(s) != 0 && b.slotNum(s) != b.numberPerSide()-1) {
            fturn(b,s).add(TM::Failed);
            return;
        }

        int t;

        if (b.slotNum(s) == 0) {
            t = b.slot(b.player(s), b.numberPerSide()-1);
        } else {
            t = b.slot(b.player(s), 0);
        }

        if (b.koed(t)) {
            fturn(b,s).add(TM::Failed);
            return;
        }

        turn(b,s)["SideChangeTarget"] = t;
    }

    static void uas (int s, int, BS &b) {
        int t = turn(b, s)["SideChangeTarget"].toInt();
        b.sendMoveMessage(190, 0, s, type(b, s), t);
        b.shiftSpots(s, t, true);
    }
};

struct MMGrowth : public MM
{
    MMGrowth() {
        functions["BeforeHitting"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        if (b.gen() >= 5 && b.isWeatherWorking(BS::Sunny))
            tmove(b,s).boostOfStat *= 2;
    }
};

struct MMFusionBolt : public MM
{
    MMFusionBolt() {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void bcd(int s, int, BS &b) {
        if (b.battleMemory().value("FusionFlare", -1) == b.turn())
            tmove(b,s).power *= 2;
    }

    static void uas(int, int, BS &b) {
        b.battleMemory()["FusionBolt"] = b.turn();
    }
};

struct MMFusionFlare : public MM
{
    MMFusionFlare() {
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void bcd(int s, int, BS &b) {
        if (b.battleMemory().value("FusionBolt", -1) == b.turn())
            tmove(b,s).power *= 2;
    }

    static void uas(int, int, BS &b) {
        b.battleMemory()["FusionFlare"] = b.turn();
    }
};

struct MMWillOWisp : public MM
{
    MMWillOWisp() {
        functions["BeforeTargetList"] = &btl;
        functions["DetermineAttackFailure"] = &daf;
    }

    static void btl(int s, int, BS &b) {
        if (b.targetList.size() <= 0 || b.gen() <= 4) {
            return;
        }
        /* Will o Wisp doesn't miss against flash fire in gen 5 */
        if (b.hasWorkingAbility(b.targetList.front(), Ability::FlashFire)) {
            tmove(b,s).accuracy = 0;
        }
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasWorkingAbility(t, Ability::FlashFire) && type(b,s) == Type::Fire) {
            b.failSilently(s);
            AMFlashFire::op(t, s, b);
        }
    }
};

struct MMAutotomize : public MM
{
    MMAutotomize() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        fpoke(b, s).weight = std::max(1, fpoke(b,s).weight-1000);
    }
};

struct MMCraftyShield: public MM
{
    MMCraftyShield() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        MMDetect::othersHaveMoved(s,s,b);
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "CraftyShield", &dgaf);
        team(b,b.player(s))["CraftyShieldUsed"] = b.turn();
        b.sendMoveMessage(199, 0, s, Pokemon::Fairy);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        int target = b.player(t);
        if (!team(b,target).contains("CraftyShieldUsed") || team(b,target)["CraftyShieldUsed"].toInt() != b.turn()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) && tmove(b,s).attack != Move::Feint) {
            return;
        }

        /* Blocks status moves */
        if (tmove(b,s).category != Move::Other || b.player(t) == b.player(s)) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        b.fail(s, 199, 0, Pokemon::Fairy, t);
    }
};

struct MMBelch :  public MM
{
    MMBelch() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
        if (!b.battleMemory().value(QString("BerryEaten%1%2").arg(b.player(s)).arg(b.currentInternalId(s))).toBool()) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMElectricTerrain : public MM {
    MMElectricTerrain() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(24, 0) ;
    }

    static void daf(int s, int, BS &b) {
        if (b.terrain == Type::Electric && b.terrainCount > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(201,0,s,Pokemon::Electric);
        b.terrainCount = 5;
        b.terrain = Type::Electric;
        b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "ElectricTerrain", &et);
    }

    static void et(int s, int, BS &b) {
        if (b.terrain != Type::Electric) {
            return;
        }
        b.terrainCount -= 1;
        if (b.terrainCount <= 0) {
            b.sendMoveMessage(201,1,s,Pokemon::Electric);
            b.terrain = 0;
            b.removeEndTurnEffect(BS::FieldEffect, 0, "ElectricTerrain");
        }
    }
};

struct MMElectrify : public MM {
    MMElectrify() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ms(int s, int, BS &b) {
        tmove(b,s).type = Pokemon::Electric;
    }

    static void uas(int s, int t, BS &b) {
        addFunction(turn(b,t), "MoveSettings", "Electrify", &ms);
        b.sendMoveMessage(202,0,s,Pokemon::Electric,t);
    }
};

struct MMFairyLock : public MM {
    MMFairyLock() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(24, 0) ;
    }

    static void daf(int s, int, BS &b) {
        if (b.battleMemory().value("FairyLockCount").toInt() > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(203,0,s,Pokemon::Fairy);
        b.battleMemory()["FairyLockCount"] = 2;
        b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "FairyLock", &et);
    }

    static void et(int s, int, BS &b) {
        inc(b.battleMemory()["FairyLockCount"], -1);
        if (b.battleMemory()["FairyLockCount"].toInt() <= 0) {
            b.sendMoveMessage(203,1,s,Pokemon::Fairy);
            b.battleMemory().remove("FairyLockCount");
            b.removeEndTurnEffect(BS::FieldEffect, 0, "FairyLock");
        }
    }
};

struct MMFellStinger : public MM {
    MMFellStinger() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        if (b.koed(t)) {
            tmove(b,s).statAffected = Attack << 16;
            tmove(b,s).boostOfStat = 2 << 16;
            tmove(b,s).classification = Move::OffensiveSelfStatChangingMove;
        }
    }
};

struct MMGrassyTerrain : public MM {
    MMGrassyTerrain() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(24, 0) ;
    }

    static void daf(int s, int, BS &b) {
        if (b.terrainCount > 0 && b.terrain == Type::Grass) {
            fturn(b,s).add(TM::Failed);
        }
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(205,0,s,Pokemon::Grass);
        b.terrainCount = 5;
        b.terrain = Type::Grass;
        b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "GrassyTerrain", &et);
    }

    static void et(int s, int, BS &b) {
        if (b.terrain != Type::Grass) {
            return;
        }
        b.terrainCount -= 1;
        if (b.terrainCount <= 0) {
            b.sendMoveMessage(205,1,s,Pokemon::Grass);
            b.terrain = Type::Curse;
            b.removeEndTurnEffect(BS::FieldEffect, 0, "GrassyTerrain");
        } else {
            b.sendMoveMessage(205,2,s,Pokemon::Grass);
            foreach (int p, b.sortedBySpeed()) {
                if (!b.isFlying(p)) {
                    b.healLife(p, b.poke(p).totalLifePoints()/16);
                }
            }
        }
    }
};

struct MMKingsShield: public MM
{
    MMKingsShield() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "KingsShield", &dgaf);
        turn(b,s)["KingsShieldUsed"] = true;
        b.sendMoveMessage(206, 0, s, type(b,s));
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        if (!turn(b,t)["KingsShieldUsed"].toBool()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        if (tmove(b,s).category == Move::Other) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;

        /* All other moves fail */
        if (turn(b,s).contains("TellPlayers")) { /* if the move was secret and cancelled, disclose it (like free fall) */
            b.notify(BS::All, BattleCommands::UseAttack, s, qint16(move(b,s)), false);
        }
        b.fail(s, 27, 0, Pokemon::Normal, t);

        if (tmove(b,s).flags & Move::ContactFlag) {
            b.inflictStatMod(s, Attack, -2, t);
        }
    }
};

struct MMMatBlock : public MM
{
    MMMatBlock() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (MMDetect::othersHaveMoved(s,s,b)) {
            return;
        }
        MMFakeOut::daf(s,s,b);
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "MatBlock", &dgaf);
        team(b,b.player(s))["MatBlockUsed"] = b.turn();
        b.sendMoveMessage(207, 0, s, type(b,s));
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        int target = b.player(t);
        if (!team(b,target).contains("MatBlockUsed") || team(b,target)["MatBlockUsed"].toInt() != b.turn()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        if (tmove(b,s).category == Move::Other) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        b.fail(s, 207, 0, Pokemon::Fighting, t);
    }
};

struct MMMistyTerrain : public MM {
    static const int type = -Type::Fairy;

    MMMistyTerrain() {
        functions["UponAttackSuccessful"] = &uas;
        functions["DetermineAttackFailure"] = &daf;
    }

    static ::bracket bracket(Pokemon::gen) {
        return makeBracket(24, 0) ;
    }

    static void daf(int s, int, BS &b) {
        if (b.terrain == type && b.terrainCount > 0) {
            fturn(b,s).add(TM::Failed);
        }
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(208,0,s,Pokemon::Fairy);
        b.terrainCount = 5;
        b.terrain = type;
        b.addEndTurnEffect(BS::FieldEffect, bracket(b.gen()), 0, "MistyTerrain", &et);
    }

    static void et(int s, int, BS &b) {
        if (b.terrain != type) {
            return;
        }
        b.terrainCount --;
        if (b.terrainCount <= 0) {
            b.sendMoveMessage(208,1,s,Pokemon::Fairy);
            b.terrain = 0;
            b.removeEndTurnEffect(BS::FieldEffect, 0, "MistyTerrain");
        }
    }
};

struct MMSpikyShield : public MM
{
    MMSpikyShield() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "SpikyShield", &dgaf);
        turn(b,s)["SpikyShieldUsed"] = true;
        b.sendMoveMessage(27, 0, s, Pokemon::Grass);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        if (!turn(b,t)["SpikyShieldUsed"].toBool()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        if (turn(b,s).contains("TellPlayers")) { /* if the move was secret and cancelled, disclose it (like free fall) */
            b.notify(BS::All, BattleCommands::UseAttack, s, qint16(move(b,s)), false);
        }
        b.fail(s, 27, 0, Pokemon::Grass, t);

        if ((tmove(b, s).flags & Move::ContactFlag) ) {
            b.inflictDamage(s,b.poke(s).totalLifePoints()/6,s,false);
            b.sendMoveMessage(209,0,s);
            return;
        }
    }
};

struct MMStickyWeb : public MM
{
    MMStickyWeb() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        if (team(b,t).value("StickyWeb").toBool() == true) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void uas(int s, int, BS &b) {
        int t = b.opponent(b.player(s));
        team(b,t)["StickyWeb"] = true;
        addFunction(team(b,t), "UponSwitchIn", "StickyWeb", &usi);
        b.sendMoveMessage(210,0,s,Pokemon::Bug,t);
    }

    static void usi(int source, int s, BS &b) {
        int t = b.opponent(b.player(s));
        if (!b.koed(s) && team(b,source).value("StickyWeb").toBool() == true && !b.isFlying(s))
        {
            b.sendMoveMessage(210,1,s,Pokemon::Bug);
            b.inflictStatMod(s, Speed, -1, t);
        }
    }
};

struct MMTopsyTurvy : public MM {
    MMTopsyTurvy() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(211, 0, s, type(b,s), t);

        for (int i = 0; i < 8; i++) {
            fpoke(b, t).boosts[i] *= -1;
        }
    }
};

struct MMVenomDrench : public MM {
    MMVenomDrench() {
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (!(b.poke(t).status() == Pokemon::Poisoned)) {
            fturn(b,s).add(TM::Failed);
        }
    }
};

struct MMFlowerShield : public MM {
    MMFlowerShield() {
        functions["BeforeTargetList"] = &btl;
        functions["UponAttackSuccessful"] = &uas;
    }

    /* Copied from Haze*/
    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            b.targetList.clear();
            b.targetList.push_back(s);
        }
    }
    static void uas(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            foreach (int p, b.sortedBySpeed())
            {
                if (b.hasType(p, Type::Grass)) {
                    b.inflictStatMod(p, Defense, 1, s);
                }
            }
        }
    }
};

struct MMRototiller : public MM {
    MMRototiller() {
        functions["BeforeTargetList"] = &btl;
        functions["UponAttackSuccessful"] = &uas;
    }

    /* Copied from Haze*/
    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            b.targetList.clear();
            b.targetList.push_back(s);
        }
    }
    static void uas(int s, int, BS &b) {
        if (tmove(b,s).power == 0) {
            foreach (int p, b.sortedBySpeed())
            {
                if (b.hasType(p, Type::Grass) && !b.isFlying(p)) {
                    b.inflictStatMod(p, Attack, 1, s);
                    b.inflictStatMod(p, SpAttack, 1, s);
                }
            }
        }
    }
};

struct MMPowder : public MM
{
    MMPowder() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
        b.sendMoveMessage(215,0,s,type(b,s),t);

        addFunction(poke(b,t), "MovePossible", "Powder", &mp);
        poke(b,t)["Powdered"] = true;
    }

    static void mp(int s, int, BS &b)
    {
        addFunction(turn(b,s), "AfterTellingPlayers", "Powder", &atp);
    }

    static void atp(int s, int, BS &b)
    {
        if (poke(b,s).value("Powdered").toBool()) {
            if (type(b,s) == Type::Fire) {
                b.sendMoveMessage(215, 1, s, Pokemon::Fire);
                removeFunction(poke(b,s), "MovePossible", "Powder");
                b.inflictDamage(s, b.poke(s).totalLifePoints()/4, s);
                turn(b,s)["ImpossibleToMove"] = true;
            }
            poke(b,s).remove("Powdered");
        }
    }
};

struct MMMagneticFlux : public MM {
    MMMagneticFlux() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        foreach (int p, b.sortedBySpeed())
        {
            if (b.player(p) == b.player(s)) {
                if (b.hasWorkingAbility(p, Ability::Minus) || b.hasWorkingAbility(p, Ability::Plus)) {
                    b.inflictStatMod(p, Defense, 1, s);
                    b.inflictStatMod(p, SpDefense, 1, s);
                }
            }
        }
    }
};

struct MMIonDeluge : public MM {
    MMIonDeluge() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(217, 0, s, Type::Electric);
        addFunction(b.battleMemory(), "MovePossible", "IonDeluge", &mp);
        b.battleMemory()["IonDelugeTurn"] = b.turn();
    }

    static void mp(int s, int, BS &b) {
        if (b.battleMemory().value("IonDelugeTurn").toInt() != b.turn()) {
            b.battleMemory().remove("IonDelugeTurn");
            removeFunction(b.battleMemory(), "MovePossible", "IonDeluge");
            return;
        }
        if (tmove(b,s).type == Type::Normal) {
            tmove(b,s).type = Type::Electric;
        }
    }
};

//ITS VERY IMPORTANT TO CELEBRATE YOUR SPECIAL DAY, OKAY?
struct MMCelebrate : public MM {
    MMCelebrate() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        b.sendMoveMessage(218,0,s,type(b,s));
    }
};

/* List of events:
    *UponDamageInflicted -- turn: just after inflicting damage
    *DetermineAttackFailure -- turn, poke: set fturn(b,s).add(TM::Failed) to true to make the attack fail
    *DetermineGeneralAttackFailure -- battle: this is a battlefield effect, same as above
    *EndTurn -- poke, battle: Called at the end of the turn
    *UponOffensiveDamageReceived -- turn: when the player received damage (not the substitute) from an attack
    *OnSetup -- none: when the move is setup
    *TurnSettings -- poke: Will be called at the beginning of the turn before even chosing moves.
    *EvenWhenCantMove -- turn: Will be called before even status check, useful for attacks like fly etc
    *BeforeTakingDamage -- turn: explicit
    *UponSwitchIn -- turn: When a new poke is switched in, like for Baton Pass/U-turn
    *AfterSwitchIn -- turn: after it's switched in, i.e after the entry effects are called
    *MoveSettings -- turn: Just after losing PP, and before chosing the target
    *BeforeTargetList -- turn: Before processing the attack for each target
    *BeforeCalculatingDamage -- turn: The right moment to change the base power of the attack if needed
    *CustomAttackingDamage -- turn: If the attack does a certain amount of damage, without regard to things like base power, inflict it here
    *UponAttackSuccessful -- turn: after inflicting damage (and damage effects called) / just after succeeding the move if the move has 0 BP
    *AfterAttackSuccessful -- turn: at the very end of the attack for that target
    *BeforeHitting -- turn: this is called instead when BeforeCalculatingDamage is not (like, brick break activates after BeforeCalculatingDamage, but before
 calculating the damages lol because it won't activate if it fails but it's still attacking
    *DetermineAttackPossible -- poke: just say if the poke is supposed to be able to attack, regarless of the the move used (like attracted pokes won't attack)
    *MovePossible -- turn: before attacking, say if the move is possible or not (like when a move just got blocked by encore, taunt,disable)
    *MovesPossible -- poke, battle: at the beginning of the turn, tells if each move is possible or not
    *AfterKoedByStraightAttack -- poke: when koed by an attack
    *BlockTurnEffects -- poke: Called before calling effects for a turn event, to see if it's blocked. Used by Substitute
    *AttackSomehowFailed -- turn, only offensive moves: When an attack fails, or misses, there may be something to do (jump kick, rollout, ..)
    *StatusChange -- poke
    *BeforeEnding
    *GeneralTargetChange
*/

#define REGISTER_MOVE(num, name) mechanics[num] = MM##name(); names[num] = #name; nums[#name] = num;

void MoveEffect::init()
{
    //REGISTER_MOVE(1, Leech); /* absorb, drain punch, part dream eater, giga drain, leech life, mega drain */ <- Built -in now, but message still there
    REGISTER_MOVE(2, AquaRing);
    REGISTER_MOVE(3, AromaTherapy);
    REGISTER_MOVE(4, Assist);
    REGISTER_MOVE(5, Assurance);
    REGISTER_MOVE(6, BatonPass);
    REGISTER_MOVE(7, BeatUp);
    REGISTER_MOVE(8, BellyDrum);
    REGISTER_MOVE(9, Bide);
    REGISTER_MOVE(10, Bind);
    REGISTER_MOVE(11, BlastBurn); /* BlastBurn, Hyper beam, rock wrecker, giga impact, frenzy plant, hydro cannon, roar of time */
    REGISTER_MOVE(12, Block);
    REGISTER_MOVE(13, Bounce);
    REGISTER_MOVE(14, BrickBreak);
    REGISTER_MOVE(15, Brine);
    REGISTER_MOVE(16, BugBite); /*Bug Bite, Pluck */
    REGISTER_MOVE(17, Camouflage);
    REGISTER_MOVE(18, Charge);
    REGISTER_MOVE(19, Conversion);
    REGISTER_MOVE(20, Conversion2);
    REGISTER_MOVE(21, Copycat);
    REGISTER_MOVE(22, Counter);
    REGISTER_MOVE(23, Covet);
    REGISTER_MOVE(24, CrushGrip); /* Crush grip, Wring out */
    REGISTER_MOVE(25, Curse);
    REGISTER_MOVE(26, DestinyBond);
    REGISTER_MOVE(27, Detect); /* Protect, Detect */
    REGISTER_MOVE(28, Disable);
    REGISTER_MOVE(29, DoomDesire);
    REGISTER_MOVE(30, DragonRage);
    REGISTER_MOVE(31, DreamingTarget); /* Part Dream eater, part Nightmare */
    REGISTER_MOVE(32, Embargo);
    REGISTER_MOVE(33, Encore);
    REGISTER_MOVE(34, Endeavor);
    REGISTER_MOVE(35, Endure);
    REGISTER_MOVE(36, Eruption); /* Eruption, Water sprout */
    REGISTER_MOVE(37, FaintUser); /* Memento, part explosion, selfdestruct, lunar dance, healing wish... */
    REGISTER_MOVE(38, Blizzard);
    REGISTER_MOVE(39, Facade);
    REGISTER_MOVE(40, FakeOut);
    REGISTER_MOVE(41, FalseSwipe);
    REGISTER_MOVE(42, Feint);
    REGISTER_MOVE(43, OHKO); /* Fissure, Guillotine, Horn Drill, Sheer cold */
    REGISTER_MOVE(44, Flail); /* Flail, Reversal */
    REGISTER_MOVE(45, Fling);
    REGISTER_MOVE(46, FocusEnergy);
    REGISTER_MOVE(47, FocusPunch);
    REGISTER_MOVE(48, FollowMe);
    REGISTER_MOVE(49, Frustration); /* Frustration, Return */
    REGISTER_MOVE(50, FuryCutter);
    REGISTER_MOVE(51, GastroAcid);
    REGISTER_MOVE(52, GrassKnot);
    REGISTER_MOVE(53, Gravity);
    REGISTER_MOVE(54, Grudge); //doesn't work
    REGISTER_MOVE(55, BoostSwap);
    REGISTER_MOVE(56, GyroBall);
    REGISTER_MOVE(57, Weather);
    REGISTER_MOVE(58, Attract);
    REGISTER_MOVE(59, HealBlock);
    //REGISTER_MOVE(60, HealHalf); Now built in, but move message still used
    REGISTER_MOVE(61, HealingWish);
    REGISTER_MOVE(62, PowerTrick);
    REGISTER_MOVE(63, HelpingHand);
    REGISTER_MOVE(64, JumpKick);
    REGISTER_MOVE(65, HiddenPower);
    REGISTER_MOVE(66, IceBall);
    REGISTER_MOVE(67, Imprison);
    REGISTER_MOVE(68, MagnetRise);
    REGISTER_MOVE(69, Judgment);
    REGISTER_MOVE(70, KnockOff);
    REGISTER_MOVE(71, LastResort);
    REGISTER_MOVE(72, LeechSeed);
    REGISTER_MOVE(73, TeamBarrier);
    REGISTER_MOVE(74, LockOn);
    REGISTER_MOVE(75, LuckyChant);
    REGISTER_MOVE(76, MagicCoat);
    REGISTER_MOVE(77, Defog);
    REGISTER_MOVE(78, Magnitude);
    REGISTER_MOVE(79, MeFirst);
    REGISTER_MOVE(80, Metronome);
    REGISTER_MOVE(81, Mimic);
    REGISTER_MOVE(82, Splash);
    REGISTER_MOVE(83, Minimize);
    REGISTER_MOVE(84, MiracleEye);
    REGISTER_MOVE(85, MirrorMove);
    REGISTER_MOVE(86, Mist);
    REGISTER_MOVE(87, Moonlight);
    REGISTER_MOVE(88, MudSport);
    REGISTER_MOVE(89, NaturalGift);
    REGISTER_MOVE(90, NaturePower);
    REGISTER_MOVE(91, NightShade);
    REGISTER_MOVE(92, NightMare);
    REGISTER_MOVE(93, Outrage);
    REGISTER_MOVE(94, PainSplit);
    REGISTER_MOVE(95, PerishSong);
    REGISTER_MOVE(96, Present);
    REGISTER_MOVE(97, Psychup);
    REGISTER_MOVE(98, PsychoShift);
    REGISTER_MOVE(99, Psywave);
    REGISTER_MOVE(100, Punishment);
    REGISTER_MOVE(101, Captivate);
    REGISTER_MOVE(102, Rage);
    REGISTER_MOVE(103, RapidSpin);
    REGISTER_MOVE(104, RazorWind);
    REGISTER_MOVE(105, Recycle);
    REGISTER_MOVE(106, Rest);
    REGISTER_MOVE(107, Roar);
    REGISTER_MOVE(108, RolePlay);
    REGISTER_MOVE(109, SafeGuard);
    REGISTER_MOVE(110, SecretPower);
    REGISTER_MOVE(111, Sketch);
    REGISTER_MOVE(112, SkillSwap);
    REGISTER_MOVE(113, WeatherBall);
    //REGISTER_MOVE(114, Explosion);
    REGISTER_MOVE(115, SleepingUser);
    REGISTER_MOVE(116, SleepTalk);
    REGISTER_MOVE(117, SmellingSalt);
    REGISTER_MOVE(118, Snatch);
    REGISTER_MOVE(119, MetalBurst);
    REGISTER_MOVE(120, Acupressure);
    REGISTER_MOVE(121, Spikes);
    REGISTER_MOVE(122, SpitUp);
    REGISTER_MOVE(123, Spite);
    REGISTER_MOVE(124, StealthRock);
    REGISTER_MOVE(125, StockPile);
    REGISTER_MOVE(126, Stomp);
    //REGISTER_MOVE(127, Struggle); - removed, but message here
    REGISTER_MOVE(128, Substitute);
    REGISTER_MOVE(129, SuckerPunch);
    REGISTER_MOVE(130, SuperFang);
    REGISTER_MOVE(131, Swallow);
    REGISTER_MOVE(132, Switcheroo);
    REGISTER_MOVE(133, TailWind);
    REGISTER_MOVE(134, Taunt);
    REGISTER_MOVE(135, Torment);
    REGISTER_MOVE(136, ToxicSpikes);
    REGISTER_MOVE(137, Transform);
    REGISTER_MOVE(138, TrickRoom);
    REGISTER_MOVE(139, TripleKick);
    REGISTER_MOVE(140, UTurn);
    REGISTER_MOVE(141, Uproar);
    REGISTER_MOVE(142, Wish);
    REGISTER_MOVE(143, WorrySeed);
    REGISTER_MOVE(144, Yawn);
    REGISTER_MOVE(145, Payback);
    REGISTER_MOVE(146, Avalanche); /* Avalanche, Revenge */
    //Chatter
    REGISTER_MOVE(148, TrumpCard);
    REGISTER_MOVE(149, Haze);
    REGISTER_MOVE(150, Roost);
    REGISTER_MOVE(151, Ingrain);
    REGISTER_MOVE(152, Thunder);
    //REGISTER_MOVE(153, UnThawing);
    REGISTER_MOVE(154, DefenseCurl);
    REGISTER_MOVE(155, GuardSplit); /* Power Split */
    REGISTER_MOVE(156, MagicRoom);
    REGISTER_MOVE(157, Soak);
    REGISTER_MOVE(158, Entrainment);
    REGISTER_MOVE(159, ShellSmash);
    REGISTER_MOVE(160, Incinerate);
    REGISTER_MOVE(161, FinalGambit);
    REGISTER_MOVE(162, Bestow);
    //REGISTER_MOVE(163, Hurricane);
    REGISTER_MOVE(164, Refresh);
    REGISTER_MOVE(165, Memento);
    REGISTER_MOVE(166, RelicSong);
    REGISTER_MOVE(167, HeavySlam);
    REGISTER_MOVE(168, WonderRoom);
    REGISTER_MOVE(169, WideGuard);
    REGISTER_MOVE(170, QuickGuard);
    //Pursuit
    REGISTER_MOVE(172, ReflectType);
    //REGISTER_MOVE(173, Acrobatics);
    REGISTER_MOVE(174, Telekinesis);
    REGISTER_MOVE(175, SmackDown);
    REGISTER_MOVE(176, AfterYou);
    REGISTER_MOVE(177, Quash);
    REGISTER_MOVE(178, FirePledge);
    REGISTER_MOVE(179, GrassPledge);
    REGISTER_MOVE(180, WaterPledge);
    REGISTER_MOVE(181, EchoedVoice);
    REGISTER_MOVE(182, ElectroBall);
    REGISTER_MOVE(183, TechnoBlast);
    REGISTER_MOVE(184, Round);
    REGISTER_MOVE(185, StoredPower);
    REGISTER_MOVE(186, Synchronoise);
    REGISTER_MOVE(187, FoulPlay);
    REGISTER_MOVE(188, Retaliate);
    REGISTER_MOVE(189, FlameBurst);
    REGISTER_MOVE(190, AllySwitch);
    REGISTER_MOVE(191, Growth);
    //REGISTER_MOVE(192, TriAttack);
    REGISTER_MOVE(193, FusionFlare);
    REGISTER_MOVE(194, FusionBolt);
    REGISTER_MOVE(195, WillOWisp);
    REGISTER_MOVE(196, Swagger);
    REGISTER_MOVE(197, Autotomize);
    //REGISTER_MOVE(198, Spore);
    REGISTER_MOVE(199, CraftyShield);
    REGISTER_MOVE(200, Belch);
    REGISTER_MOVE(201, ElectricTerrain);
    REGISTER_MOVE(202, Electrify);
    REGISTER_MOVE(203, FairyLock);
    REGISTER_MOVE(204, FellStinger);
    REGISTER_MOVE(205, GrassyTerrain);
    REGISTER_MOVE(206, KingsShield);
    REGISTER_MOVE(207, MatBlock);
    REGISTER_MOVE(208, MistyTerrain);
    REGISTER_MOVE(209, SpikyShield);
    REGISTER_MOVE(210, StickyWeb);
    REGISTER_MOVE(211, TopsyTurvy);
    REGISTER_MOVE(212, VenomDrench);
    REGISTER_MOVE(213, FlowerShield);
    REGISTER_MOVE(214, Rototiller);
    REGISTER_MOVE(215, Powder);
    REGISTER_MOVE(216, MagneticFlux);
    REGISTER_MOVE(217, IonDeluge);
    REGISTER_MOVE(218, Celebrate)
}

/* Not done: Ion deluge */

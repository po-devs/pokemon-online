#include "../moves.h"
#include "../miscmoves.h"
#include "../../PokemonInfo/pokemoninfo.h"
#include "../items.h"
#include "../battlecounterindex.h"
#include "../../Shared/battlecommands.h"

typedef BS::priorityBracket bracket;
using namespace Move;
typedef BattleCounterIndex BC;
/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

Q_DECLARE_METATYPE(QList<int>)

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
        if (!b.koed(s) && !b.poke(s).isFull()) {
            b.healLife(s, b.poke(s).totalLifePoints()/16);
            b.sendMoveMessage(2, 1, s, Pokemon::Water);
        }
    }
};

struct MMAssurance : public MM
{
    MMAssurance() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (turn(b,t).contains("DamageTaken")) {
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

        b.sendMoveMessage(16,0,s,type(b,s),t,item);
        b.devourBerry(s, item, s);
        b.disposeItem(t);
    }
};

struct MMCamouflage : public MM {
    MMCamouflage() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        if (b.gen() >= 5) {
            fpoke(b,s).type1 = Pokemon::Ground;
            b.sendMoveMessage(17,0,s,4);
        } else {
            fpoke(b,s).type1 = Pokemon::Normal;
            b.sendMoveMessage(17,0,s,0);
        }
        fpoke(b,s).type2 = Pokemon::Curse;
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
            fpoke(b,s).type1 = type;
            fpoke(b,s).type2 = Pokemon::Curse;
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
            if (!(fpoke(b,s).type1 == i && fpoke(b,s).type2 == Pokemon::Curse) && TypeInfo::Eff(attackType, i) < Type::Effective) {
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
        fpoke(b,s).type1 = type;
        fpoke(b,s).type2 = Pokemon::Curse;
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

        if (trn == b.turn() || (trn+1 == b.turn() && !fturn(b,s).contains(TM::HasMoved))) {
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

    static void daf(int s, int, BS &b) {
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
            return;
        }

        if (poke(b,s).contains("ProtectiveMoveTurn") && poke(b,s)["ProtectiveMoveTurn"].toInt() == b.turn() - 1) {
            if (!testSuccess(poke(b,s)["ProtectiveMoveCount"].toInt(), b)) {
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
        if (turn(b, t)["DetectUsed"].toBool() == true) {
            turn(b, t)["DetectUsed"] = false;
        } else if (b.gen() <= 4){
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
        b.changeHp(s, b.poke(s).lifePoints() - std::max(b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100,1));
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
            if (!b.poke(player,i).ko() && (move == Aromatherapy || (b.gen() == 5 && move == HealBell) || b.poke(player,i).ability() != Ability::Soundproof)) {
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
        if (b.poke(s).status() != Pokemon::Frozen && b.poke(s).status() != Pokemon::Paralysed)
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
        if (b.poke(t).item() != 0 && !b.hasWorkingAbility(t, Ability::StickyHold)
                && (!b.hasWorkingAbility(t, Ability::Multitype) || (b.gen() >= 5 && !ItemInfo::isPlate(b.poke(t).item())))
                && !b.hasWorkingAbility(s, Ability::Multitype)
                && b.poke(s).item() == 0
                && !(b.poke(t).item() == Item::GriseousOrb && (b.gen() <= 4 || PokemonInfo::OriginalForme(b.poke(t).num()) == Pokemon::Giratina || PokemonInfo::OriginalForme(b.poke(s).num()) == Pokemon::Giratina))
                && !ItemInfo::isMail(b.poke(t).item())
                && !(ItemInfo::isDrive(b.poke(t).item()) && (PokemonInfo::OriginalForme(b.poke(s).num()) == Pokemon::Genesect || PokemonInfo::OriginalForme(b.poke(t).num()) == Pokemon::Genesect)))
                /* Sticky Hold, MultiType, Giratina_O, Mail, Genesect Drives*/
        {
            b.sendMoveMessage(23,(move(b,s)==Covet)?0:1,s,type(b,s),t,b.poke(t).item());
            b.acqItem(s, b.poke(t).item());
            b.loseItem(t);
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
        if (move == 0 || move == Copycat || move == Move::DragonTail || move == Move::OverheadThrow || move == Move::Struggle) {
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
            (*this) << NoMove << Assist << Chatter << Copycat << Counter << Covet << DestinyBond << Detect
                              << DragonTail << Endure << FastGuard << Feint << FocusPunch << FollowMe << HelpingHand << MeFirst
                              << Metronome << Mimic << MirrorCoat << MirrorMove << OverheadThrow << Protect  << RagePower
                              << Sketch << SleepTalk << Snatch << Struggle << Switcheroo << Thief << Trick << WideGuard;
        }

        bool contains(int move, Pokemon::gen gen=GenInfo::GenMax()) const {
            if (move == Transform || move == NaturePower) {
                return gen >= 5;
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

                    b.inflictDamage(s, b.poke(s).totalLifePoints()/(b.hasWorkingItem(trapper, Item::PressureBand) ? 8 : 16),s,false);
                }
            }
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
        if (b.hasSubstitute(t) && move(b,s) == Move::FreeFall) {
            b.notify(BS::All, BattleCommands::UseAttack, s, qint16(Move::FreeFall));
            fturn(b,s).add(TM::Failed);
        }
    }

    static void ms(int s, int, BS &b) {
        if (b.hasWorkingItem(s, Item::PowerHerb)) {
            QStringList args = turn(b,s)["Bounce_Arg"].toString().split('_');
            b.sendMoveMessage(13,args[0].toInt(),s,type(b,s),s,move(b,s));
            b.sendItemMessage(11,s);
            b.disposeItem(s);

            removeFunction(turn(b,s), "UponAttackSuccessful", "Bounce");
            if (move(b,s) == ShadowForce) {
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &uas2);
            }
        } else {
            tmove(b, s).power = 0;
            tmove(b, s).accuracy = 0;
            if (move(b,s) != Move::FreeFall) {
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

            if (move == ShadowForce) {
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &uas2);
            } else if (move == FreeFall) {
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

    /* Only called with Shadow Force, breaks protect */
    static void uas2(int , int t, BS &b) {
        if (turn(b, t).value("DetectUsed").toBool() == true) {
            turn(b, t)["DetectUsed"] = false;
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
        if (att == Move::FreeFall) {
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

    static void daf (int s, int, BS &b) {
        if (!turn(b,s).contains("CounterDamage")) {
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
                    if (typemod == 0) {
                        /* If it's ineffective we just say it */
                        b.notify(BS::All, BattleCommands::Effective, s, quint8(typemod));
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
                    b.notify(BS::All, BattleCommands::Effective, s, quint8(typemod));
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
            b.removeEndTurnEffect(BS::PokeEffect, s, "Embargo");
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
            if (fturn(b,t).contains(TM::NoChoice)) {
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
                    if (b.PP(s, i) <= 0)
                        b.counters(s).removeCounter(BC::Encore);
                    break;
                }
            }
        }
        if (b.counters(s).count(BC::Encore) < 0) {
            removeFunction(poke(b,s), "MovesPossible", "Encore");
            b.removeEndTurnEffect(BS::PokeEffect, s, "Encore");
            b.sendMoveMessage(33,0,s);
            b.counters(s).removeCounter(BC::Encore);
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
        functions["AttackSomehowFailed"] = &ma;
        functions["BeforeCalculatingDamage"] = &bcd;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void ma(int s, int, BS &b) {
        poke(b,s)["FuryCutterCount"] = 0;
    }

    static void uas(int s, int, BS &b) {
        if (b.gen() >= 5 && poke(b,s)["LastMoveUsed"].toInt() != FuryCutter) {
            poke(b,s)["FuryCutterCount"] = 0;
        }
        poke(b,s)["FuryCutterCount"] = std::min(poke(b,s)["FuryCutterCount"].toInt() * 2 + 1,b.gen().num() == 4 ? 15 : 7);
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
            tmove(b,s).power = 5 + (PokemonInfo::BaseStats(fpoke(b,s).id).baseAttack()/10);
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
        functions["OnFoeOnAttack"] = &uas;
        functions["BeforeTargetList"] = &btl;
    }

    static void daf(int s, int, BS &b) {
        if (!turn(b,s).contains("FlingItem")) {
            fturn(b,s).add(TM::Failed);
        }
    }

    static void btl(int s, int, BS &b) {
        if (b.poke(s).item() != 0 && b.hasWorkingItem(s, b.poke(s).item()) && ItemInfo::Power(b.poke(s).item()) > 0) {
            if (b.gen() >= 5 && b.hasWorkingAbility(s, Ability::Klutz))
                return;
            turn(b,s)["FlingItem"] = b.poke(s).item();
            tmove(b, s).power = tmove(b, s).power * ItemInfo::Power(b.poke(s).item());
            int t = b.targetList.front();
            b.sendMoveMessage(45, 0, s, type(b,s), t, b.poke(s).item());
            b.disposeItem(s);
        }
    }

    static void uas (int s, int t, BS &b) {
        int item = turn(b,s)["FlingItem"].toInt();
        if (!ItemInfo::isBerry(item)) {
            if (item == Item::WhiteHerb || item == Item::MentalHerb) {
                int oppitem = b.poke(t).item();
                ItemEffect::activate("OnSetup", item, t,s,b);
                b.poke(t).item() = oppitem; /* the effect of mental herb / white herb may have disposed of the foes item */
            } else if (item == Item::RazorFang || item == Item::KingsRock) {
                fturn(b,t).add(TM::Flinched); /* king rock, razor fang */
            } else if (!team(b, b.player(t)).contains("SafeGuardCount"))  {
                switch (item) {
                case Item::FlameOrb: b.inflictStatus(t, Pokemon::Burnt, s); break; /*flame orb*/
                case Item::ToxicOrb: b.inflictStatus(t, Pokemon::Poisoned, s, 15, 15); break; /*toxic orb*/
                case Item::LightBall: b.inflictStatus(t, Pokemon::Paralysed, s); break; /* light ball */
                case Item::PoisonBarb: b.inflictStatus(t, Pokemon::Poisoned, s); break; /* poison barb */
                }
            }
        } else {
            b.sendMoveMessage(16,0,t,type(b,s),t,item);
            b.devourBerry(s, item, s);
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

    static void gtc(int s, int, BS &b) {
        int tar = b.opponent(b.player(s));

        if (team(b, tar)["FollowMeTurn"] != b.turn())
            return;
        int target = team(b, tar)["FollowMePlayer"].toInt();
        if (b.koed(target) || !poke(b, target).contains("FollowMe") || b.player(s) == b.player(target))
            return;

        int tarChoice = tmove(b,s).targets;
        bool muliTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

        if (muliTar) {
            return;
        }

        if (!b.canTarget(move(b,s), s, target)) {
            return;
        }

        turn(b,s)["TargetChanged"] = true;
        turn(b,s)["Target"] = target;
    }
};

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
                if (move == Fly || move == Bounce || move == FreeFall) {
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
            (*this) << Bounce << Fly << FreeFall << JumpKick << HiJumpKick << Splash << MagnetRise;
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

MMGravity::FM MMGravity::forbidden_moves;

struct MMSmackDown : public MM
{
    MMSmackDown() {
        functions["OnFoeOnAttack"] = &ofoa;
    }

    static void ofoa(int s, int t, BS &b) {
        if (b.isFlying(t)) {
            b.sendMoveMessage(175, 0, t, type(b,s), s);
            poke(b,t)["SmackedDown"] = true;
        }

        if (b.koed(t)) {
            return;
        }

        if(poke(b,t).value("Invulnerable").toBool()) {
            int move = poke(b,t)["2TurnMove"].toInt();
            if (move == Fly || move == Bounce || move == FreeFall) { 
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
            int move = b.randint(MoveInfo::NumberOfMoves());

            if (!MoveInfo::Exists(move, b.gen())) {
                continue;
            }

            bool correctMove = !b.hasMove(s,move) && ((b.gen() <= 4 && !MMAssist::forbidden_moves.contains(move, b.gen())) ||
                                                      (b.gen() >= 5 && !forbidden.contains(move)));

            if (correctMove) {
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

    struct MMMetroSet : public QSet<int> {
        MMMetroSet() {
            (*this).unite(MMAssist::forbidden_moves );

            (*this) << Move::GiftPass << Move::YouFirst << Move::IceBurn << Move::FreezeShock
                                                 << Move::NaturePower << Move::Stall << Move::FastGuard << Move::RagePower
                                                 << Move::AncientSong << Move::SacredSword << Move::TechnoBuster << Move::Transform
                                                 << Move::V_Generate << Move::WideGuard << Move::BackOut;
        }
    };

    static MMMetroSet forbidden;
};

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

        if (team(b,target) != team(b,b.player(s)) && tmove(b,s).attack == Move::Feint) {
            if (team(b,target).contains("WideGuardUsed")) {
                team(b,target).remove("WideGuardUsed");
                b.sendMoveMessage(169, 1, t, Pokemon::Normal);
                return;
            }
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

struct MMFastGuard : public MM
{
    MMFastGuard() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure", "FastGuard", &dgaf);
        team(b,b.player(s))["FastGuardUsed"] = b.turn();
        b.sendMoveMessage(170, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        int target = b.player(t);
        if (!team(b,target).contains("FastGuardUsed") || team(b,target)["FastGuardUsed"].toInt() != b.turn()) {
            return;
        }

        if (team(b,target) != team(b,b.player(s)) && tmove(b,s).attack == Move::Feint) {
            if (team(b,target).contains("FastGuardUsed")) {
                team(b,target).remove("FastGuardUsed");
                b.sendMoveMessage(170, 1, t, Pokemon::Normal);
                return;
            }
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) && tmove(b,s).attack != Move::Feint) {
            return;
        }

        /* Mischievous heart looks at the priority ofthe move, the raw one.
           If the priority was altered by Mischievous heart or whatever,
           that doesn't matter */
        if (MoveInfo::SpeedPriority(tmove(b,s).attack, b.gen()) <= 0) {
            return;
        }

        /* Mind Reader */
        if (poke(b,s).contains("LockedOn") && poke(b,t).value("LockedOnEnd").toInt() >= b.turn() && poke(b,s).value("LockedOn").toInt() == t )
            return;
        /* All other moves fail */
        b.fail(s, 170, 0, Pokemon::Normal, t);
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

void init_moves_1(QHash<int, MoveMechanics> &mechanics, QHash<int, QString> &names, QHash<QString, int> &nums)
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
    REGISTER_MOVE(16, BugBite);
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

    REGISTER_MOVE(53, Gravity);

    REGISTER_MOVE(80, Metronome);

    REGISTER_MOVE(169, WideGuard);
    REGISTER_MOVE(170, FastGuard);

    REGISTER_MOVE(175, SmackDown);
}

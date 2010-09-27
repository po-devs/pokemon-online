#include "moves.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "items.h"

QHash<int, MoveMechanics> MoveEffect::mechanics;
QHash<int, QString> MoveEffect::names;
QHash<QString, int> MoveEffect::nums;

Q_DECLARE_METATYPE(QList<int>);

using namespace Move;

int MoveMechanics::num(const QString &name)
{
    if (!MoveEffect::nums.contains(name)) {
	return 0;
    } else {
	return MoveEffect::nums[name];
    }
}

MoveEffect::MoveEffect(int num, int gen, BattleSituation::BasicMoveInfo &data)
{
    /* Different steps: critical raise, number of times, ... */
    data.critRaise = MoveInfo::CriticalRaise(num, gen);
    data.repeatMin = MoveInfo::RepeatMin(num, gen);
    data.repeatMax = MoveInfo::RepeatMax(num, gen);
    data.priority = MoveInfo::SpeedPriority(num, gen);
    data.flags = MoveInfo::Flags(num, gen);
    data.power = MoveInfo::Power(num, gen);
    data.accuracy = MoveInfo::Acc(num, gen);
    data.type = MoveInfo::Type(num, gen);
    data.category = MoveInfo::Category(num, gen);
    data.rate = MoveInfo::EffectRate(num, gen);
    //(*this)["StatEffect"] = MoveInfo::Effect(num, gen);
    data.flinchRate = MoveInfo::FlinchRate(num, gen);
    data.recoil = MoveInfo::Recoil(num, gen);
    data.attack = num;
    data.targets = MoveInfo::Target(num, gen);
    data.healing = MoveInfo::Healing(num, gen);
    data.classification = MoveInfo::Classification(num, gen);
    data.status = MoveInfo::Status(num, gen);
    data.statusKind = MoveInfo::StatusKind(num, gen);
    data.minTurns = MoveInfo::MinTurns(num, gen);
    data.maxTurns = MoveInfo::MaxTurns(num, gen);
    data.statAffected = MoveInfo::StatAffected(num, gen);
    data.boostOfStat = MoveInfo::BoostOfStat(num, gen);
    data.rateOfStat = MoveInfo::RateOfStat(num, gen);
}

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

void MoveEffect::setup(int num, int source, int target, BattleSituation &b)
{
    /* first the basic info */
    MoveEffect e(num, b.gen(), b.fieldmoves[source]);

    /* then the hard info */
    QStringList specialEffects = MoveInfo::SpecialEffect(num).split('|');

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
	    b.turnlong[source][n+"_Arg"] = specialEffectS.mid(pos+1);
	}

	for(i = m.functions.begin(); i != m.functions.end(); ++i) {
	    if (i.key() == "OnSetup") {
		i.value()(source,target,b);
	    } else {
		Mechanics::addFunction(b.turnlong[source], i.key(), n, i.value());
	    }
	}
    }
}


/* Used by moves like Metronome that may use moves like U-Turn. Then AfterAttackSuccessful would be called twice, and that would
    not be nice because U-Turning twice :s*/
void MoveEffect::unsetup(int num, int source, BattleSituation &b)
{
    /* then the hard info */
    QStringList specialEffects = MoveInfo::SpecialEffect(num).split('|');

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
                Mechanics::removeFunction(b.turnlong[source], i.key(), n);
            }
        }
    }

    b.fieldmoves[source].classification = Move::StandardMove;
}

/* List of events:
    *UponDamageInflicted -- turn: just after inflicting damage
    *DetermineAttackFailure -- turn, poke: set turn()["Failed"] to true to make the attack fail
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
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "EndTurn60", "AquaRing", &et);
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

struct MMBatonPass : public MM
{
    MMBatonPass() {
	functions["DetermineAttackSuccessful"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
        // the function has to be in that list when called by metronome, so it can be unsetup
        functions["AfterAttackFinished"] = &aaf;
    }

    static void daf(int s, int, BS &b) {
        if (b.countBackUp(b.player(s)) == 0) {
	    turn(b,s)["Failed"] =true;
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
	c.remove("Move3Used");
        c.remove("BerryUsed");
        c.remove("HasMovedOnce");
        /* Removing attract */
        c.remove("AttractBy");
        foreach( int opp, b.revs(s)) {
            if (b.linked(opp, "Attract"))
                poke(b, opp).remove("AttractBy");
        }

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
        //and we decrease the switch count associated, so attract & co still work
        inc(slot(b,s)["SwitchCount"], -1);

        if (poke(b,s).value("Substitute").toBool()) {
            b.notifySub(s,true);
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
    }

    static void aas(int s, int, BS &b) {
        if (!turn(b,s).contains("UTurnSuccess")) {
            return;
        }
        if (b.countAlive(s) <= 1) {
            return;
        }
        if (b.koed(s)) {
            return;
        }
        b.requestSwitch(s);
    }
};


struct MMBlastBurn : public MM
{
    MMBlastBurn() {
        functions["UponAttackSuccessful"] = &aas;
    }

    static void aas(int s, int, BS &b) {
	addFunction(poke(b, s), "TurnSettings", "BlastBurn", &ts);
	poke(b, s)["BlastBurnTurn"] = b.turn();
    }
    static void ts(int s, int, BS &b) {
	if (poke(b, s)["BlastBurnTurn"].toInt() < b.turn() - 1) {
            return;
        }

        turn(b, s)["NoChoice"] = true;

        addFunction(turn(b,s), "MoveSettings", "BlastBurn", &ms);
    }

    static void ms(int s, int, BS &b) {
        turn(b, s)["TellPlayers"] = false;
        tmove(b, s).targets = Move::User;
        addFunction(turn(b,s), "UponAttackSuccessful", "BlastBurn", &uas);
    }

    static void uas(int s, int, BS &b) {
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
	/* First check if there's even 1 move available */
	for (int i = 0; i < 4; i++) {
            if (MoveInfo::Type(b.move(s,i), b.gen()) != Type::Curse) {
		break;
	    }
	    if (i == 3) {
		turn(b, s)["Failed"] = true;
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
            turn(b,s)["ConversionType"] = MoveInfo::Type(poss[b.true_rand()%poss.size()], b.gen());
	} else {
	    QList<int> poss;
	    for (int i = 0; i < 4; i++) {
                if (MoveInfo::Type(b.move(s,i), b.gen()) != Type::Curse && MoveInfo::Type(b.move(s,i), b.gen()) != fpoke(b,s).type1) {
		    poss.push_back(b.move(s,i));
		}
	    }
	    if (poss.size() == 0) {
		turn(b, s)["Failed"] = true;
	    } else {
                turn(b,s)["ConversionType"] = MoveInfo::Type(poss[b.true_rand()%poss.size()], b.gen());
	    }
	}
    }

    static void uas(int s, int, BS &b) {
	int type = turn(b,s)["ConversionType"].toInt();
        fpoke(b,s).type1 = type;
        fpoke(b,s).type2 = Pokemon::Curse;
	b.sendMoveMessage(19, 0, s, type, s);
    }
};

struct MMConversion2 : public MM
{
    MMConversion2() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	if (!poke(b,s).contains("LastAttackToHit"))
	{
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int attackType = MoveInfo::Type(poke(b,s)["LastAttackToHit"].toInt(), b.gen());
        if (attackType == Type::Curse) {
	    turn(b,s)["Failed"] = true;
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
	    turn(b,s)["Failed"] = true;
	} else {
            turn(b,s)["Conversion2Type"] = poss[b.true_rand()%poss.size()];
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
        tmove(b, s).power = tmove(b, s).power * 120 * b.poke(t).lifePoints()/b.poke(t).totalLifePoints();
    }
};

struct MMCurse : public MM
{
    MMCurse() {
        functions["MoveSettings"] = &ms;
        functions["OnFoeOnAttack"] = &uas;
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

    static void uas(int s, int t, BS &b) {
	if (turn(b,s)["CurseGhost"].toBool() == true) {
            b.inflictPercentDamage(s, 50, s);
            addFunction(poke(b,t), "EndTurn68", "Cursed", &et);
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
	int trn = poke(b,s)["DestinyBondTurn"].toInt();

	if (trn == b.turn() || (trn+1 == b.turn() && !turn(b,s).value("HasMoved").toBool() )) {
	    b.sendMoveMessage(26, 0, s, Pokemon::Ghost, t);
	    b.koPoke(t, s, false);
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
            if (!turn(b,t).value("HasMoved").toBool() && !turn(b,t).value("CantGetToMove").toBool() && !b.koed(t) && s!=t) {
                fail = false;
                break;
            }
        }

        if (fail) {
            turn(b,s)["Failed"] = true;
            return;
        }

	if (poke(b,s).contains("ProtectiveMoveTurn") && poke(b,s)["ProtectiveMoveTurn"].toInt() == b.turn() - 1) {
            if (b.true_rand()%2 == 0) {
		turn(b,s)["Failed"] = true;
	    } else {
		poke(b,s)["ProtectiveMoveTurn"] = b.turn();
	    }
	} else {
	    poke(b,s)["ProtectiveMoveTurn"] = b.turn();
	}
    }

    static void uas(int s, int, BS &b) {
	addFunction(b.battlelong, "DetermineGeneralAttackFailure", "Detect", &dgaf);
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
        if (poke(b,s).value("HasMovedOnce").toBool()) {
	    turn(b,s)["Failed"] = true;
	}
    }
};

struct MMDreamingTarget : public MM
{
    MMDreamingTarget() {
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
	if (b.poke(t).status() != Pokemon::Asleep || b.hasSubstitute(t)) {
            b.fail(s, 31, 0, type(b,s), t);
	}
    }
};

struct MMHiddenPower : public MM
{
    MMHiddenPower() {
	functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        int *dvs = fpoke(b,s).dvs;

        int type = HiddenPowerInfo::Type(dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);
        tmove(b, s).type = type;
        tmove(b, s).power = HiddenPowerInfo::Power(dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);

        /* In 3rd gen, hidden powers can be physical! */
        if (b.gen() <= 3) {
            tmove(b, s).category = TypeInfo::Category(type);
        }
    }
};

struct MMFaintUser : public MM
{
    MMFaintUser() {
        functions["BeforeEnding"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (!turn(b,s).value("FaintActivationPrevented").toBool() && !b.koed(s))
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
	    turn(b, s)["Failed"] = true;
	}
    }
};

struct MMOHKO : public MM
{
    MMOHKO() {
	functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.inflictDamage(t, b.poke(t).totalLifePoints(), s, true);
        if (b.koed(t)) {
            b.sendMoveMessage(43,1,s,type(b,s));
        }
    }

    static void daf(int s, int t, BS &b) {
        if (b.OHKOClause()) {
            turn(b,s)["Failed"] = true;
            b.notifyClause(ChallengeInfo::OHKOClause);
        }
	if (b.poke(s).level() < b.poke(t).level()) {
	    turn(b,s)["Failed"] = true;
            return;
	}
        if (b.gen() <= 4 && b.hasWorkingAbility(t, Ability::Sturdy)) {
            b.fail(s,43,0,type(b,s));
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

struct MMTrumpCard : public MM
{
    MMTrumpCard() {
        functions["BeforeTargetList"] = &bcd;
    }

    static void bcd(int s, int, BS &b)
    {
	int n = b.poke(s).move(poke(b,s)["MoveSlot"].toInt()).PP();
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

struct MMSuperFang : public MM
{
    MMSuperFang() {
	functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.inflictDamage(t,b.poke(t).lifePoints()/2, s, true);
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
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        tmove(b,s).targets = Move::User;
    }

    static void uas(int s, int t, BS &b) {
        for (int t = 0; t < b.numberOfSlots(); t++) {
            if (poke(b,t).contains("PerishSongCount") || b.koed(t)) {
                return;
            }
            addFunction(poke(b,t), "EndTurn8", "PerishSong", &et);
            poke(b, t)["PerishSongCount"] = tmove(b,s).minTurns + (b.true_rand() % (tmove(b,s).maxTurns+1-tmove(b,s).maxTurns));
        }
        b.sendMoveMessage(95);
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;

	int count = poke(b,s)["PerishSongCount"].toInt();
        //SoundProof
        if (!b.hasWorkingAbility(s,Ability::Soundproof))
            b.sendMoveMessage(95,1,s,0,0,count);
	if (count > 0) {
	    poke(b,s)["PerishSongCount"] = count - 1;
        } else if (!b.hasWorkingAbility(s,Ability::Soundproof)){ //SoundProof
	    b.koPoke(s,s,false);
	}
    }
};

struct MMHaze : public MM
{
    MMHaze() {
        functions["BeforeTargetList"] = &btl;
	functions["UponAttackSuccessful"] = &uas;
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

    static void uas(int , int , BS &b) {
        b.sendMoveMessage(149);

        foreach (int p, b.sortedBySpeed())
        {
            for (int i = 1; i <= 7; i++) {
                fpoke(b,p).boosts[i] = 0;
            }
        }
    }
};

struct MMLeechSeed : public MM
{
    MMLeechSeed() {
	functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
	if (b.hasType(t, Pokemon::Grass) || (poke(b,t).contains("SeedSource"))) {
            b.fail(s, 72,0,Pokemon::Grass,t);
	}
    }

    static void uas(int s, int t, BS &b) {
        addFunction(poke(b,t), "EndTurn64", "LeechSeed", &et);
	poke(b,t)["SeedSource"] = s;
	b.sendMoveMessage(72, 1, s, Pokemon::Grass, t);
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s) || b.hasWorkingAbility(s, Ability::MagicGuard))
	    return;
        int s2 = poke(b,s)["SeedSource"].toInt();
        if (b.koed(s2))
            return;

        int damage = std::min(int(b.poke(s).lifePoints()), std::max(b.poke(s).totalLifePoints() / 8, 1));
        
	b.sendMoveMessage(72, 2, s, Pokemon::Grass);
	b.inflictDamage(s, damage, s, false);

	if (b.koed(s2))
	    return;
        if (!b.hasWorkingAbility(s, Ability::LiquidOoze))
            b.healLife(s2, damage);
        else {
            b.sendMoveMessage(1,2,s2,Pokemon::Poison,s);
            b.inflictDamage(s2, damage,s2,false);
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
            turn(b,s)["Failed"] = true;
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(150,0,s,Pokemon::Flying);

        poke(b,s)["Roosted"] = true;
	addFunction(poke(b,s), "EndTurn", "Roost", &et);
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
        if (b.poke(s).status() == Pokemon::Asleep || b.poke(s).isFull() || b.hasWorkingAbility(s,Ability::Insomnia)
            || b.hasWorkingAbility(s,Ability::VitalSpirit) || b.isThereUproar()) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	b.healLife(s, b.poke(s).totalLifePoints());
        b.sendMoveMessage(106,0,s,type(b,s));
        b.changeStatus(s, Pokemon::Asleep,false);
        b.poke(s).statusCount() = 2;
        poke(b,s)["Rested"] = true;
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
	}
    }
    static void uas(int s, int, BS &b) {
        if (move(b,s) == Move::BellyDrum) {
            b.sendMoveMessage(8,1,s,type(b,s));
            b.inflictStatMod(s,Attack,12, s, false);
        }
        b.changeHp(s, b.poke(s).lifePoints() - std::max(b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100,1));
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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
        slot(b,s)["WishTurn"] = b.turn() + 1;
        slot(b,s)["Wisher"] = b.poke(s).nick();
        addFunction(slot(b,s), "EndTurn2", "Wish", &et);
    }

    static void et(int s, int, BS &b) {
        int turn = slot(b,s)["WishTurn"].toInt();
        if (turn != b.turn()) {
	    return;
	}
        if (!b.koed(s)) {
            b.sendMoveMessage(142, 0, 0, 0, 0, 0, slot(b,s)["Wisher"].toString());
	    b.healLife(s, b.poke(s).totalLifePoints()/2);
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
            turn(b,s)["Failed"] = true;
    }

    static void uas (int s, int t, BS &b) {
        b.link(s, t, "Blocked");
	b.sendMoveMessage(12, 0, s, type(b,s), t);
    }
};

struct MMIngrain : public MM
{
    MMIngrain() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int , BS &b) {
	if (poke(b,s)["Rooted"].toBool() == true) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	poke(b,s)["Rooted"] = true;
	b.sendMoveMessage(151,0,s,Pokemon::Grass);
        addFunction(poke(b,s), "EndTurn60", "Ingrain", &et);
    }

    static void et(int s, int, BS &b) {
	if (!b.koed(s) && !b.poke(s).isFull() && poke(b,s)["Rooted"].toBool() == true) {
	    b.healLife(s, b.poke(s).totalLifePoints()/16);
	    b.sendMoveMessage(151,1,s,Pokemon::Grass);
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
                    turn(b,s)["Failed"] = true;
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
        return;
    }

    static void aaf(int s, int, BS &b) {
        int t = turn(b,s)["Target"].toInt();
        if (!turn(b,s).contains("RoarSuccess"))
            return;

	QList<int> switches;
        int target = b.player(t);
	for (int i = 0; i < 6; i++) {
            if (!b.isOut(target, i) && !b.poke(target,i).ko()) {
		switches.push_back(i);
	    }
	}
        b.sendBack(t, true);
        b.sendPoke(t, switches[b.true_rand()%switches.size()], true);
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
        if (team(b,b.opponent(b.player(s))).value("Spikes").toInt() >= 3) {
	    turn(b,s)["Failed"] = true;
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
	int n = (spikeslevel+1);
        b.sendMoveMessage(121,1,slot);
        b.inflictDamage(slot, b.poke(slot).totalLifePoints()*n/16, slot);
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
	    turn(b,s)["Failed"] = true;
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
            int n = TypeInfo::Eff(Pokemon::Rock, b.getType(s, 1)) * TypeInfo::Eff(Pokemon::Rock, b.getType(s, 2));
	    b.inflictDamage(s, b.poke(s).totalLifePoints()*n/32, s);
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
	    turn(b,s)["Failed"] = true;
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
	if (b.hasSubstitute(s) || b.isFlying(s)) {
	    return;
	}
        if (team(b,source).value("SafeGuardCount").toInt() > 0) {
	    return;
	}
        if (b.ability(source) == Ability::MagicGuard) {
            return;
        }

        int spikeslevel = team(b,source).value("ToxicSpikes").toInt();

	switch (spikeslevel) {
         case 0: return;
         case 1: b.inflictStatus(s, Pokemon::Poisoned, s); break;
         default: b.inflictStatus(s, Pokemon::Poisoned, s, 15, 15); break;
        }
    }
};

struct MMRapidSpin : public MM
{
    MMRapidSpin() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        if (poke(b,s).contains("SeedSource")) {
            b.sendMoveMessage(103,1,s);
            poke(b,s).remove("SeedSource");
            removeFunction(poke(b,s), "EndTurn64", "LeechSeed");
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
	if (poke(b,s).value("Substitute").toBool() == true) {
	    b.fail(s, 128);
	}
    }

    static void uas(int s, int, BS &b) {
	poke(b,s)["Substitute"] = true;
	poke(b,s)["SubstituteLife"] = b.poke(s).totalLifePoints()/4;
        b.sendMoveMessage(128,4,s);
        b.notifySub(s,true);
//	addFunction(poke(b,s), "BlockTurnEffects", "Substitute", &bte);
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
	if (turn(b,s).contains("DamageTakenByAttack")) {
	    b.fail(s,47,0,Pokemon::Fighting);
	}
    }

    static void os(int s, int, BS &b) {
	b.sendMoveMessage(47,1,s,Pokemon::Fighting);
    }
};

struct MMNightShade : public MM
{
    MMNightShade() {
	functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.inflictDamage(t, fpoke(b,s).level, s, true);
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
            if (!b.poke(player,i).ko() && (move == Aromatherapy || b.poke(player,i).ability() != Ability::Soundproof)) {
                b.changeStatus(player,i,Pokemon::Fine);
	    }
	}
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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas (int s, int t, BS &b) {
	b.sendMoveMessage(58,1,s,0,t);
        if (b.hasWorkingItem(s, Item::MentalHerb)) /* mental herb*/ {
	    b.sendItemMessage(7,s);
	    b.disposeItem(t);
        } else {
            b.link(s, t, "Attract");
            addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);
        }
    }

    static void pda(int s, int, BS &b) {
        if (turn(b,s).value("HasPassedStatus").toBool())
            return;
        if (b.linked(s, "Attract")) {
            int seducer = poke(b,s)["AttractBy"].toInt();

            b.sendMoveMessage(58,0,s,0,seducer);
            if (b.true_rand() % 2 == 0) {
                turn(b,s)["ImpossibleToMove"] = true;
                b.sendMoveMessage(58, 2,s);
            }
	}
    }
};

struct MMKnockOff : public MM
{
    MMKnockOff() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s,int t,BS &b)
    {
        if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, Ability::StickyHold) && !b.hasWorkingAbility(t, Ability::Multitype)
            && b.poke(t).num().pokenum != Pokemon::Giratina) /* Sticky Hold, MultiType, Giratina-O */
	{
	    b.sendMoveMessage(70,0,s,type(b,s),t,b.poke(t).item());
            b.loseItem(t);
            b.battlelong[QString("KnockedOff%1%2").arg(b.player(t)).arg(b.currentInternalId(t))] = true;
	}
    }
};

struct MMCovet : public MM
{
    MMCovet() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s,int t,BS &b)
    {
        if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, Ability::StickyHold)
            && b.ability(t) != Ability::Multitype && !b.hasWorkingAbility(s, Ability::Multitype)
            && b.pokenum(s).pokenum != Pokemon::Giratina && b.poke(s).item() == 0
                    && b.pokenum(t).pokenum != Pokemon::Giratina && !ItemInfo::isMail(b.poke(t).item())) /* Sticky Hold, MultiType, Giratina_O, Mail*/
        {
            b.sendMoveMessage(23,(move(b,s)==Covet)?0:1,s,type(b,s),t,b.poke(t).item());
	    b.acqItem(s, b.poke(t).item());
            b.loseItem(t);
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
        if (b.koed(t) || (b.poke(t).item() == 0 && b.poke(s).item() == 0) || b.hasWorkingAbility(t, Ability::StickyHold)
            || b.ability(t) == Ability::Multitype || b.pokenum(s).pokenum == Pokemon::Giratina || b.pokenum(t).pokenum == Pokemon::Giratina
                    || ItemInfo::isMail(b.poke(s).item()) || ItemInfo::isMail(b.poke(t).item()))
            /* Sticky Hold, MultiType, Giratina-O, Mail */
            {
	    turn(b,s)["Failed"] = true;
	}
        /* Knock off */
        if (b.battlelong.value(QString("KnockedOff%1%2").arg(b.player(t)).arg(b.currentInternalId(t))).toBool()
            || b.battlelong.value(QString("KnockedOff%1%2").arg(b.player(t)).arg(b.currentInternalId(t))).toBool()) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b)
    {
	b.sendMoveMessage(132,0,s,type(b,s),t);
	int i1(b.poke(s).item()), i2(b.poke(t).item());
	b.acqItem(s, i2);
	b.acqItem(t, i1);

        if (i2)
            b.sendMoveMessage(132,1,s,type(b,s),t,i2);
        if (i1)
            b.sendMoveMessage(132,1,t,type(b,s),s,i1);
    }
};

struct MMDragonRage : public MM
{
    MMDragonRage() {
	functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.inflictDamage(t, turn(b,s)["DragonRage_Arg"].toInt(), s, true);
    }
};


struct MMCopycat : public MM
{
    MMCopycat() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	/* First check if there's even 1 move available */
        if (!b.battlelong.contains("LastMoveSuccessfullyUsed") || b.battlelong["LastMoveSuccessfullyUsed"].toInt() == Copycat) {
	    turn(b,s)["Failed"] = true;
	} else {
	    turn(b,s)["CopycatMove"] = b.battlelong["LastMoveSuccessfullyUsed"];
	}
    }

    static void uas(int s, int t, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "Copycat");
        removeFunction(turn(b,s), "DetermineAttackFailure", "Copycat");
	int attack = turn(b,s)["CopycatMove"].toInt();
	MoveEffect::setup(attack, s, t, b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
	b.useAttack(s, turn(b,s)["CopycatMove"].toInt(), true);
        MoveEffect::unsetup(attack, s, b);
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
            (*this) << NoMove << Assist << Chatter << Copycat << Counter << Covet << DestinyBond
                    << Detect << Endure << Feint << FocusPunch << FollowMe << HelpingHand << MeFirst
                    << Metronome << Mimic << MirrorCoat << MirrorMove << Protect << Sketch << SleepTalk
                    << Snatch << Struggle << Switcheroo << Thief << Trick;
	}
    };
    static FM forbidden_moves;

    static void uas(int s, int, BS &b)
    {
	removeFunction(turn(b,s), "UponAttackSuccessful", "Assist");
	removeFunction(turn(b,s), "DetermineAttackFailure", "Assist");
	int attack = turn(b,s)["AssistMove"].toInt();
        MoveEffect::setup(attack, s, s, b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
	b.useAttack(s, turn(b,s)["AssistMove"].toInt(), true);
        MoveEffect::unsetup(attack, s, b);
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
		    if (!forbidden_moves.contains(m))
                        possible_moves.push_back(m);
		}
	    }
	}
	if (!possible_moves.empty()) {
            turn(b,s)["AssistMove"] = *(possible_moves.begin() + (b.true_rand() %possible_moves.size()));
	} else {
	    turn(b,s)["Failed"] = true;
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
	    turn(b,s)["Failed"] = true;
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
        }
    }

    static void ts(int s, int, BS &b) {
	int _turn = poke(b,s)["BideTurn"].toInt();
	if (_turn + 2 < b.turn()) {
	    return;
        }

        addFunction(turn(b,s),"UponOffensiveDamageReceived", "Bide", &udi);
        tmove(b,s).priority = 1;
        turn(b,s)["NoChoice"] = true;
    }

    static void ccd(int s, int t, BS &b) {
	b.inflictDamage(t, 2*poke(b,s)["BideDamageCount"].toInt(),s,true);
    }
};

struct MMBind : public MM
{
    MMBind() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
        b.link(s, t, "Trapped");
        BS::BasicMoveInfo &fm = tmove(b,s);
        poke(b,t)["TrappedRemainingTurns"] = b.poke(s).item() == Item::GripClaw ?
                                             fm.maxTurns - 1 : (b.true_rand()%fm.maxTurns+1-fm.minTurns) + fm.minTurns - 1; /* Grip claw = max turns */
	poke(b,t)["TrappedMove"] = move(b,s);
        addFunction(poke(b,t), "EndTurn68", "Bind", &et);
    }

    static void et (int s, int, BS &b) {
        int count = poke(b,s)["TrappedRemainingTurns"].toInt() - 1;
        int move = poke(b,s)["TrappedMove"].toInt();

        if (!b.koed(s)) {
            if (!b.linked(s, "Trapped")) {
                poke(b,s).remove("TrappedBy");
                removeFunction(poke(b,s),"EndTurn68", "Bind");
                return;
            }
            if (count <= 0) {
                poke(b,s).remove("TrappedBy");
                removeFunction(poke(b,s),"EndTurn68", "Bind");
                if (count == 0)
                    b.sendMoveMessage(10,1,s,MoveInfo::Type(move, b.gen()),s,move);
            } else {
                poke(b,s)["TrappedRemainingTurns"] = count;
                b.sendMoveMessage(10,0,s,MoveInfo::Type(move, b.gen()),s,move);
                b.inflictDamage(s, b.poke(s).totalLifePoints()/16,s,false);
            }
        }
    }
};

struct MMBounce : public MM
{
    MMBounce() {
	functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        QStringList args = turn(b,s)["Bounce_Arg"].toString().split('_');
        b.sendMoveMessage(13,args[0].toInt(),s,type(b,s));

        if (b.hasWorkingItem(s, Item::PowerHerb)) {
            b.sendItemMessage(11,s);
            b.disposeItem(s);

            removeFunction(turn(b,s), "UponAttackSuccessful", "Bounce");
            if (move(b,s) == ShadowForce) {
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &uas2);
            }
        } else {
            tmove(b, s).power = 0;
            tmove(b, s).accuracy = 0;
            tmove(b, s).targets = Move::User;
            poke(b,s)["2TurnMove"] = move(b,s);
            turn(b,s)["TellPlayers"] = false;
        }
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s)["Invulnerable"].toBool()) {
            turn(b,s)["NoChoice"] = true;

            int move = poke(b,s)["2TurnMove"].toInt();

            MoveEffect(move, b.gen(),b.fieldmoves[s]);
            addFunction(turn(b,s), "EvenWhenCantMove", "Bounce", &ewc);

            if (move == ShadowForce) {
                addFunction(turn(b,s), "UponAttackSuccessful", "Bounce", &uas2);
            }
        }
        removeFunction(poke(b,s), "TurnSettings", "Bounce");
    }

    /* Only called with Shadow Force, breaks protect */
    static void uas2(int , int t, BS &b) {
        if (turn(b, t).value("DetectUsed").toBool() == true) {
            turn(b, t)["DetectUsed"] = false;
        }
    }

    static void ewc(int s, int, BS &b) {
	poke(b,s)["Invulnerable"] = false;
        b.changeSprite(s, 0);
    }

    static void uas(int s, int, BS &b) {
	QStringList args = turn(b,s)["Bounce_Arg"].toString().split('_');
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
    }

    static void dgaf(int s, int t, BS &b) {
	if (s == t || t == -1) {
	    return;
	}
        if (!poke(b,s).value("Invulnerable").toBool()) {
	    return;
	}
        int attack = move(b,t);
	/* Lets see if the poke is vulnerable to that one attack */
        QList<int> vuln_moves = poke(b,s)["VulnerableMoves"].value<QList<int> >();
        QList<int> vuln_mults = poke(b,s)["VulnerableMults"].value<QList<int> >();

	for (int i = 0; i < vuln_moves.size(); i++) {
	    if (vuln_moves[i] == attack) {
                tmove(b, s).power = tmove(b, s).power * vuln_mults[i];
		return;
	    }
	}

	/* All other moves fail */
        turn(b,s)["EvadeAttack"] = true;
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
        if (b.gen() <= 3 && TypeInfo::Category(type(b, source)) != turn(b,s)["Counter_Arg"].toInt()) {
            return;
        }

        if (turn(b, s)["DamageTakenByAttack"].toInt() <= 0) {
            return;
        }

        turn(b,s)["CounterDamage"] = 2 * turn(b,s)["DamageTakenByAttack"].toInt();
        turn(b,s)["CounterTarget"] = source;
    }

    static void ms (int s, int, BS &b) {
        turn(b,s)["Target"] = turn(b,s)["CounterTarget"];
        tmove(b,s).targets = Move::ChosenTarget;
    }

    static void daf (int s, int, BS &b) {
        if (!turn(b,s).contains("CounterDamage"))
            turn(b,s)["Failed"] = true;
    }

    static void cad(int s, int t, BS &b) {
	b.inflictDamage(t, turn(b,s)["CounterDamage"].toInt(), s, true);
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
        int dam = turn(b,s).value("DamageTakenByAttack").toInt();
        if (dam == 0) {
            turn(b,s)["Failed"] = true;
            return;
        }
        turn(b,s)["CounterDamage"] = dam * 3 / 2;
    }

    static void cad(int s, int t, BS &b) {
        b.inflictDamage(t, turn(b,s)["CounterDamage"].toInt(), s, true);
    }
};

struct MMTaunt : public MM
{
    MMTaunt() {
        functions["OnFoeOnAttack"] = &uas;
        functions["DetermineAttackFailure"]=  &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (poke(b,t)["TauntsUntil"].toInt() >= b.turn())
            turn(b,s)["Failed"] = true;
    }

    static void uas (int s, int t, BS &b) {
        if (b.gen() <= 3) {
            poke(b,t)["TauntsUntil"] = b.turn() + 1;
        } else if (b.gen() == 4) {
            poke(b,t)["TauntsUntil"] = b.turn() + 2 + (b.true_rand()%3);
        } else {
            poke(b,t)["TauntsUntil"] = b.turn() + 2;
        }

	addFunction(poke(b,t), "MovesPossible", "Taunt", &msp);
        addFunction(poke(b,t), "MovePossible", "Taunt", &mp);
        addFunction(poke(b,t), "EndTurn611", "Taunt", &et);
	b.sendMoveMessage(134,1,s,Pokemon::Dark,t);
    }

    static void et(int s, int, BS &b)
    {
        if (b.koed(s))
            return;
        int tt = poke(b,s)["TauntsUntil"].toInt();
        if (tt == b.turn()) {
            poke(b,s).remove("TauntsUntil");
            removeFunction(poke(b,s), "MovesPossible", "Taunt");
            removeFunction(poke(b,s), "MovePossible", "Taunt");
            removeFunction(poke(b,s), "EndTurn611", "Taunt");
            if (b.gen() >= 4)
                b.sendMoveMessage(134,2,s,Pokemon::Dark);
        }
    }

    static void msp(int s, int, BS &b) {
	int tt = poke(b,s)["TauntsUntil"].toInt();
	if (tt < b.turn()) {
	    return;
	}
	for (int i = 0; i < 4; i++) {
            if (MoveInfo::Power(b.move(s,i), b.gen()) == 0) {
		turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
	    }
	}
    }

    static void mp(int s, int, BS &b) {
	int tt = poke(b,s)["TauntsUntil"].toInt();
	if (tt < b.turn()) {
	    return;
	}
	int move = turn(b,s)["MoveChosen"].toInt();
        if (MoveInfo::Power(move, b.gen()) == 0) {
	    turn(b,s)["ImpossibleToMove"] = true;
	    b.sendMoveMessage(134,0,s,Pokemon::Dark,s,move);
	}
    }
};

struct MMDisable : public MM
{
    MMDisable() {
	functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
	if (poke(b,t).contains("DisablesUntil") && poke(b,t).value("DisablesUntil").toInt() >= b.turn())
	{
	    turn(b,s)["Failed"] = true;
	    return;
	}
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int move = poke(b,t)["LastMoveUsed"].toInt();
    	int sl = -1;
	for (int i = 0; i < 4; i++) {
	    if (b.move(t, i) == move) {
		sl = i;
	    }
	}
	if (sl == -1 || b.poke(s).move(sl).PP() == 0 ) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas (int s, int t, BS &b) {
        int mv = poke(b,t)["LastMoveUsed"].toInt();
        poke(b,t)["DisablesUntil"] = b.turn() + 3 + (b.true_rand()%4);
        poke(b,t)["DisabledMove"] = mv;
	addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
        addFunction(poke(b,t), "MovePossible", "Disable", &mp);
        addFunction(poke(b,t), "EndTurn611", "Disable", &et);
        b.sendMoveMessage(28,0,s,0,t,mv);
    }

    static void et (int s, int, BS &b)
    {
	int tt = poke(b,s)["DisablesUntil"].toInt();
	if (tt <= b.turn()) {
	    removeFunction(poke(b,s), "MovesPossible", "Disable");
            removeFunction(poke(b,s), "MovePossible", "Disable");
            removeFunction(poke(b,s), "EndTurn611", "Disable");
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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void cad(int s, int t, BS &b) {
	int move = MM::move(b,s);
	turn(b,s)["CriticalHit"] = false;
        tmove(b, s).power = tmove(b, s).power * MoveInfo::Power(move, b.gen());
        slot(b,t)["DoomDesireDamage"] = b.calculateDamage(s, t);
        slot(b,t)["DoomDesireTurn"] = b.turn() + 2;
        slot(b,t)["DoomDesireMove"] = move;
        tmove(b, s).accuracy = MoveInfo::Acc(move, b.gen());
        slot(b,t)["DoomDesireFailed"] = !b.testAccuracy(s,t,true);
        addFunction(slot(b,t), "EndTurn7", "DoomDesire", &et);
        b.sendMoveMessage(29, move==DoomDesire?2:1, s, type(b,s));
    }

    static void et (int s, int, BS &b) {
        if (b.turn() == slot(b,s).value("DoomDesireTurn"))
	{
            if (slot(b,s)["DoomDesireFailed"].toBool()) {
                int move = slot(b,s)["DoomDesireMove"].toInt();
                b.sendMoveMessage(29,0,s,MoveInfo::Type(move, b.gen()),s,move);
                b.notifyFail(s);
            } else if (!b.koed(s)) {
                int move = slot(b,s)["DoomDesireMove"].toInt();
                b.sendMoveMessage(29,0,s,MoveInfo::Type(move, b.gen()),s,move);
                b.inflictDamage(s,slot(b,s)["DoomDesireDamage"].toInt(), s, true, true);
	    }
            removeFunction(slot(b,s), "EndTurn7", "DoomDesire");
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
            turn(b,s)["Failed"] = true;
        else if (poke(b,s).contains("EmbargoEnd") && poke(b,s)["EmbargoEnd"].toInt() >= b.turn()) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
	b.sendMoveMessage(32,0,s,type(b,s),t);
	poke(b,t)["Embargoed"] = true;
	poke(b,t)["EmbargoEnd"] = b.turn() + 4;
        addFunction(poke(b,t), "EndTurn611", "Embargo", &et);
    }

    static void et(int s, int , BS &b) {
	if (poke(b,s).value("Embargoed").toBool() && poke(b,s)["EmbargoEnd"].toInt() <= b.turn()) {
	    b.sendMoveMessage(32,1,s,0);
            removeFunction(poke(b,s), "EndTurn611", "Embargo");
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

    static void daf(int s, int t, BS &b)
    {
	if (poke(b,t).contains("EncoresUntil") && poke(b,t).value("EncoresUntil").toInt() >= b.turn())
	{
	    turn(b,s)["Failed"] = true;
	    return;
	}
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        if (poke(b,t).contains("NoChoice")) {
            turn(b,s)["Failed"] = true;
            return;
        }
        int move = poke(b,t)["LastMoveUsed"].toInt();
        bool cont = forbidden_moves.contains(move);

        if (cont) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int sl = -1;
	for (int i = 0; i < 4; i++) {
	    if (b.move(t, i) == move) {
		sl = i;
	    }
	}
        if (sl == -1 || b.poke(t).move(sl).PP() == 0 ) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas (int s, int t, BS &b) {
        if (b.gen() <= 3)
            poke(b,t)["EncoresUntil"] = b.turn() + 1 + (b.true_rand()%5);
        else if (b.gen() == 4)
            poke(b,t)["EncoresUntil"] = b.turn() + 3 + (b.true_rand()%5);
        else
            poke(b,t)["EncoresUntil"] = b.turn() + 2;

        int mv =  poke(b,t)["LastMoveUsed"].toInt();
        poke(b,t)["EncoresMove"] = mv;

        /*Changes the encored move, if no choice is off (otherwise recharging moves like blast burn would attack again,
            and i bet something strange would also happen with charging move) */
        if (!turn(b,t).contains("NoChoice")) {
            for (int i = 0; i < 4; i ++) {
                if (b.move(t, i) == mv) {
                    MoveEffect::unsetup(move(b,t), t, b);
                    b.choice[t].numSwitch = i;
                    b.choice[t].targetPoke = b.randomValidOpponent(t);
                    MoveEffect::setup(mv, t, s, b);
                    break;
                }
            }
        }
	addFunction(poke(b,t), "MovesPossible", "Encore", &msp);
        addFunction(poke(b,t), "EndTurn611", "Encore", &et);
        b.sendMoveMessage(33,1,s,0,t);
    }

    static void et (int s, int, BS &b)
    {
        if (b.koed(s))
            return;
    	for (int i = 0; i < 4; i++) {
	    if (b.move(s,i) == poke(b,s)["EncoresMove"].toInt()) {
		if (b.poke(s).move(i).PP() <= 0) {
		    removeFunction(poke(b,s), "MovesPossible", "Encore");
                    removeFunction(poke(b,s), "EndTurn611", "Encore");
		    poke(b,s)["EncoresUntil"] = b.turn();
		    b.sendMoveMessage(33,0,s);
		    return;
		}
		break;
	    }
	}
	int tt = poke(b,s)["EncoresUntil"].toInt();
	if (tt <= b.turn()) {
	    removeFunction(poke(b,s), "MovesPossible", "Encore");
            removeFunction(poke(b,s), "EndTurn611", "Encore");
	    b.sendMoveMessage(33,0,s);
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
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void cad(int s, int t, BS &b) {
	b.inflictDamage(t, b.poke(t).lifePoints()-b.poke(s).lifePoints(),s,true);
    }
};

struct MMEndure : public MM
{
    MMEndure() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	if (poke(b,s).contains("ProtectiveMoveTurn") && poke(b,s)["ProtectiveMoveTurn"].toInt() == b.turn() - 1) {
            if (b.true_rand()%2 == 0) {
		turn(b,s)["Failed"] = true;
	    } else {
		poke(b,s)["ProtectiveMoveTurn"] = b.turn();
	    }
	} else {
	    poke(b,s)["ProtectiveMoveTurn"] = b.turn();
	}
    }

    static void uas(int s, int, BS &b) {
	turn(b,s)["CannotBeKoed"] = true;
        addFunction(turn(b,s), "UponSelfSurvival", "Endure", &uodr);
        b.sendMoveMessage(35,1,s);
    }

    static void uodr(int s, int, BS &b) {
	if (b.poke(s).lifePoints() == 1) {
	    b.sendMoveMessage(35,0,s);
	}
    }
};

struct MMFalseSwipe : public MM
{
    MMFalseSwipe() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }
    static void bcd(int s, int t, BS &b) {
        turn(b,t)["CannotBeKoedBy"] = s;
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
	poke(b,s)["FuryCutterCount"] = std::min(poke(b,s)["FuryCutterCount"].toInt() * 2 + 1,15);
    }

    static void bcd(int s, int, BS &b) {
        tmove(b, s).power = tmove(b, s).power * (poke(b,s)["FuryCutterCount"].toInt()+1);
    }
};

struct MMGastroAcid : public MM
{
    MMGastroAcid() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.ability(t) == Ability::Multitype)
            turn(b,s)["Failed"] = true;
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(51,0,s,type(b,s),t,b.ability(t));
        b.loseAbility(t);
        poke(b,t)["AbilityNullified"] = true;
    }
};

struct MMGravity : public MM
{
    MMGravity() {
        functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.battlelong.value("Gravity").toBool()) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int, BS &b) {
	b.battlelong["Gravity"] = true;
        b.battlelong["GravityCount"] = 5;
	b.sendMoveMessage(53,0,s,type(b,s));

        std::vector<int> list = b.sortedBySpeed();

        foreach(int p, list) {
            if (b.koed(p))
                continue;
            if (b.isFlying(p)) {
                b.sendMoveMessage(53,2,p,Type::Psychic);
            }
            if(poke(b,p).value("Invulnerable").toBool() && (poke(b,p)["2TurnMove"].toInt()==Move::Fly || poke(b,p)["2TurnMove"].toInt() == Move::Bounce)) {
                poke(b,p)["Invulnerable"] = false;
                b.changeSprite(p, 0);
                b.sendMoveMessage(53,3, p, Type::Psychic, s, poke(b,p)["2TurnMove"].toInt());
            }
        }
        addFunction(b.battlelong, "EndTurn5", "Gravity", &et);
        addFunction(b.battlelong, "MovesPossible", "Gravity", &msp);
        addFunction(b.battlelong, "MovePossible", "Gravity", &mp);
    }

    static void et(int s, int, BS &b) {
	if (b.battlelong.value("Gravity").toBool()) {
            int count = b.battlelong["GravityCount"].toInt() - 1;
	    if (count <= 0) {
		b.sendMoveMessage(53,1,s,Pokemon::Psychic);
                removeFunction(b.battlelong, "EndTurn5", "Gravity");
                removeFunction(b.battlelong, "MovesPossible", "Gravity");
		b.battlelong["Gravity"] = false;
            } else {
                b.battlelong["GravityCount"] = count;
            }
	}
    }

    struct FM : public QSet<int> {
        FM() {
            (*this) << Bounce << Fly << JumpKick << HiJumpKick << Splash << MagnetRise;
        }
    };
    static FM forbidden_moves;

    static void mp (int s, int, BS &b) {
        if (!b.battlelong.value("Gravity").toBool()) {
            return;
        }

        int mv = move(b,s);
        if(forbidden_moves.contains(mv)) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(53,4,s,Type::Psychic,s,mv);
        }
    }

    static void msp (int s, int , BS &b) {
        if (!b.battlelong.value("Gravity").toBool()) {
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

struct MMGrassKnot : public MM
{
    MMGrassKnot() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
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
        tmove(b, s).power = bp;
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

	if (trn == b.turn() || (trn+1 == b.turn() && !turn(b,s).value("HasMoved").toBool())) {
	    if (!b.koed(t) && !b.hasSubstitute(t)) {
		int slot = poke(b, t)["MoveSlot"].toInt();
		b.sendMoveMessage(54,0,s,Pokemon::Ghost,t,b.move(t,slot));
		b.losePP(t, slot, 48);
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

        tmove(b, s).power = bp;
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
            turn(b,s)["Failed"] = true;
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

MMWeather::WI MMWeather::weather_items;

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
            turn(b,s)["Failed"] = true;
        }
    }

    static void btl(int s, int, BS &b) {
        turn(b,s)["HealingWishSuccess"] = true;
    }

    static void aaf(int s, int, BS &b) {
        if (!turn(b,s).contains("HealingWishSuccess"))
            return;
        addFunction(turn(b,s), "AfterSwitchIn", "HealingWish", &asi);

        /* On gen 5 and further, the pokemon is switched at the end of the turn */
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
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
	poke(b,t)["HealBlockCount"] = 5;
        addFunction(poke(b,t), "EndTurn611", "HealBlock", &et);
	addFunction(turn(b,t), "MovePossible", "HealBlock", &mp);
	addFunction(poke(b,t), "MovesPossible", "HealBlock", &msp);
	b.sendMoveMessage(59,0,s,type(b,s),t);
    }
    static void et(int s, int , BS &b) {
	inc(poke(b,s)["HealBlockCount"], -1);
	int count = poke(b,s)["HealBlockCount"].toInt();

	if (count == 0) {
            b.sendMoveMessage(59,2,s,Type::Psychic);
            removeFunction(poke(b,s), "EndTurn611", "HealBlock");
	    removeFunction(poke(b,s), "MovesPossible", "HealBlock");
	}
    }

    static void msp(int s, int, BS &b) {
	for (int i = 0; i < 4; i++) {
            if (MoveInfo::Flags(b.move(s, i), b.gen()) & Move::HealingFlag) {
		turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
	    }
	}
    }

    static void mp(int s, int, BS &b) {
        int mv = move(b,s);
        if(tmove(b,s).flags & Move::HealingFlag) {
	    turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(59,1,s,Type::Psychic,s,mv);
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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void btl(int s, int, BS &b) {
        if (b.poke(s).item() != 0 && b.hasWorkingItem(s, b.poke(s).item()) && ItemInfo::Power(b.poke(s).item()) > 0) {
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
                turn(b,t)["Flinched"] = true; /* king rock, razor fang */
            } else if (!team(b, b.player(t)).contains("SafeGuardCount"))  {
                switch (item) {
                    case Item::FlameOrb: b.inflictStatus(t, Pokemon::Burnt, s); break; /*flame orb*/
                case Item::ToxicOrb: b.inflictStatus(t, Pokemon::Poisoned, s, 15, 15); break; /*toxic orb*/
                    case Item::LightBall: b.inflictStatus(t, Pokemon::Paralysed, s); break; /* light ball */
                    case Item::PoisonBarb: b.inflictStatus(t, Pokemon::Poisoned, s); break; /* poison barb */
                }
            }
        } else {
            int sitem = b.poke(t).item();

            /* Setting up the conditions so berries work properly */
            turn(b,t)["BugBiter"] = true; // for testPinch of pinch berries to return true
            QVariant tempItemStorage = poke(b,t)["ItemArg"];
            poke(b,t)["ItemArg"] = poke(b,s)["ItemArg"];

            b.sendMoveMessage(16,0,t,type(b,s),t,item);

            /* Finding the function to call :P */
            QList<ItemInfo::Effect> l = ItemInfo::Effects(item, b.gen());

            foreach(ItemInfo::Effect e, l) { /* Ripped from items.cpp (ItemEffect::activate, with some changes) */
                if (!ItemEffect::mechanics.contains(e.num)) {
                    continue;
                }
                foreach (function f, ItemEffect::mechanics[e.num].functions) {
                    f(t, s, b);
                    if (b.poke(t).item() == 0)
                        break;
                }
            }

            /* Restoring initial conditions */
            poke(b,t)["ItemArg"] = tempItemStorage;
            turn(b,t).remove("BugBiter");

            /* in case the item was "eaten" as a berry */
            b.poke(t).item() = sitem;
        }
    }
};

struct MMJumpKick : public MM
{
    MMJumpKick() {
	functions["AttackSomehowFailed"] = &asf;
    }

    static void asf(int s, int t, BS &b) {
	int typemod;
	int typeadv[] = {b.getType(t, 1), b.getType(t, 2)};
	int type = MM::type(b,s);
	if (typeadv[0] == Pokemon::Ghost) {
            if (b.gen() <= 3)
                return;
	    typemod = TypeInfo::Eff(type, typeadv[1]);
	} else if (typeadv[1] == Pokemon::Ghost) {
            if (b.gen() <= 3)
                return;
	    typemod = TypeInfo::Eff(type, typeadv[0]);
	} else {
	    typemod = TypeInfo::Eff(type, typeadv[0]) * TypeInfo::Eff(type, typeadv[1]);
	}
	turn(b,s)["TypeMod"] = typemod;
        turn(b,s)["Stab"] = b.hasType(s, Type::Fighting) ? 3 : 2;
        int damage;
        if (b.gen() >= 4)
            damage = std::min(b.calculateDamage(s,t)/2, b.poke(t).totalLifePoints()/2);
        else
            damage = std::min(b.calculateDamage(s,t)/8, b.poke(t).totalLifePoints()/2);
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
	    turn(b,s)["NoChoice"] = true;
            MoveEffect::setup(poke(b,s)["LastSpecialMoveUsed"].toInt(),s,t,b);
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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	addFunction(b.battlelong, "MovePossible", "Imprison", &mp);
	addFunction(b.battlelong, "MovesPossible", "Imprison", &msp);
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
        if (b.hasWorkingAbility(s,Ability::Levitate) || poke(b,s).value("Rooted").toBool()) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(68,0,s,Pokemon::Electric);
	poke(b,s)["MagnetRiseCount"] = 5;
        addFunction(poke(b,s), "EndTurn611", "MagnetRise", &et);
    }

    static void et(int s, int, BS &b) {
	inc(poke(b,s)["MagnetRiseCount"], -1);
	int count = poke(b,s)["MagnetRiseCount"].toInt();

        if (count == 0 && !b.koed(s)) {
            b.sendMoveMessage(68,1,s, Type::Electric);
            removeFunction(poke(b,s), "EndTurn611", "MagnetRise");
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
	if (ItemInfo::isPlate(item) && b.hasWorkingItem(s, item)) {
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
	    turn(b,s)["Failed"] = true;
	    return;
	}
	bool succ = true;
	int slot = poke(b,s)["MoveSlot"].toInt();
	for (int i = 0; i < 4; i++) {
	    if (i != slot && b.move(s,i) != 0 && !poke(b,s).value(QString("Move%1Used").arg(i)).toBool()) {
		succ= false;
	    }
	}
	if (!succ) {
	    turn(b,s)["Failed"] = true;
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
        int cat = turn(b,s)["TeamBarrier_Arg"].toInt();
        int source = b.player(s);

        if (team(b,source).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int, BS &b) {
        int source = b.player(s);

	int nturn;
        if (b.hasWorkingItem(s, Item::LightClay)) { /* light clay */
	    nturn = 8;
	} else {
	    nturn = 5;
	}
	int cat = turn(b,s)["TeamBarrier_Arg"].toInt();

        b.sendMoveMessage(73,(cat-1)+b.doubles()*2,s,type(b,s));
        team(b,source)["Barrier" + QString::number(cat) + "Count"] = nturn;

        addFunction(team(b,source), "EndTurn", "TeamBarrier", &et);
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
};

struct MMBrickBreak : public MM
{
    MMBrickBreak() {
	functions["BeforeHitting"] = &bh;
    }
    static void bh(int s, int t, BS &b) {
        int opp = b.player(t);
        if (team(b,opp).value("Barrier0Count").toInt() > 0 || team(b,opp).value("Barrier1Count").toInt() > 0) {
	    b.sendMoveMessage(14,0,s,Pokemon::Fighting);
            team(b,opp)["Barrier0Count"] = 0;
            team(b,opp)["Barrier1Count"] = 0;
	}
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

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(75,0,s,type(b,s));

        int source = b.player(s);

        team(b,source)["LuckyChantCount"] = 5;
        addFunction(team(b,source), "EndTurn", "LuckyChant", &et);
    }

    static void et(int s, int, BS &b) {
	inc(team(b,s)["LuckyChantCount"], -1);
	int count = team(b,s)["LuckyChantCount"].toInt();

	if (count == 0) {
	    b.sendMoveMessage(75,1,s);
	    removeFunction(team(b,s), "EndTurn", "LuckyChant");
	}
    }
};

struct MMMagicCoat : public MM
{
    MMMagicCoat() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        addFunction(b.battlelong, "DetermineGeneralAttackFailure2", "MagicCoat", &dgaf);
	turn(b,s)["MagicCoated"] = true;
	b.sendMoveMessage(76,0,s,Pokemon::Psychic);
    }

    static void dgaf(int s, int t, BS &b) {
        bool bounced = tmove(b, s).flags & Move::MagicCoatableFlag;
        if (!bounced)
            return;
        /* Don't double bounce something */
        if (b.battlelong.contains("CoatingAttackNow")) {
            return;
        }
        int target = -1;
        if (t != s && (turn(b,t).value("MagicCoated").toBool() || b.hasWorkingAbility(t, Ability::MagicMirror)) ) {
            target = t;
        } else {
            /* Entry hazards */
            foreach(int t, b.revs(s)) {
                if (turn(b,t).value("MagicCoated").toBool() || b.hasWorkingAbility(t, Ability::MagicMirror)) {
                    target = t;
                    break;
                }
            }
        }

        if (target == -1)
            return;

        int move = MM::move(b,s);

        b.fail(s,76,b.hasWorkingAbility(t, Ability::MagicMirror) ? 2 : 1,Pokemon::Psychic);
        /* Now Bouncing back ... */
        BS::context ctx = turn(b,target);
        BS::BasicMoveInfo info = tmove(b,s);

        turn(b,target).clear();
        MoveEffect::setup(move,target,s,b);
        turn(b,target)["Target"] = s;
        b.battlelong["CoatingAttackNow"] = true;
        b.useAttack(target,move,true,false);
        b.battlelong.remove("CoatingAttackNow");

        /* Restoring previous state. Only works because moves reflected don't store useful data in the turn memory,
            and don't cause any such data to be stored in that memory */
        turn(b,target) = ctx;
        tmove(b,s) = info;
    }
};

struct MMDefog : public MM
{
    MMDefog() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
        bool clear = false;


        BS::context &c = team(b,t);

        if (c.contains("Barrier0Count") || c.contains("Barrier1Count") || c.contains("Spikes") || c.contains("ToxicSpikes")
            || c.contains("StealthRock") || c.contains("MistCount") || c.contains("SafeGuardCount")) {
            clear = true;

            c.remove("Barrier0Count");
            c.remove("Barrier1Count");
            c.remove("Spikes");
            c.remove("ToxicSpikes");
            c.remove("StealthRock");
            c.remove("MistCount");
            c.remove("SafeGuardCount");
        }

        if (clear) {
            b.sendMoveMessage(77,0, s, type(b,s), t);
        }
    }
};

struct MMMagnitude: public MM
{
    MMMagnitude() {
        functions["BeforeTargetList"] = &bcd;
	functions["BeforeHitting"] = &bh;
    }

    static void bcd(int s, int, BS &b) {
        int randnum = b.true_rand()%20;

	int pow, magn;

	switch (randnum) {
         case 0: magn = 4; pow = 10; break;
         case 1: case 2: magn = 5; pow = 30; break;
         case 3: case 4: case 5: case 6: magn = 6; pow = 50; break;
         case 7: case 8: case 9: case 10: case 11: case 12: magn = 7; pow = 70; break;
         case 13: case 14: case 15: case 16: magn = 8; pow = 90; break;
         case 17: case 18: magn = 9; pow = 110; break;
         case 19: default: magn = 10; pow = 150; break;
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
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
	/* if has moved or is using a multi-turn move */
	if (b.koed(t) || turn(b,t).value("HasMoved").toBool() || turn(b,t).value("NoChoice").toBool()) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int num = move(b,t);
        if (MoveInfo::Power(num, b.gen()) == 0) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	turn(b,s)["MeFirstAttack"] = num;
    }

    static void uas(int s, int t, BS &b) {
	removeFunction(turn(b,s), "DetermineAttackFailure", "MeFirst");
	removeFunction(turn(b,s), "UponAttackSuccessful", "MeFirst");
	int move = turn(b,s)["MeFirstAttack"].toInt();
	MoveEffect::setup(move,s,t,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
	b.useAttack(s,move,true,true);
        MoveEffect::unsetup(move,s,b);
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
            int move = b.true_rand() % MoveInfo::NumberOfMoves();

            bool correctMove = !b.hasMove(s,move) && !MMAssist::forbidden_moves.contains(move) && MoveInfo::Exists(move, b.gen());

            if (correctMove) {
		MoveEffect::setup(move,s,t,b);
                turn(b,s)["Target"] = b.randomValidOpponent(s);
		b.useAttack(s,move,true,true);
                MoveEffect::unsetup(move, s, b);
		break;
	    }
	}
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
            (*this) << Metronome << Struggle << Sketch << Mimic << Chatter;
        }
    };

    static FailedMoves FM;

    static void daf(int s, int t, BS &b) {
        if (!poke(b,t).contains("LastMoveUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
        int move = poke(b,t)["LastMoveUsed"].toInt();
        if (b.hasMove(s,move) || FM.contains(move)) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas(int s, int t, BS &b) {
        int move = poke(b,t)["LastMoveUsed"].toInt();
        int slot = poke(b,s)["MoveSlot"].toInt();
        b.changeTempMove(s, slot, move);
	b.sendMoveMessage(81,0,s,type(b,s),t,move);
    }
};

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
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	removeFunction(turn(b,s), "DetermineAttackFailure", "MirrorMove");
	removeFunction(turn(b,s), "UponAttackSuccessful", "MirrorMove");

	int move = poke(b,s)["MirrorMoveMemory"].toInt();
        MoveEffect::setup(move,s,s,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
	b.useAttack(s,move,true,true);
        MoveEffect::unsetup(move,s,b);
    }
};

struct MMMist : public MM
{
    MMMist() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(86,0,s,Pokemon::Ice);
        int source = b.player(s);

        team(b,source)["MistCount"] = 5;
        addFunction(team(b,source), "EndTurn", "Mist", &et);
    }

    static void et(int s, int, BS &b) {
        if (team(b,s).value("MistCount") == 0) {
	    return;
	}

        inc(team(b,s)["MistCount"], -1);
        int count = team(b,s)["MistCount"].toInt();
	if (count == 0) {
	    b.sendMoveMessage(86,1,s,Pokemon::Ice);
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

        b.sendMoveMessage(87,0,s,type(b,s));
	if (weather == BattleSituation::NormalWeather || !b.isWeatherWorking(weather)) {
	    b.healLife(s, b.poke(s).totalLifePoints()/2);
	} else if (b.isWeatherWorking(BattleSituation::Sunny)) {
	    b.healLife(s, b.poke(s).totalLifePoints()*2/3);
	} else {
	    b.healLife(s, b.poke(s).totalLifePoints()/4);
	}
    }
};

struct MMMudSport : public MM
{
    MMMudSport() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	int move = MM::move(b,s);
        b.sendMoveMessage(88, move == MudSport ? 0 : 1, s, type(b,s));
        int type = turn(b,s)["MudSport_Arg"].toInt();
	poke(b,s)["Sported" + QString::number(type)] = true;
	b.battlelong["Sported"+ QString::number(type)] = s;
    }
};

struct MMNightMare : public MM
{
    MMNightMare() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int, int t, BS &b) {
	b.sendMoveMessage(92, 0, t, Pokemon::Ghost);
	poke(b,t)["HavingNightmares"] = true;
	addFunction(poke(b,t),"AfterStatusChange", "NightMare", &asc);
        addFunction(poke(b,t),"EndTurn64", "NightMare", &et);
    }

    static void asc(int s, int, BS &b) {
        if (b.poke(s).status() != Pokemon::Asleep) {
            removeFunction(poke(b,s),"AfterStatusChange", "NightMare");
            removeFunction(poke(b,s),"EndTurn64", "NightMare");
        }
    }

    static void et(int s, int, BS &b) {
        if (!b.koed(s) && b.poke(s).status() == Pokemon::Asleep && !b.hasWorkingAbility(s, Ability::MagicGuard)) {
	    b.sendMoveMessage(92,0,s,Pokemon::Ghost);
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
        tmove(b, s).power = tmove(b, s).power * 40 * (b.true_rand() % 4);
        if (tmove(b, s).power == 0) {
            tmove(b, s).power = 1;
        }
    }

    static void cad(int s, int t, BS &b) {
	b.sendMoveMessage(96,0,s,type(b,s),t);
        b.healLife(t, 80);
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
	    turn(b,s)["Failed"] = true;
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

    static void cad (int s, int t, BS &b) {
        b.inflictDamage(t, fpoke(b,s).level * (5 + (b.true_rand() % 11)) / 10,s,true);
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

            b.sendMoveMessage(104, turn(b,s)["RazorWind_Arg"].toInt(), s, type(b,s));
            /* Skull bash */
            if (mv == SkullBash) {
                b.inflictStatMod(s,Defense,1, s);
            }

            if (b.hasWorkingItem(s, Item::PowerHerb)) {
		//Power Herb
		b.sendItemMessage(11,s);
		b.disposeItem(s);

                if (mv == SolarBeam && b.weather != BS::NormalWeather && b.weather != BS::Sunny && b.isWeatherWorking(b.weather)) {
                    tmove(b, s).power = tmove(b, s).power * 2;
                }
	    } else {              
		poke(b,s)["ChargingMove"] = mv;
		poke(b,s)["ReleaseTurn"] = b.turn() + 1;
		turn(b,s)["TellPlayers"] = false;
                tmove(b, s).power = 0;
                tmove(b, s).targets = Move::User;
		addFunction(poke(b,s), "TurnSettings", "RazorWind", &ts);
	    }
	}
    }

    static void ts(int s, int, BS &b) {
	if (poke(b,s).value("ReleaseTurn").toInt() != b.turn()) {
	    removeFunction(poke(b,s), "TurnSettings", "RazorWind");
	    return;
	}
	turn(b,s)["NoChoice"] = true;
        int mv = poke(b,s)["ChargingMove"].toInt();
        MoveEffect::setup(mv,s,s,b);
        if (mv == SolarBeam && b.weather != BS::NormalWeather && b.weather != BS::Sunny && b.isWeatherWorking(b.weather)) {
            tmove(b, s).power = tmove(b, s).power * 2;
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
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "UponOffensiveDamageReceived", "Rage", &uodr);
        if (poke(b,s).contains("RageBuilt") && poke(b,s)["LastMoveUsed"] == Move::Rage) {
            poke(b,s).remove("AttractBy");
            b.healConfused(s);
            poke(b,s).remove("Tormented");
        }
        poke(b,s).remove("RageBuilt");
    }

    static void uodr(int s, int, BS &b) {
        if (!b.koed(s) && poke(b,s)["LastMoveUsed"] == Move::Rage) {
            poke(b,s)["RageBuilt"] = true;
            if (!b.hasMaximalStatMod(s, Attack)) {
                b.inflictStatMod(s, Attack, 1,false);
                b.sendMoveMessage(102, 0, s);
            }
        }

    }
};

struct MMSafeGuard : public MM
{
    MMSafeGuard() {
        functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        int source = b.player(s);
        if (team(b,source).value("SafeGuardCount").toInt() > 0)
            turn(b,s)["Failed"] = true;
    }

    static void uas(int s, int, BS &b) {
        int source = b.player(s);
	b.sendMoveMessage(109,0,s,type(b,s));
        team(b,source)["SafeGuardCount"] = 5;
        addFunction(team(b,source), "EndTurn", "SafeGuard", &et);
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
        if (b.koed(t) || move == Struggle || move == Chatter || move == 0) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int t, BS &b) {
	int mv = poke(b,t)["LastMoveUsed"].toInt();
	b.sendMoveMessage(111,0,s,type(b,s),t,mv);
	int slot = poke(b,s)["MoveSlot"].toInt();
	b.poke(s).move(slot).num() = mv;
        b.poke(s).move(slot).load(b.gen());
	b.changePP(s,slot,2);
	b.changePP(s,slot,b.poke(s).move(slot).totalPP());
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
	    turn(b,s)["Failed"] = true;
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
	    /*
    * That and any move the user cannot choose for use, including moves with zero PP
*/
            (*this) << Assist << Bide << Bounce << Chatter << Copycat << Dig << Dive << Fly
                    << FocusPunch << MeFirst << Metronome << MirrorMove << ShadowForce <<
                    SkullBash << SkyAttack << SleepTalk << SolarBeam << RazorWind << Uproar;
	}
    };

    static FM forbidden_moves;

    static void daf(int s, int, BS &b) {
	b.callpeffects(s, s, "MovesPossible");
	QList<int> mp;

        for (int i = 0; i < 4; i++) {
            /* Sleep talk can work on 0 PP moves but not on disabled moves*/
            /* On gen 5 it can work several times behind a choice band, so i allowed disabled moves, as
               choice band blocks moves the same way, but it needs to be cross checked. */
            if ( (b.gen() >= 5 || turn(b, s).value("Move" + QString::number(i) + "Blocked").toBool() == false)
                && !forbidden_moves.contains(b.move(s,i))) {
                mp.push_back(i);
            }
        }

	if (mp.size() == 0) {
	    turn(b,s)["Failed"] = true;
	} else {
            turn(b,s)["SleepTalkedMove"] = b.move(s, mp[b.true_rand()%mp.size()]);
	}
    }

    static void uas(int s, int, BS &b) {
	removeFunction(turn(b,s), "DetermineAttackFailure", "SleepTalk");
	removeFunction(turn(b,s), "UponAttackSuccessful", "SleepTalk");
	int mv = turn(b,s)["SleepTalkedMove"].toInt();
        MoveEffect::setup(mv,s,s,b);
        turn(b,s)["Target"] = b.randomValidOpponent(s);
	b.useAttack(s, mv, true);
        MoveEffect::unsetup(mv,s,b);
    }
};

MMSleepTalk::FM MMSleepTalk::forbidden_moves;

struct MMSmellingSalt : public MM
{
    MMSmellingSalt () {
	functions["BeforeCalculatingDamage"] = &bcd;
	functions["AfterAttackSuccessful"] = &aas;
    }

    static void bcd(int s, int t, BS &b) {
        if (b.hasSubstitute(t))
            return;

        int st = turn(b,s)["SmellingSalt_Arg"].toInt();
        if ( (st == 0 && b.poke(t).status() != Pokemon::Fine) || b.poke(t).status() == st) {
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
	addFunction(b.battlelong, "DetermineGeneralAttackFailure", "Snatch", &dgaf);
	b.battlelong["Snatcher"] = s;
	turn(b,s)["Snatcher"] = true;
        b.sendMoveMessage(118,1,s,type(b,s));
    }

    static void dgaf(int s, int , BS &b) {
	if (b.battlelong.contains("Snatcher")) {
            int snatcher = b.battlelong["Snatcher"].toInt();
            if (b.player(s) == b.player(snatcher)) {
                return;
            }
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
                b.battlelong.remove("Snatcher");
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
            turn(b,s)["Failed"] = true;
            return;
        }
        int tu = poke(b,t)["LastMoveUsedTurn"].toInt();
        if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
            turn(b,s)["Failed"] = true;
            return;
        }
        int slot = poke(b,t)["MoveSlot"].toInt();
        if (b.poke(t).move(slot).PP() == 0) {
            turn(b,s)["Failed"] = true;
            return;
        }
    }
    static void uas(int s, int t, BS &b)
    {
        int slot = poke(b,t)["MoveSlot"].toInt();
        if (b.gen() >= 4)
            b.losePP(t, slot, 4);
        else
            b.losePP(t, slot, 2 + (b.true_rand()%4) );
        b.sendMoveMessage(123,0,s,Pokemon::Ghost,t,b.move(t,slot));
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

struct MMStomp : public MM
{
    MMStomp(){
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        if (poke(b,t).value("Minimize").toBool()) {
            tmove(b, s).power = tmove(b, s).power * 2;
        }
    }
};

struct MMSuckerPunch : public MM
{
    MMSuckerPunch(){
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (turn(b,t).value("HasMoved").toBool() || tmove(b, t).power == 0) {
            turn(b,s)["Failed"] = true;
        }
    }
};

struct MMTailWind : public MM {
    MMTailWind(){
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b){
        b.sendMoveMessage(133,0,s,Pokemon::Flying);
        int source = b.player(s);
        team(b,source)["TailWindCount"] = b.gen() <= 4 ? 3 : 4;
        addFunction(team(b,source), "EndTurn", "TailWind", &et);
    }

    static void et(int s, int, BS &b) {
        inc(team(b,s)["TailWindCount"], -1);
        if (team(b,s)["TailWindCount"].toInt() == 0) {
            removeFunction(team(b,s), "EndTurn", "TailWind");
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
            turn(b,s)["Failed"] = true;
    }

    static void uas (int s, int t, BS &b) {
        poke(b,t)["Tormented"] = true;
        addFunction(poke(b,t), "MovesPossible", "Torment", &msp);
        b.sendMoveMessage(135,0,s,Pokemon::Dark,t);
    }

    static void msp(int s, int, BS &b) {
        if (!poke(b,s).contains("Tormented") || poke(b,s)["LastMoveUsedTurn"].toInt() != b.turn() - 1)
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

    static void uas(int s, int, BS &b) {
        if (b.battlelong.value("TrickRoomCount").toInt() > 0) {
            b.sendMoveMessage(138,1,s,Pokemon::Psychic);
            b.battlelong.remove("TrickRoomCount");
            removeFunction(b.battlelong, "EndTurn9", "TrickRoom");
        } else {
            b.sendMoveMessage(138,0,s,Pokemon::Psychic);
            b.battlelong["TrickRoomCount"] = 5;
            addFunction(b.battlelong, "EndTurn9", "TrickRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battlelong["TrickRoomCount"], -1);
        if (b.battlelong["TrickRoomCount"].toInt() == 0) {
            b.sendMoveMessage(138,1,s,Pokemon::Psychic);
            b.battlelong.remove("TrickRoomCount");
        }
    }
};

struct MMTripleKick : public MM {
    MMTripleKick() {
        functions["BeforeHitting"] = &bh;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void bh(int s, int t, BS &b) {
        int count = 1;
        if (b.testAccuracy(s, t, true)) {
            count += 1;
            if (b.testAccuracy(s, t), true) {
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
        if (b.ability(t) == Ability::Truant || b.ability(t) == Ability::Multitype) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        int ab = turn(b,s)["WorrySeed_Arg"].toInt();
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

        if (b.poke(t).status() != Pokemon::Fine || team(b,opp).value("SafeGuardCount").toInt() > 0 || poke(b,t).value("YawnCount").toInt() > 0) {
            turn(b,s)["Failed"] = true;
            return;
        }
        if (b.hasWorkingAbility(t, Ability::Insomnia) || b.hasWorkingAbility(t, Ability::VitalSpirit)) {
            b.fail(s, 144, 2, 0, t);
        }
        if (b.sleepClause() && b.currentForcedSleepPoke[b.player(t)] != -1) {
            b.notifyClause(ChallengeInfo::SleepClause, true);
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(144,0,s,Pokemon::Normal,t);
        poke(b,t)["YawnCount"] = 2;
        addFunction(poke(b,t), "EndTurn617", "Yawn", &et);
    }

    static void et(int s, int, BS &b) {
        inc(poke(b,s)["YawnCount"], -1);
        int count = poke(b,s)["YawnCount"].toInt();
        if (count != 0) {

        } else {
            if (b.poke(s).status() == Pokemon::Fine) {
                if (b.sleepClause() && b.currentForcedSleepPoke[b.player(s)] != -1) {
                    b.notifyClause(ChallengeInfo::SleepClause, true);
                } else {
                    b.inflictStatus(s, Pokemon::Asleep, s);
                    if (b.sleepClause() && b.poke(s).status() == Pokemon::Asleep) {
                        b.currentForcedSleepPoke[b.player(s)] = b.currentInternalId(s);
                    }
                }
            }
            removeFunction(poke(b,s),"EndTurn617", "Yawn");
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
            turn(b,s)["Failed"] = true;
        }
    }
};

struct MMExplosion : public MM {
    MMExplosion() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int , BS &b) {
        //Damp
        foreach (int i, b.sortedBySpeed()) {
            if (b.hasWorkingAbility(i, Ability::Damp) && !b.koed(i)) {
                b.sendMoveMessage(114,0,i);
                turn(b,s)["FaintActivationPrevented"] = true;
                tmove(b, s).power = 0;
                tmove(b,s).targets = Move::User;
                return;
            }
        }
    }
};

struct MMCamouflage : public MM {
    MMCamouflage() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        fpoke(b,s).type1 = Pokemon::Normal;
        fpoke(b,s).type2 = Pokemon::Curse;
        b.sendMoveMessage(17,0,s,0);
    }
};

struct MMNaturePower : public MM
{
    MMNaturePower() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        removeFunction(turn(b,s), "UponAttackSuccessful", "NaturePower");

        int move = TriAttack;
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
        if (b.ability(t) == Ability::WonderGuard || b.ability(t) == Ability::Multitype || b.ability(t) == Ability::Illusion) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.sendMoveMessage(108,0,s,Pokemon::Psychic,t,b.ability(t));
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
            || b.ability(s) == Ability::WonderGuard || b.ability(s) == Ability::Illusion || b.ability(t) == Ability::Illusion) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        int tab = b.ability(t);
        int sab = b.ability(s);

        b.sendMoveMessage(112,0,s,Pokemon::Psychic,t);
        b.acquireAbility(s, tab);
        b.acquireAbility(t, sab);
    }
};

struct MMSecretPower : public MM {
    MMSecretPower() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        tmove(b,s).classification = Move::OffensiveStatusInducingMove;
        tmove(b,s).status = Pokemon::Paralysed;
        tmove(b,s).rate = 30;
    }
};

struct MMBeatUp : public MM {
    MMBeatUp() {
        functions["MoveSettings"] = &ms;
        functions["DetermineAttackFailure"] = &daf;
        functions["CustomAttackingDamage"] = &cad;
    }

    static void ms(int s, int, BS &b) {
        tmove(b,s).type = Pokemon::Curse;
        tmove(b,s).minTurns = 0;
        tmove(b,s).maxTurns = 0;
    }

    static void daf(int s,int, BS&b) {
        int source = b.player(s);
        for (int i = 0; i < 6; i++) {
            if (b.poke(source, i).status() == Pokemon::Fine) {
                return;
            }
        }
        turn(b,s)["Failed"] = true;
    }

    static void cad(int s, int t, BS &b) {
        int source = b.player(s);
        int def = PokemonInfo::Stat(b.poke(t).num(),Defense,b.poke(t).level(),0,0);
        for (int i = 0; i < 6; i++) {
            PokeBattle &p = b.poke(source,i);
            if (p.status() == Pokemon::Fine) {
                int att = PokemonInfo::Stat(p.num(),Attack,p.level(),0,0);
                int damage = (((((p.level() * 2 / 5) + 2) * 10 * att / 50) / def) + 2) * (b.true_rand() % (255-217) + 217)*100/255/100;
                b.sendMoveMessage(7,0,s,Pokemon::Dark,t,0,p.nick());
                if (b.hasSubstitute(t))
                    b.inflictSubDamage(t,damage,t);
                else
                    b.inflictDamage(t,damage,t,true);
                turn(b,t)["HadSubstitute"] = turn(b,t)["Substitute"];
            }
            if (b.koed(t))
                return;
        }
    }
};

struct MMOutrage : public MM
{
    MMOutrage() {
        functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static void uas(int s, int, BS &b) {
        // Asleep is for Sleep Talk
        if ( (!turn(b,s)["OutrageBefore"].toBool() || poke(b,s).value("OutrageUntil").toInt() < b.turn())
                    && b.poke(s).status() != Pokemon::Asleep) {
            poke(b,s)["OutrageUntil"] = b.turn() +  1 + (b.true_rand() % 2);
            addFunction(poke(b,s), "TurnSettings", "Outrage", &ts);
            addFunction(poke(b,s), "MoveSettings", "Outrage", &ms);
            addFunction(poke(b,s), "EndTurn610", "Outrage", &aas);
            poke(b,s)["OutrageMove"] = move(b,s);
        }
    }

    static void aas(int s, int, BS &b) {
        if (poke(b,s).contains("OutrageUntil") && b.turn() == poke(b,s)["OutrageUntil"].toInt() && poke(b,s)["LastOutrage"] == b.turn()) {
            removeFunction(poke(b,s), "TurnSettings", "Outrage");
            removeFunction(poke(b,s), "EndTurn610", "Outrage");
            poke(b,s).remove("OutrageUntil");
            poke(b,s).remove("OutrageMove");
            poke(b,s).remove("LastOutrage");
            b.sendMoveMessage(93,0,s,type(b,s));
            b.inflictConfused(s, s, true);
        }
    }

    static void ts(int s, int, BS &b) {
        if (poke(b,s).value("OutrageUntil").toInt() >= b.turn() && poke(b,s)["LastOutrage"].toInt() == b.turn()-1) {
            turn(b,s)["NoChoice"] = true;
            MoveEffect::setup(poke(b,s)["OutrageMove"].toInt(),s,s,b);
        }
    }

    static void ms(int s, int, BS &b) {
        turn(b,s)["OutrageBefore"] = poke(b,s).contains("LastOutrage") && poke(b,s)["LastOutrage"].toInt() == b.turn() - 1;
        poke(b,s)["LastOutrage"] = b.turn();
    }
};

struct MMUproar : public MM {
    MMUproar() {
        functions["UponAttackSuccessful"] = &uas;
        functions["MoveSettings"] = &ms;
    }

    static void uas(int s,int, BS &b) {
        if (poke(b,s).value("UproarUntil").toInt() < b.turn() || !turn(b,s).value("UproarBefore").toBool()) {
            if (b.gen() <= 4)
                poke(b,s)["UproarUntil"] = b.turn() + 1 + (b.true_rand() % 4);
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
            addFunction(poke(b,s), "EndTurn610", "Uproar", &et);
            addFunction(poke(b,s), "TurnSettings", "Uproar", &ts);
            poke(b,s)["UproarMove"] = move(b,s);
        }
        poke(b,s)["LastUproar"] = b.turn();
    }


    static void ts(int s, int, BS &b) {
        turn(b,s)["UproarBefore"] = poke(b,s).contains("LastUproar") && poke(b,s).value("LastUproar").toInt() == b.turn()  - 1;
        if (poke(b,s).value("UproarUntil").toInt() >= b.turn() && poke(b,s).value("LastUproar").toInt() == b.turn() - 1) {
            turn(b,s)["NoChoice"] = true;
            MoveEffect::setup(poke(b,s)["UproarMove"].toInt(),s,s,b);
        }
    }

    static void ms(int s, int, BS &b) {
        turn(b,s)["Uproared"] = true;
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;
        if (!turn(b,s).contains("Uproared"))
            return;
        if (poke(b,s).value("UproarUntil").toInt() > b.turn()) {
            b.sendMoveMessage(141,1,s);

            foreach (int i, b.sortedBySpeed()) {
                if (b.poke(i).status() == Pokemon::Asleep) {
                    b.sendMoveMessage(141,3,i);
                    b.changeStatus(i, Pokemon::Normal);
                }
            }
        } else {
            if (b.turn() >= poke(b,s)["UproarUntil"].toInt()) {
                removeFunction(poke(b,s), "TurnSettings", "Uproar");
                removeFunction(poke(b,s), "EndTurn610", "Uproar");
                poke(b,s).remove("UproarUntil");
                poke(b,s).remove("LastUproar");
                b.removeUproarer(s);
                b.sendMoveMessage(141,2,s,type(b,s));
            }
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
            turn(b,s)["Failed"] = true;
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

struct MMSwallow: public MM
{
    MMSwallow() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (poke(b,s).value("StockPileCount").toInt() == 0) {
            turn(b,s)["Failed"] = true;
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
        case 1: b.healLife(s, b.poke(s).totalLifePoints()/4); break;
        case 2: b.healLife(s, b.poke(s).totalLifePoints()/2); break;
        case 3: default: b.healLife(s, b.poke(s).totalLifePoints()); break;
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
            turn(b,s)["Failed"] = true;
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

struct MMBugBite : public MM
{
    MMBugBite() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        int item = b.poke(t).item();
        if (!ItemInfo::isBerry(item))
            return;
        int sitem = b.poke(s).item();
        b.poke(s).item() = item;
        b.disposeItem(t);

        /* Setting up the conditions so berries work properly */
        turn(b,s)["BugBiter"] = true; // for testPinch of pinch berries to return true
        QVariant tempItemStorage = poke(b,s)["ItemArg"];
        poke(b,s)["ItemArg"] = poke(b,t)["ItemArg"];

        b.sendMoveMessage(16,0,s,type(b,s),t,item);

        /* Finding the function to call :P */
        QList<ItemInfo::Effect> l = ItemInfo::Effects(item, b.gen());

        foreach(ItemInfo::Effect e, l) { /* Ripped from items.cpp (ItemEffect::activate, with some changes) */
            if (!ItemEffect::mechanics.contains(e.num)) {
                continue;
            }
            foreach (function f, ItemEffect::mechanics[e.num].functions) {
                f(s, t, b);

                //Some berries have 2 functions for pinch testing... so quitting after one used up the berry
                if (b.poke(s).item() == 0) {
                    break;
                }
            }
        }

        /* Restoring initial conditions */
        poke(b,s)["ItemArg"] = tempItemStorage;
        turn(b,s).remove("BugBiter");
        b.poke(s).item() = sitem;
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
            turn(b,s)["Failed"] = true;
        }
    }

    static void btl(int s, int, BS &b) {
        int berry = b.poke(s).item();

        if (!b.hasWorkingItem(s, berry) || !ItemInfo::isBerry(berry)) {
            return;
        }

        b.eatBerry(s);

        tmove(b, s).power = tmove(b, s).power * ItemInfo::BerryPower(berry);
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
        int source = b.player(s);

        if (!team(b,source).contains("RecyclableItem") || b.poke(s).item() != 0) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas (int s, int, BS &b) {
        int source = b.player(s);

        int item = team(b,source)["RecyclableItem"].toInt();
        b.sendMoveMessage(105,0,s,0,s,item);
        b.acqItem(s, item);
        team(b,source).remove("RecyclableItem");
    }
};

struct MMTransform : public MM {
    MMTransform() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        /* Give new values to what needed */
        Pokemon::uniqueId num = b.pokenum(t);
        if (num.toPokeRef() == Pokemon::Giratina_O && b.poke(s).item() != Item::GriseousOrb)
            num = Pokemon::Giratina;
        if (PokemonInfo::OriginalForme(num) == Pokemon::Arceus) {
            num.subnum = ItemInfo::PlateType(b.poke(s).item());
        }

        b.sendMoveMessage(137,0,s,0,s,num.pokenum);

        BS::BasicPokeInfo &po = fpoke(b,s);
        BS::BasicPokeInfo &pt = fpoke(b,t);

        po.id = num;
        po.weight = PokemonInfo::Weight(num);
        po.type1 = PokemonInfo::Type1(num, b.gen());
        po.type2 = PokemonInfo::Type2(num, b.gen());
        po.ability = b.ability(t);

        b.changeSprite(s, num);

        for (int i = 0; i < 4; i++) {
            b.changeTempMove(s,i,b.move(t,i));
        }

        for (int i = 1; i < 6; i++)
            po.stats[i] = pt.stats[i];

        for (int i = 0; i < 6; i++) {
            po.dvs[i] = pt.dvs[i];
        }

        for (int i = 0; i < 8; i++) {
            po.boosts[i] = pt.boosts[i];
        }

        b.acquireAbility(s, b.ability(s));
    }
};

struct MMPayback : public MM
{
    MMPayback() {
        functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
        //Attack / Switch --> power *= 2
        if (turn(b,t).value("HasMoved").toBool() || turn(b,t).value("CantGetToMove").toBool()) {
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
        turn(b,s)["Failed"] = true;
    }

    static void uas(int , int t, BS &b) {
        QVector<int> stats;
        for (int i = Attack; i <= Accuracy; i++) {
            if (fpoke(b,t).boosts[i] < 6) {
                stats.push_back(i);
            }
        }
        if (stats.empty())
            return;
        b.inflictStatMod(t, stats[b.true_rand()%stats.size()], 2, t);
    }
};

struct MMHelpingHand : public MM
{
    MMHelpingHand() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (!b.doubles() || b.koed(b.partner(s))) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int , BS &b) {
        int p = b.partner(s);

        b.sendMoveMessage(63, 0, s, type(b,s), p);
        turn(b,p)["HelpingHanded"] = true;
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

        addFunction(b.battlelong, "GeneralTargetChange", "FollowMe", &gtc);
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

        turn(b,s)["TargetChanged"] = true;
        turn(b,s)["Target"] = target;
    }
};

struct MMGuardShare : public MM
{
    MMGuardShare() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        QStringList stats = turn(b,s)["GuardShare_Arg"].toString().split('_');

        b.sendMoveMessage(155, move(b,s) == Move::PowerShare ? 0 : 1, s, type(b,s), t);
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

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        if (b.battlelong.value("MagicRoomCount").toInt() > 0) {
            b.sendMoveMessage(156,1,s,Pokemon::Psychic);
            b.battlelong.remove("MagicRoomCount");
            removeFunction(b.battlelong, "EndTurn9", "MagicRoom");
        } else {
            b.sendMoveMessage(156,0,s,Pokemon::Psychic);
            b.battlelong["MagicRoomCount"] = 5;
            addFunction(b.battlelong, "EndTurn9", "MagicRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battlelong["MagicRoomCount"], -1);
        if (b.battlelong["MagicRoomCount"].toInt() == 0) {
            b.sendMoveMessage(156,1,s,Pokemon::Psychic);
            b.battlelong.remove("MagicRoomCount");
        }
    }
};

struct MMSoak : public MM {
    MMSoak() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasType(t, Pokemon::Water))
            turn(b,s)["Failed"] = true;
    }

    static void uas(int, int t, BS &b) {
        fpoke(b, t).type1 = Pokemon::Water;
        fpoke(b, t).type2 = Pokemon::Curse;
        b.sendMoveMessage(157, 0, t, Pokemon::Water, t);
    }
};

struct MMAssembleCrew : public MM {
    MMAssembleCrew() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.ability(t) == Ability::Multitype || b.ability(t) == Ability::Truant) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.acquireAbility(t, b.ability(s));
        b.sendMoveMessage(158,0,s,type(b,s),t,b.ability(s));
    }
};

struct MMShellCrack : public MM {
    MMShellCrack() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        b.inflictStatMod(s, Defense, -1, s);
        b.inflictStatMod(s, SpDefense, -1, s);
        b.inflictStatMod(s, Attack, 2, s);
        b.inflictStatMod(s, SpAttack, 2, s);
        b.inflictStatMod(s, Speed, 2, s);
    }
};

struct MMIncinerate : public MM {
    MMIncinerate() {
        functions["OnFoeOnAttack"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        if (ItemInfo::isBerry(b.poke(t).item())) {
            b.sendMoveMessage(160, 0, s, Type::Fire, t, b.poke(t).item());
            b.disposeItem(t);
        }
    }
};

struct MMDesperation : public MM {
    MMDesperation() {
        functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.inflictDamage(t, b.poke(s).lifePoints(), s, true);
    }
};

struct MMGiftPass : public MM {
    MMGiftPass() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
        if (!b.koed(t) && b.poke(s).item() != 0
            && b.ability(s) != Ability::Multitype && !b.hasWorkingAbility(s, Ability::Multitype)
            && b.pokenum(s).pokenum != Pokemon::Giratina && b.poke(t).item() == 0 && b.pokenum(t).pokenum != Pokemon::Giratina
                    && !ItemInfo::isMail(b.poke(s).item())) {
            //ok
        } else {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b)
    {
        b.sendMoveMessage(162,0,s,type(b,s),t,b.poke(s).item());
        b.acqItem(t, b.poke(s).item());
        b.loseItem(s);
    }
};

struct MMWindStorm : public MM {
    MMWindStorm() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (b.isWeatherWorking(b.weather)) {
            tmove(b, s).accuracy += 30;
        }
    }
};

struct MMRefresh : public MM {
    MMRefresh() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (b.poke(s).status() != 0) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int, BS &b) {
        b.healStatus(s, 0);
        b.sendMoveMessage(164, 0, s);
    }
};

struct MMMemento : public MM {
    MMMemento() {
        functions["DetermineAttackFailure"] = &daf;
        functions["OnFoeOnAttack"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.hasMinimalStatMod(t, Attack) && b.hasMinimalStatMod(t, SpAttack)) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.inflictStatMod(t, Attack, -2, s);
        b.inflictStatMod(t, SpAttack, -2, s);
    }
};

struct MMAncientSong : public MM
{
    MMAncientSong() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        if (b.poke(s).num() != Pokemon::Meloia)
            return;
        b.changePokeForme(s, Pokemon::Meloia_S);
    }
};

struct MMHeavyBomber : public MM
{
    MMHeavyBomber() {
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

        tmove(b,s).power *= ratio;
    }
};

struct MMWonderRoom : public MM {
    MMWonderRoom() {
        functions["UponAttackSuccessful"] = &uas;
    }

    //fixme: store weather effects (gravity, trickroom, magicroom, wonderroom) in a flagged int hard coded in BattleSituation
    static void uas(int s, int, BS &b) {
        if (b.battlelong.value("WonderRoomCount").toInt() > 0) {
            b.sendMoveMessage(168,1,s,Pokemon::Psychic);
            b.battlelong.remove("WonderRoomCount");
            removeFunction(b.battlelong, "EndTurn9", "WonderRoom");
        } else {
            b.sendMoveMessage(168,0,s,Pokemon::Psychic);
            b.battlelong["WonderRoomCount"] = 5;
            addFunction(b.battlelong, "EndTurn9", "WonderRoom", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(b.battlelong["WonderRoomCount"], -1);
        if (b.battlelong["WonderRoomCount"].toInt() == 0) {
            b.sendMoveMessage(168,1,s,Pokemon::Psychic);
            b.battlelong.remove("WonderRoomCount");
        }
    }
};

struct MMWideGuard : public MM
{
    MMWideGuard() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battlelong, "DetermineGeneralAttackFailure", "WideGuard", &dgaf);
        team(b,b.player(s))["WideGuardUsed"] = b.turn();
        b.sendMoveMessage(169, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1 || b.player(s) == b.player(t)) {
            return;
        }
        if (!team(b,t).contains("WideGuardUsed") || team(b,t)["WideGuardUsed"].toInt() != b.turn()) {
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

struct MMFastGuard : public MM
{
    MMFastGuard() {
        functions["DetermineAttackFailure"] = &MMDetect::daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        addFunction(b.battlelong, "DetermineGeneralAttackFailure", "FastGuard", &dgaf);
        team(b,b.player(s))["FastGuardUsed"] = b.turn();
        b.sendMoveMessage(170, 0, s, Pokemon::Normal);
    }

    static void dgaf(int s, int t, BS &b) {
        if (s == t || t == -1) {
            return;
        }
        if (!team(b,t).contains("FastGuardUsed") || team(b,t)["FastGuardUsed"].toInt() != b.turn()) {
            return;
        }

        if (! (tmove(b, s).flags & Move::ProtectableFlag) ) {
            return;
        }

        if (tmove(b,s).priority <= 0) {
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
    *DetermineAttackFailure -- turn, poke: set turn()["Failed"] to true to make the attack fail
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
    REGISTER_MOVE(114, Explosion);
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
    REGISTER_MOVE(133, TailWind)
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
    REGISTER_MOVE(146, Avalanche); /* avalanche, revenge */
    //Chatter
    REGISTER_MOVE(148, TrumpCard);
    REGISTER_MOVE(149, Haze);
    REGISTER_MOVE(150, Roost);
    REGISTER_MOVE(151, Ingrain);
    REGISTER_MOVE(152, Thunder);
    //REGISTER_MOVE(153, UnThawing);
    REGISTER_MOVE(154, DefenseCurl);
    REGISTER_MOVE(155, GuardShare);
    REGISTER_MOVE(156, MagicRoom);
    REGISTER_MOVE(157, Soak);
    REGISTER_MOVE(158, AssembleCrew);
    REGISTER_MOVE(159, ShellCrack);
    REGISTER_MOVE(160, Incinerate);
    REGISTER_MOVE(161, Desperation);
    REGISTER_MOVE(162, GiftPass);
    REGISTER_MOVE(163, WindStorm);
    REGISTER_MOVE(164, Refresh);
    REGISTER_MOVE(165, Memento);
    REGISTER_MOVE(166, AncientSong);
    REGISTER_MOVE(167, HeavyBomber);
    REGISTER_MOVE(168, WonderRoom);
    REGISTER_MOVE(169, WideGuard);
    REGISTER_MOVE(170, FastGuard);
}


#include "moves.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "items.h"

QTSHash<int, MoveMechanics> MoveEffect::mechanics;
QTSHash<int, QString> MoveEffect::names;
QTSHash<QString, int> MoveEffect::nums;

Q_DECLARE_METATYPE(QList<int>)

int MoveMechanics::num(const QString &name)
{
    if (!MoveEffect::nums.contains(name)) {
	return 0;
    } else {
	return MoveEffect::nums[name];
    }
}

MoveEffect::MoveEffect(int num)
{
    /* Different steps: critical raise, number of times, ... */
    (*this)["CriticalRaise"] = MoveInfo::CriticalRaise(num);
    (*this)["RepeatMin"] = MoveInfo::RepeatMin(num);
    (*this)["RepeatMax"] = MoveInfo::RepeatMax(num);
    (*this)["SpeedPriority"] = MoveInfo::SpeedPriority(num);
    (*this)["PhysicalContact"] = MoveInfo::PhysicalContact(num);
    (*this)["KingRock"] = MoveInfo::KingRock(num);
    (*this)["Power"] = MoveInfo::Power(num);
    (*this)["Accuracy"] = MoveInfo::Acc(num);
    (*this)["Type"] = MoveInfo::Type(num);
    (*this)["Category"] = MoveInfo::Category(num);
    (*this)["EffectRate"] = MoveInfo::EffectRate(num);
    (*this)["StatEffect"] = MoveInfo::Effect(num);
    (*this)["FlinchRate"] = MoveInfo::FlinchRate(num);
    (*this)["Recoil"] = MoveInfo::Recoil(num);
    (*this)["Attack"] = num;
    (*this)["PossibleTargets"] = MoveInfo::Target(num);
}

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

void MoveEffect::setup(int num, int source, int target, BattleSituation &b)
{
    MoveEffect e(num);

    /* first the basic info */
    merge(b.turnlong[source], e);

    /* then the hard info */
    QStringList specialEffects = MoveInfo::SpecialEffect(num).split('|');

    foreach (QString specialEffectS, specialEffects) {
	std::string s = specialEffectS.toStdString();

	int specialEffect = atoi(s.c_str());

	/* if the effect is invalid or not yet implemented then no need to go further */
	if (!mechanics.contains(specialEffect)) {
	    return;
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

struct MMLeech : public MM
{
    MMLeech() {
	functions["UponDamageInflicted"] = &aad;
    }

    static void aad(int s, int t, BS &b) {
	if (!b.koed(s)) {
	    int damage = turn(b, s)["DamageInflicted"].toInt();

	    if (damage != 0) {
		int recovered = std::max(1, damage/2);
		int move = MM::move(b,s);
		b.sendMoveMessage(1, move == 106 ? 1 :0, s, type(b,s), t);
		if (b.hasWorkingItem(s, 1)) /* Big root */ {
		    recovered = recovered * 13 / 10;
		}
                //Liquid Ooze
                if (!b.hasWorkingAbility(t, 51)) {
                    b.healLife(s, recovered);
                } else {
                    b.sendMoveMessage(1,2,s,Pokemon::Poison,t);
                    b.inflictDamage(s,recovered,s,false);
                }
	    }
	}
    }
};

struct MMAquaRing : public MM
{
    MMAquaRing() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	addFunction(poke(b,s), "EndTurn", "AquaRing", &et);
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

    static void bcd(int s, int, BS &b) {
	if (turn(b,s).contains("DamageTaken")) {
	    turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 2;
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
	    turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 2;
	}
    }
};

struct MMBatonPass : public MM
{
    MMBatonPass() {
	functions["DetermineAttackSuccessful"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	if (b.countAlive(s) <= 1) {
	    turn(b,s)["Failed"] =true;
	}
    }

    static void uas(int s, int, BS &b) {
	/* first we copy the temp effects, then put them to the next poke */
	BS::context c = poke(b, s);
    	c.remove("Type1");
	c.remove("Type2");
	c.remove("Num");
	c.remove("Minimize");
	c.remove("DefenseCurl");
	c.remove("Ability");
	c.remove("Weight");
	/* removing last resort memory */
	c.remove("Move0Used");
	c.remove("Move1Used");
	c.remove("Move2Used");
	c.remove("Move3Used");
	c.remove("Move0");
	c.remove("Move1");
	c.remove("Move2");
	c.remove("Move3");
	/* choice band etc. would force the same move
		if on both the passed & the passer */
	c.remove("ChoiceMemory");
	for (int i = 1; i < 6; i++) {
	    c.remove(QString("Stat%1").arg(i));
	}
	c.remove("Level");
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

        if (b.hasSubstitute(s)) {
            b.notifySub(s,true);
        }
    }
};

struct MMBlastBurn : public MM
{
    MMBlastBurn() {
	functions["AfterAttackSuccessful"] = &aas;
    }

    static void aas(int s, int, BS &b) {
	addFunction(poke(b, s), "TurnSettings", "BlastBurn", &ts);
	poke(b, s)["BlastBurnTurn"] = b.turn();
    }
    static void ts(int s, int, BS &b) {
	if (poke(b, s)["BlastBurnTurn"].toInt() < b.turn() - 1) {
	    ;
	} else {
	    turn(b, s)["NoChoice"] = true;
	    turn(b, s)["TellPlayers"] = false;
	    turn(b, s)["PossibleTargets"] = Move::None;
	    addFunction(turn(b,s), "UponAttackSuccessful", "BlastBurn", &uas);
	}
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
	    turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 2;
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
	if (poke(b,s)["Charged"] == true && turn(b, s)["Type"].toInt() == Move::Electric) {
	    if (turn(b, s)["Power"].toInt() > 0) {
		turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 2;
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
	    if (MoveInfo::Type(b.move(s,i)) != Move::Curse) {
		break;
	    }
	    if (i == 3) {
		turn(b, s)["Failed"] = true;
		return;
	    }
	}
	if (poke(b,s)["Type2"].toInt() != Pokemon::Curse) {
	    /* It means the pokemon has two types, i.e. conversion always works */
	    QList<int> poss;
	    for (int i = 0; i < 4; i++) {
		if (MoveInfo::Type(b.move(s,i)) != Move::Curse) {
		    poss.push_back(b.move(s,i));
		}
	    }
            turn(b,s)["ConversionType"] = poss[true_rand()%poss.size()];
	} else {
	    QList<int> poss;
	    for (int i = 0; i < 4; i++) {
		if (MoveInfo::Type(b.move(s,i)) != Move::Curse && MoveInfo::Type(b.move(s,i)) != poke(b,s)["Type1"].toInt()) {
		    poss.push_back(b.move(s,i));
		}
	    }
	    if (poss.size() == 0) {
		turn(b, s)["Failed"] = true;
	    } else {
                turn(b,s)["ConversionType"] = poss[true_rand()%poss.size()];
	    }
	}
    }

    static void uas(int s, int, BS &b) {
	int type = turn(b,s)["ConversionType"].toInt();
	poke(b,s)["Type1"] = type;
	poke(b,s)["Type2"] = Pokemon::Curse;
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
	int attackType = MoveInfo::Type(poke(b,s)["LastAttackToHit"].toInt());
	if (attackType == Move::Curse) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	/* Gets types available */
	QList<int> poss;
	for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
	    if (!(poke(b,s)["Type1"].toInt() == i && poke(b,s)["Type2"].toInt() == Pokemon::Curse) && TypeInfo::Eff(attackType, i) < Type::Effective) {
		poss.push_back(i);
	    }
	}
	if (poss.size() == 0) {
	    turn(b,s)["Failed"] = true;
	} else {
            turn(b,s)["Conversion2Type"] = poss[true_rand()%poss.size()];
	}
    }

    static void uas(int s, int, BS &b) {
    	int type = turn(b,s)["Conversion2Type"].toInt();
	poke(b,s)["Type1"] = type;
	poke(b,s)["Type2"] = Pokemon::Curse;
	b.sendMoveMessage(20, 0, s, type, s);
    }
};

struct MMCrushGrip : public MM
{
    MMCrushGrip() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
	turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 120 * b.poke(t).lifePoints()/b.poke(t).totalLifePoints();
	b.sendMoveMessage(24, 0, s, type(b,s), t);
    }
};

struct MMCurse : public MM
{
    MMCurse() {
	functions["BeforeHitting"] = &bh;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void bh(int s, int, BS &b) {
	if (b.hasType(s, Pokemon::Ghost)) {
	    turn(b,s)["StatEffect"] = "";
	    turn(b,s)["CurseGhost"] = true;
	}
    }

    static void uas(int s, int t, BS &b) {
	if (turn(b,s)["CurseGhost"].toBool() == true) {
            b.inflictPercentDamage(s, 50, s);
	    addFunction(poke(b,t), "EndTurn", "Cursed", &et);
	    b.sendMoveMessage(25, 0, s, Pokemon::Curse, t);
	}
    }

    static void et(int s, int, BS &b) {
        b.inflictPercentDamage(s, 25, s);
	b.sendMoveMessage(25, 1, s, Pokemon::Curse);
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
	b.sendMoveMessage(26, 0, s, Pokemon::Ghost);
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
	if (poke(b,s).contains("ProtectiveMoveTurn") && poke(b,s)["ProtectiveMoveTurn"].toInt() == b.turn() - 1) {
            if (true_rand()%2 == 0) {
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
    }

    static void dgaf(int s, int t, BS &b) {
	if (s == t || t == -1) {
	    return;
	}
	if (!turn(b,t)["DetectUsed"].toBool()) {
	    return;
	}
	int attack = turn(b,s)["Attack"].toInt();
	/* Curse, Feint, Psychup, Role Play, Transform */
	if (attack == 78 || attack == 128 || attack == 298 || attack == 330 || attack == 432) {
	    return;
	}
	/* All other moves fail */
	b.fail(s, 27, 0, Pokemon::Normal);
    }
};

struct MMEruption : public MM
{
    MMEruption() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt()*150*b.poke(s).lifePoints()/b.poke(s).totalLifePoints();
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
	    turn(b,s)["Power"] = turn(b,s)["Power"].toInt()*2;
	}
    }
};

struct MMFakeOut : public MM
{
    MMFakeOut() {
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
	if (poke(b,s)["MovesUsed"].toInt() != 1) {
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
	    b.fail(s, 31);
	}
    }
};

struct MMHiddenPower : public MM
{
    MMHiddenPower() {
	functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
	QList<int> &dvs = b.poke(s).dvs();
	turn(b,s)["Type"] = HiddenPowerInfo::Type(dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);
	turn(b,s)["Power"] = HiddenPowerInfo::Power(dvs[0], dvs[1], dvs[2], dvs[3], dvs[4], dvs[5]);
    }
};

struct MMFaintUser : public MM
{
    MMFaintUser() {
	functions["BeforeTargetList"] = &ms;
    }

    static void ms(int s, int, BS &b) {
        if (!turn(b,s).value("FaintActivationPrevented").toBool())
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
	} else {
	    turn(b, s)["Failed"] = true;
	}
    }
};

struct MMOHKO : public MM
{
    MMOHKO() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.inflictDamage(t, b.poke(t).totalLifePoints(), s, true);
    }

    static void daf(int s, int t, BS &b) {
	if (b.poke(s).level() < b.poke(t).level()) {
	    turn(b,s)["Failed"] = true;
            return;
	}
        if (b.hasWorkingAbility(t, 103)) {
            b.fail(s,43,0,type(b,s));
        }
    }
};

struct MMFlail : public MM
{
    MMFlail() {
	functions["BeforeCalculatingDamage"] = &bcd;
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

	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * mult;
    }
};

struct MMTrumpCard : public MM
{
    MMTrumpCard() {
	functions["BeforeCalculatingDamage"] = &bcd;
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
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * mult;
    }
};

struct MMFrustration : public MM
{
    MMFrustration() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int, BS &b) {
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * 102;
    }
};

struct MMSuperFang : public MM
{
    MMSuperFang() {
	functions["CustomAttackingDamage"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
        b.inflictPercentDamage(t,50, s, true);
    }
};

struct MMPainSplit : public MM
{
    MMPainSplit() {
	functions["UponAttackSuccessful"] = &uas;
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
    }

    static void uas(int, int, BS &b) {
	for (int t = BS::Player1; t <= BS::Player2; t++) {
            //SoundProof
            if (poke(b,t).contains("PerishSongCount")) {
		continue;
	    }
	    addFunction(poke(b,t), "EndTurn", "PerishSong", &et);
	    poke(b, t)["PerishSongCount"] = 3;
	}
	b.sendMoveMessage(95);
    }

    static void et(int s, int, BS &b) {
	int count = poke(b,s)["PerishSongCount"].toInt();
        //SoundProof
        if (!b.hasWorkingAbility(s,95))
            b.sendMoveMessage(95,1,s,0,0,count);
	if (count > 0) {
	    poke(b,s)["PerishSongCount"] = count - 1;
        } else if (!b.hasWorkingAbility(s,95)){ //SoundProof
	    b.koPoke(s,s,false);
	}
    }
};

struct MMHaze : public MM
{
    MMHaze() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	/* sending the message only once */
	if (s == t)
	    b.sendMoveMessage(149);
	for (int i = 1; i <= 7; i++) {
	    poke(b,t)["Stat"+QString::number(i)] = 0;
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
	    b.fail(s, 72,0,Pokemon::Grass);
	}
    }

    static void uas(int s, int t, BS &b) {
	addFunction(poke(b,t), "EndTurn", "LeechSeed", &et);
	poke(b,t)["SeedSource"] = s;
	b.sendMoveMessage(72, 1, s, Pokemon::Grass, t);
    }

    static void et(int s, int, BS &b) {
	if (b.koed(s))
	    return;
	int damage = b.poke(s).totalLifePoints() / 8;
	b.sendMoveMessage(72, 2, s, Pokemon::Grass);
	b.inflictDamage(s, damage, s, false);
	int s2 = poke(b,s)["SeedSource"].toInt();
	if (b.koed(s2))
	    return;
	b.healLife(s2, damage);
    }
};

struct MMHealHalf : public MM
{
    MMHealHalf() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.healLife(s, b.poke(s).totalLifePoints()/2);
	b.sendMoveMessage(60, 0, s, type(b,s));
    }
};

struct MMRoost : public MM
{
    MMRoost() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(150,0,s,Pokemon::Flying);

	turn(b,s)["Roosted"] = true;
	addFunction(poke(b,s), "EndTurn", "Roost", &et);
    }

    static void et(int s, int, BS &b) {
	turn(b,s)["Roosted"] = false;
    }
};

struct MMRest : public MM
{
    MMRest() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        // Insomnia, Vital Spirit
        if (b.poke(s).status() == Pokemon::Asleep || b.poke(s).isFull() || b.hasWorkingAbility(s,42) || b.hasWorkingAbility(s,118)) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	b.healLife(s, b.poke(s).totalLifePoints());
	b.changeStatus(s, Pokemon::Asleep);
	b.poke(s).sleepCount() = 2;
    }
};

struct MMBellyDrum : public MM
{
    MMBellyDrum() {
	functions["UponAttackSuccessful"] = &uas;
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int, BS &b) {
	if (b.poke(s).lifePoints() <= b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100) {
	    b.fail(s, 8);
	}
    }
    static void uas(int s, int, BS &b) {
	b.inflictDamage(s, b.poke(s).totalLifePoints()*turn(b,s)["BellyDrum_Arg"].toInt()/100,s);
    }
};

struct MMWish : public MM
{
    MMWish() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
        if (team(b,s).contains("WishCount") && team(b,s)["WishCount"].toInt() >= 0) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
        team(b,s)["WishCount"] = 1;
        team(b,s)["Wisher"] = b.poke(s).nick();
	b.sendMoveMessage(142, 0, s);
        addFunction(team(b,s), "EndTurn", "Wish", &et);
    }

    static void et(int s, int, BS &b) {
	if (b.koed(s)) {
	    return;
	}
        int count = team(b,s)["WishCount"].toInt();
	if (count < 0) {
	    return;
	}
	if (count == 0) {
            b.sendMoveMessage(142, 1, 0, 0, 0, 0, team(b,s)["Wisher"].toString());
	    b.healLife(s, b.poke(s).totalLifePoints()/2);
	}
        team(b,s)["WishCount"] = count - 1;
    }
};

struct MMBlock : public MM
{
    MMBlock() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
	poke(b,s)["Blocked"] = t;
	poke(b,t)["BlockedBy"] = s;
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
	addFunction(poke(b,s), "EndTurn", "Ingrain", &et);
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
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
	/* ingrain & suction cups */
        if (poke(b,t).value("Rooted").toBool()) {
            b.fail(s, 107, 1, Pokemon::Grass);
        } else if (b.hasWorkingAbility(t,104)) {
            b.fail(s, 107, 0);
        } else{
	    if (b.countAlive(t) <= 1) {
		turn(b,s)["Failed"] = true;
	    }
	}
    }

    static void uas(int, int t, BS &b) {
	int num = b.currentPoke(t);
	b.sendBack(t);
	QList<int> switches;
	for (int i = 0; i < 6; i++) {
	    if (i != num && !b.poke(t,i).ko()) {
		switches.push_back(i);
	    }
	}
        b.sendPoke(t, switches[true_rand()%switches.size()]);
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
	if (team(b,b.rev(s)).value("Spikes").toInt() >= 3) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	int t = b.rev(s);
	team(b,t)["Spikes"] = std::min(3, team(b,t).value("Spikes").toInt()+1);
	addFunction(team(b,t), "UponSwitchIn", "Spikes", &usi);
	b.sendMoveMessage(121, 0, s, 0,b.rev(s));
    }

    static void usi(int s, int, BS &b) {
	int spikeslevel = team(b,s)["Spikes"].toInt();
	if (spikeslevel <= 0 || b.isFlying(s)) {
	    return;
	}
	int n = (spikeslevel+1);
	b.sendMoveMessage(121,1,s);
	b.inflictDamage(s, b.poke(s).totalLifePoints()*n/16, s);
    }
};

struct MMStealthRock : public MM
{
    MMStealthRock() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	int t = b.rev(s);
	if (team(b,t).value("StealthRock").toBool() == true) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	int t = b.rev(s);
	team(b,t)["StealthRock"] = true;
	addFunction(team(b,t), "UponSwitchIn", "StealthRock", &usi);
	b.sendMoveMessage(124,0,s,Pokemon::Rock,b.rev(s));
    }

    static void usi(int s, int, BS &b) {
	if (team(b,s)["StealthRock"].toBool() == true)
	{
	    b.sendMoveMessage(124,1,s,Pokemon::Rock);
	    int n = TypeInfo::Eff(Pokemon::Rock, poke(b,s)["Type1"].toInt()) * TypeInfo::Eff(Pokemon::Rock, poke(b,s)["Type2"].toInt());
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
	int t = b.rev(s);
	if (team(b,t).value("ToxicSpikes").toInt() >= 2) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	int t = b.rev(s);
	team(b,t)["ToxicSpikes"] = team(b,t)["ToxicSpikes"].toInt()+1;
	b.sendMoveMessage(136, 0, s, Pokemon::Poison, b.rev(s));
	addFunction(team(b,t), "UponSwitchIn", "ToxicSpikes", &usi);
    }

    static void usi(int s, int, BS &b) {
	if (b.hasType(s, Pokemon::Poison)) {
	    team(b,s)["ToxicSpikes"] = 0;
	    b.sendMoveMessage(136, 1, s, Pokemon::Poison, b.rev(s));
	    return;
	}
	if (b.hasSubstitute(s) || b.isFlying(s)) {
	    return;
	}
	if (team(b,s).value("SafeGuardCount").toInt() > 0) {
	    return;
	}
	int spikeslevel = team(b,s)["ToxicSpikes"].toInt();
	switch (spikeslevel) {
	    case 0: return;
            case 1: b.inflictStatus(s, Pokemon::Poisoned, s); break;
            default: b.inflictStatus(s, Pokemon::DeeplyPoisoned, s); break;
	}
    }
};

struct MMRapidSpin : public MM
{
    MMRapidSpin() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	team(b,s).remove("Spikes");
	team(b,s).remove("ToxicSpikes");
	team(b,s).remove("StealthRock");
	poke(b,s).remove("SeedSource");
	poke(b,s).remove("TrappedBy");
    }
};

struct MMUTurn : public MM
{
    MMUTurn() {
	functions["AfterAttackSuccessful"] = &aas;
    }

    static void aas(int s, int, BS &b) {
	if (b.countAlive(s) <= 1) {
	    return;
	}
	if (b.koed(s)) {
	    return;
	}
	b.requestSwitch(s);
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
        b.notifySub(s,true);
	addFunction(poke(b,s), "BlockTurnEffects", "Substitute", &bte);
    }

    static void bte(int s, int t, BS &b) {
        if (s == t || s==-1) {
	    return;
	}
        if (!b.hasSubstitute(s)) {
	    return;
	}
        QString effect = turn(b,t)["EffectActivated"].toString();

	if (effect == "Bind" || effect == "Block" || effect == "Covet" || effect == "Curse" || effect == "Embargo" || effect == "GastroAcid" || effect == "Grudge"
	    || effect == "HealBlock" || effect == "KnockOff" || effect == "LeechSeed"
	    || effect == "LockOn" || effect == "Mimic" || effect == "PsychoShift" || effect == "Sketch" || effect == "Switcheroo"
	    || effect == "WorrySeed" || effect == "Yawn")
	{
            turn(b,t)["EffectBlocked"] = true;
            if (turn(b,t)["Power"].toInt() == 0)
                b.sendMoveMessage(128, 2, t,0,s,turn(b,t)["Attack"].toInt());
	    return;
	}
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
	b.inflictDamage(t, poke(b,s)["Level"].toInt(), s, true);
    }
};

struct MMAromaTherapy : public MM
{
    MMAromaTherapy() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	int move = MM::move(b,s);
	b.sendMoveMessage(3, (move == 16) ? 0 : 1, s, type(b,s));
	for (int i = 0; i < 6; i++) {
            //SoundProof blocks healbell but not aromatherapy
            if (!b.poke(s,i).ko() && (move == 16 || b.poke(s,i).ability() != 95)) {
		b.changeStatus(s,i,Pokemon::Fine);
	    }
	}
    }
};

struct MMAttract : public MM
{
    MMAttract() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (!b.isSeductionPossible(s,t) || poke(b,t).contains("AttractedTo")){
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas (int s, int t, BS &b) {
	poke(b,t)["AttractedTo"] = s;
	poke(b,s)["Attracted"] = t;
	addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);
	b.sendMoveMessage(58,1,s,0,t);
	if (b.hasWorkingItem(s, 17)) /* mental herb*/ {
	    b.sendItemMessage(7,s);
            poke(b,t).remove("Attracted");
	    b.disposeItem(t);
	}
    }

    static void pda(int s, int, BS &b) {
	if (poke(b,s).contains("AttractedTo")) {
	    int seducer = poke(b,s)["AttractedTo"].toInt();
	    if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
		b.sendMoveMessage(58,0,s,0,seducer);
                if (true_rand() % 2 == 0) {
		    turn(b,s)["ImpossibleToMove"] = true;
		    b.sendMoveMessage(58, 2,s);
		}
	    }
	}
    }
};

struct MMThunderWave : public MM
{
    MMThunderWave() {
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
	if (b.hasType(t, Pokemon::Ground)) {
	    turn(b,s)["Failed"] = true;
	}
    }
};

struct MMKnockOff : public MM
{
    MMKnockOff() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s,int t,BS &b)
    {
	if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, 101) && !b.hasWorkingAbility(t, 59)) /* Sticky Hold, MultiType */
	{
	    b.sendMoveMessage(70,0,s,type(b,s),t,b.poke(t).item());
	    b.disposeItem(t);
	}
    }
};

struct MMCovet : public MM
{
    MMCovet() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s,int t,BS &b)
    {
	if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, 101) && !b.hasWorkingAbility(t, 59) && b.poke(s).item() == 0) /* Sticky Hold, MultiType */
	{
	    b.sendMoveMessage(23,(move(b,s)==71)?0:1,s,type(b,s),t,b.poke(t).item());
	    b.acqItem(s, b.poke(t).item());
	    b.disposeItem(t);
	}
    }
};

struct MMSwitcheroo : public MM
{
    MMSwitcheroo() {
	functions["UponAttackSuccessful"] = &uas;
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
	if (b.koed(t) || (b.poke(t).item() == 0 && b.poke(s).item() == 0) || b.hasWorkingAbility(t, 101) || b.hasWorkingAbility(t, 59) ) /* Sticky Hold, MultiType */
	{
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int t, BS &b)
    {
	b.sendMoveMessage(132,0,s,type(b,s),t);
	int i1(b.poke(s).item()), i2(b.poke(t).item());
	b.disposeItem(s);
	b.disposeItem(t);
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
	if (!b.battlelong.contains("LastMoveSuccessfullyUsed") || b.battlelong["LastMoveSuccessfullyUsed"].toInt() == 67) {
	    turn(b,s)["Failed"] = true;
	} else {
	    turn(b,s)["CopycatMove"] = b.battlelong["LastMoveSuccessfullyUsed"];
	}
    }

    static void uas(int s, int t, BS &b) {
	removeFunction(turn(b,s), "UponAttackSuccessful", "CopyCat");
	removeFunction(turn(b,s), "DetermineAttackFailure", "CopyCat");
	int attack = turn(b,s)["CopycatMove"].toInt();
	MoveEffect::setup(attack, s, t, b);
	b.useAttack(s, turn(b,s)["CopycatMove"].toInt(), true);
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
	     /* * No Move
		* Assist
		* Chatter
		* Copycat
		* Counter
		* Covet
		* Destiny Bond
		* Detect
		* Endure
		* Feint
		* Focus Punch
		* Follow Me
		* Helping Hand
		* Me First
		* Metronome
		* Mimic
		* Mirror Coat
		* Mirror Move
		* Protect
		* Sketch
		* Sleep Talk
		* Snatch
		* Struggle
		* Switcheroo
		* Thief
		* Trick */
	    (*this) << 0 << 17 << 58 << 70 << 71 << 85 << 86 << 116 << 128 << 145 << 146 << 182 << 233 << 245 << 247 << 251 << 252 << 296 << 358 << 367 << 373 << 394
		<< 409 << 420 << 434;
	}
    };
    static FM forbidden_moves;

    static void uas(int s, int t, BS &b)
    {
	removeFunction(turn(b,s), "UponAttackSuccessful", "Assist");
	removeFunction(turn(b,s), "DetermineAttackFailure", "Assist");
	int attack = turn(b,s)["AssistMove"].toInt();
	MoveEffect::setup(attack, s, t, b);
	b.useAttack(s, turn(b,s)["AssistMove"].toInt(), true);
    }

    static void daf(int s, int, BS &b)
    {
	QSet<int> possible_moves;
	for (int i = 0; i < 6; i++) {
	    if (i != b.currentPoke(s) && b.poke(s, i).num() != 0) {
		PokeBattle &p = b.poke(s,i);
		for(int j = 0; j < 4; j++) {
		    int m = p.move(j);
		    if (!forbidden_moves.contains(m))
			possible_moves.insert(m);
		}
	    }
	}
	if (!possible_moves.empty()) {
            turn(b,s)["AssistMove"] = *(possible_moves.begin() + (true_rand() %possible_moves.size()));
	} else {
	    turn(b,s)["Failed"] = true;
	}
    }
};

MMAssist::FM MMAssist::forbidden_moves;

struct MMBide : public MM
{
    MMBide() {
	functions["OnSetup"] = &os;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int , BS &b) {
	addFunction(poke(b,s), "TurnSettings", "Bide", &ts);
	addFunction(turn(b,s), "UponOffensiveDamageReceived", "Bide", &udi);
	poke(b,s)["BideDamageCount"] = 0;
	poke(b,s)["BideTurn"] = b.turn();
	b.sendMoveMessage(9,0,s,type(b,s));
    }

    static void udi(int s, int, BS &b) {
	inc(poke(b,s)["BideDamageCount"],poke(b,s)["DamageTakenByAttack"].toInt());
    }

    static void daf(int s, int, BS &b) {
	if (poke(b,s)["BideDamageCount"].toInt() == 0) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void ms(int s, int, BS &b) {
	b.sendMoveMessage(9,1,s,type(b,s));
    }

    static void uas2(int s, int, BS &b) {
	b.sendMoveMessage(9,0,s,type(b,s));
    }

    static void os(int s, int, BS &b) {
	turn(b,s)["Power"] = 0;
    }

    static void ts(int s, int, BS &b) {
	int _turn = poke(b,s)["BideTurn"].toInt();
	if (_turn + 2 < b.turn()) {
	    return;
	} else {
	    addFunction(turn(b,s),"UponOffensiveDamageReceived", "Bide", &udi);
	    turn(b,s)["SpeedPriority"] = 1;
	    turn(b,s)["NoChoice"] = true;
	    turn(b,s)["TellPlayers"] = false;
	    if (_turn +1 == b.turn()) {
		turn(b,s)["PossibleTargets"] = Move::None;
		turn(b,s)["Power"] = 0;
		addFunction(turn(b,s), "UponAttackSuccessful", "Bide", &uas2);
	    } else {
		turn(b,s)["PossibleTargets"] = Move::ChosenTarget;
		turn(b,s)["Power"] = 1;
		turn(b,s)["Type"] = Pokemon::Curse;
		addFunction(turn(b,s), "BeforeTargetList", "Bide", &ms);
		addFunction(turn(b,s), "CustomAttackingDamage", "Bide", &ccd);
		addFunction(turn(b,s), "DetermineAttackFailure", "Bide",&daf);
	    }
	}
    }

    static void ccd(int s, int t, BS &b) {
	b.inflictDamage(t, 2*poke(b,s)["BideDamageCount"].toInt(),s,true);
    }
};

struct MMBind : public MM
{
    MMBind() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
	poke(b,t)["TrappedBy"] = s;
        poke(b,t)["BindedBy"] = s;
        poke(b,t)["TrappedCount"] = b.poke(s).item() == 190 ? 5 : (true_rand()%4) + 2; /* Grip claw = 5 turns */
	poke(b,t)["TrappedMove"] = move(b,s);
	poke(b,s)["Trapped"] = t;
        poke(b,s)["Binded"] = t;
	addFunction(poke(b,t), "EndTurn", "Bind", &et);
    }

    static void et (int s, int, BS &b) {
	int count = poke(b,s)["TrappedCount"].toInt() - 1;
	int move = poke(b,s)["TrappedMove"].toInt();
        int t = b.rev(s);
        if (t != poke(b,s).value("BindedBy").toInt() || b.koed(t) || b.koed(s) || !poke(b,t).contains("Binded") || poke(b,t)["Binded"] != s) {
            poke(b,s).remove("TrappedBy");
            removeFunction(poke(b,s),"EndTurn", "Bind");
            return;
        }
	if (count <= 0) {
	    poke(b,s).remove("TrappedBy");
	    removeFunction(poke(b,s),"EndTurn", "Bind");
	    b.sendMoveMessage(10,1,s,MoveInfo::Type(move),s,move);
	} else {
	    poke(b,s)["TrappedCount"] = count;
	    b.sendMoveMessage(10,0,s,MoveInfo::Type(move),s,move);
            b.inflictDamage(s, b.poke(s).totalLifePoints()/16,s,false);
	}
    }
};

struct MMBounce : public MM
{
    MMBounce() {
	functions["UponAttackSuccessful"] = &uas;
	functions["OnSetup"] = &os;
	functions["MoveSettings"] = &ms;
    }

    static void os(int s, int, BS &b) {
	turn(b,s)["TellPlayers"] = false;
    }

    static void ms(int s, int, BS &b) {
	addFunction(poke(b,s), "TurnSettings", "Bounce", &ts);
	poke(b,s)["2TurnMove"] = turn(b,s)["Attack"];
	turn(b,s)["Power"] = 0;
	turn(b,s)["Accuracy"] = 0;
	turn(b,s)["PossibleTarget"] = Move::None;
    }

    static void ts(int s, int, BS &b) {
	turn(b,s)["NoChoice"] = true;
	merge(turn(b,s), MoveEffect(poke(b,s)["2TurnMove"].toInt()));
	addFunction(turn(b,s), "EvenWhenCantMove", "Bounce", &bes);
	removeFunction(poke(b,s), "TurnSettings", "Bounce");
    }

    static void bes(int s, int, BS &b) {
	poke(b,s)["Invulnerable"] = false;
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
	b.sendMoveMessage(13,args[0].toInt(),s,type(b,s));
	addFunction(b.battlelong, "DetermineGeneralAttackFailure", "Bounce", &dgaf);
    }

    static void dgaf(int s, int t, BS &b) {
	if (s == t || t == -1) {
	    return;
	}
	if (!poke(b,t).value("Invulnerable").toBool()) {
	    return;
	}
	int attack = turn(b,s)["Attack"].toInt();
	/* Lets see if the poke is vulnerable to that one attack */
	QList<int> vuln_moves = poke(b,t)["VulnerableMoves"].value<QList<int> >();
	QList<int> vuln_mults = poke(b,t)["VulnerableMults"].value<QList<int> >();

	for (int i = 0; i < vuln_moves.size(); i++) {
	    if (vuln_moves[i] == attack) {
		turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * vuln_mults[i];
		return;
	    }
	}

	/* All other moves fail */
	turn(b,s)["Failed"] = true;
    }
};

struct MMCounter : public MM
{
    MMCounter() {
	functions["MoveSettings"] = &ms;
	functions["DetermineAttackFailure"] = &daf;
	functions["CustomAttackingDamage"] = &cad;
    }

    static void ms (int s, int, BS &b) {
	turn(b,s)["PossibleTargets"] = Move::ChosenTarget;
    }

    static void daf (int s, int t, BS &b) {
	int dam = turn(b,s).value("DamageTakenByAttack").toInt();
	if (dam == 0) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	QStringList args = turn(b,s)["Counter_Arg"].toString().split('_');
	if (args[0].length() == 1 && turn(b,t)["Category"].toInt() != args[0].toInt()) {
	    turn(b,s)["Failed"] = true;
	}
	turn(b,s)["CounterDamage"] = dam * args[1].toInt() / 2;
    }

    static void cad(int s, int t, BS &b) {
	b.inflictDamage(t, turn(b,s)["CounterDamage"].toInt(), s, true);
    }
};

struct MMTaunt : public MM
{
    MMTaunt() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (poke(b,t)["TauntsUntil"].toInt() >= b.turn())
            turn(b,s)["Failed"] = true;
    }

    static void uas (int s, int t, BS &b) {
        poke(b,t)["TauntsUntil"] = b.turn() + 1 + (true_rand()%4);
	addFunction(poke(b,t), "MovesPossible", "Taunt", &msp);
	addFunction(turn(b,t), "MovePossible", "Taunt", &mp);
	b.sendMoveMessage(134,1,s,Pokemon::Dark,t);
    }

    static void msp(int s, int, BS &b) {
	int tt = poke(b,s)["TauntsUntil"].toInt();
	if (tt < b.turn()) {
	    return;
	}
	for (int i = 0; i < 4; i++) {
	    if (MoveInfo::Power(b.move(s,i)) == 0) {
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
	if (MoveInfo::Power(move) == 0) {
	    turn(b,s)["ImpossibleToMove"] = true;
	    b.sendMoveMessage(134,0,s,Pokemon::Dark,s,move);
	}
    }
};

struct MMDisable : public MM
{
    MMDisable() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b)
    {
	if (poke(b,t).contains("DisablesUntil") && poke(b,t).value("DisablesUntil").toInt() >= b.turn())
	{
	    turn(b,s)["Failed"] = true;
	    return;
	}
	if (!poke(b,t).contains("LastMoveSuccessfullyUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int tu = poke(b,t)["LastMoveSuccessfullyUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int move = poke(b,t)["LastMoveSuccessfullyUsed"].toInt();
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
        poke(b,t)["DisablesUntil"] = b.turn() + 3 + (true_rand()%4);
	poke(b,t)["DisabledMove"] = poke(b,t)["MoveSlot"];
	addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
	addFunction(turn(b,t), "MovePossible", "Disable", &mp);
	addFunction(poke(b,t), "EndTurn", "Disable", &et);
	b.sendMoveMessage(28,0,s,0,t,b.move(t,poke(b,t)["MoveSlot"].toInt()));
    }

    static void et (int s, int, BS &b)
    {
	int tt = poke(b,s)["DisablesUntil"].toInt();
	if (tt <= b.turn()) {
	    removeFunction(poke(b,s), "MovesPossible", "Disable");
	    b.sendMoveMessage(28,2,s);
	}
    }

    static void msp(int s, int, BS &b) {
	turn(b,s)["Move" + QString::number(poke(b,s)["DisabledMove"].toInt()) + "Blocked"] = true;
    }

    static void mp(int s, int, BS &b) {
	if(poke(b,s)["MoveSlot"] == poke(b,s)["DisabledMove"]) {
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
	turn(b,s)["Type"] = Pokemon::Curse;
	turn(b,s)["Power"] = 1;
    }

    static void daf(int s, int t, BS &b) {
	if (team(b,t).contains("DoomDesireTurn") && team(b,s)["DoomDesireTurn"].toInt() >= b.turn()) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void cad(int s, int t, BS &b) {
	int move = MM::move(b,s);
	turn(b,s)["CriticalHit"] = false;
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * MoveInfo::Power(move);
	team(b,t)["DoomDesireDamage"] = b.calculateDamage(s, t);
	team(b,t)["DoomDesireTurn"] = b.turn() + 2;
	team(b,t)["DoomDesireMove"] = move;
	addFunction(team(b,t), "EndTurn", "DoomDesire", &et);
	b.sendMoveMessage(29, 1, s, type(b,s));
    }

    static void et (int s, int, BS &b) {
	if (b.turn() == team(b,s).value("DoomDesireTurn"))
	{
	    if (!b.koed(s)) {
		int move = team(b,s)["DoomDesireMove"].toInt();
		b.sendMoveMessage(29,0,s,MoveInfo::Type(move),s,move);
		b.inflictDamage(s,team(b,s)["DoomDesireDamage"].toInt(), s, false); /*false is weird, it's so endure doesn't work */
	    }
	    removeFunction(team(b,s), "EndTurn", "DoomDesire");
	}
    }
};

struct MMEmbargo : public MM
{
    MMEmbargo() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.sendMoveMessage(32,0,s,type(b,s),t);
	poke(b,t)["Embargoed"] = true;
	poke(b,t)["EmbargoEnd"] = b.turn() + 4;
	addFunction(poke(b,t), "EndTurn", "Embargo", &et);
    }

    static void et(int s, int , BS &b) {
	if (poke(b,s).value("Embargoed").toBool() && poke(b,s)["EmbargoEnd"].toInt() <= b.turn()) {
	    b.sendMoveMessage(32,1,s,0);
	    removeFunction(poke(b,s), "EndTurn", "Embargo");
	}
    }
};

struct MMEncore : public MM
{
    MMEncore() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    struct FM : public QSet<int>
    {
	FM() {
	    /* Encore , Mimic Mirror Move, Sketch,  Struggle Transform, ,  */
	    (*this)  << 114 << 247 << 252 << 358 << 394 << 432;
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
	if (!poke(b,t).contains("LastMoveSuccessfullyUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int tu = poke(b,t)["LastMoveSuccessfullyUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int move = poke(b,t)["LastMoveSuccessfullyUsed"].toInt();
	if (forbidden_moves.contains(move)) {
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
        poke(b,t)["EncoresUntil"] = b.turn() + 3 + (true_rand()%5);
	poke(b,t)["EncoresMove"] = poke(b,t)["LastMoveSuccessfullyUsed"];
	addFunction(poke(b,t), "MovesPossible", "Encore", &msp);
	addFunction(turn(b,t), "MovePossible", "Encore", &mp);
	addFunction(poke(b,t), "EndTurn", "Encore", &et);
	b.sendMoveMessage(33,2,s,0,t,poke(b,t)["LastMoveSuccessfullyUsed"].toInt());
    }

    static void et (int s, int, BS &b)
    {
    	for (int i = 0; i < 4; i++) {
	    if (b.move(s,i) == poke(b,s)["EncoresMove"].toInt()) {
		if (b.poke(s).move(i).PP() <= 0) {
		    removeFunction(poke(b,s), "MovesPossible", "Encore");
		    removeFunction(poke(b,s), "EndTurn", "Encore");
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
	    removeFunction(poke(b,s), "EndTurn", "Encore");
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

    static void mp(int s, int, BS &b) {
	if(b.move(s,poke(b,s)["MoveSlot"].toInt()) != poke(b,s)["EncoresMove"].toInt()) {
	    turn(b,s)["ImpossibleToMove"] = true;
	    b.sendMoveMessage(33,1,s,0,s,b.move(s, poke(b,s)["MoveSlot"].toInt()));
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
            if (true_rand()%2 == 0) {
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
	addFunction(turn(b,s), "UponOffensiveDamageReceived", "Endure", &uodr);
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
	if (turn(b,s)["Power"].toInt() > 0) {
            inc(turn(b,s)["CriticalRaise"], 2);
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
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * (poke(b,s)["FuryCutterCount"].toInt()+1);
    }
};

struct MMGastroAcid : public MM
{
    MMGastroAcid() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.sendMoveMessage(51,0,s,type(b,s),t);
	poke(b,t)["AbilityNullified"] = true;
    }
};

struct MMGravity : public MM
{
    MMGravity() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.battlelong["Gravity"] = true;
	b.battlelong["GavityCount"] = 5;
	b.sendMoveMessage(53,0,s,type(b,s));
	addFunction(b.battlelong, "EndTurn", "Gravity", &et);
    }

    static void et(int s, int, BS &b) {
	if (b.battlelong.value("Gravity").toBool()) {
	    int count = b.battlelong["GavityCount"].toInt() - 1;
	    if (count <= 0) {
		b.sendMoveMessage(53,1,s,Pokemon::Psychic);
		removeFunction(b.battlelong, "EndTurn", "Gravity");
		b.battlelong["Gravity"] = false;
	    }
	}
    }
};

struct MMGrassKnot : public MM
{
    MMGrassKnot() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
	float weight = poke(b,t)["Weight"].toDouble();
	int bp;
	/* I had to make some hacks due to the floating point precision, so this is a '<' here and not
	   a '<='. Will be fixed if someone wants to do it */
	if (weight < 10.0f) {
	    bp = 20;
	} else if (weight < 25.0f) {
	    bp = 40;
	} else if (weight < 50.0f) {
	    bp = 60;
	} else if (weight < 100.0f) {
	    bp = 80;
	} else if (weight < 200.0f) {
	    bp = 100;
	} else {
	    bp = 120;
	}
	turn(b,s)["Power"] = bp;
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
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	QStringList args = poke(b,s)["BoostSwap_Arg"].toString().split('_');
	foreach(QString str, args) {
	    std::swap(poke(b,s)["Boost"+str], poke(b,t)["Boost"+str]);
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
	int bp = 1 + 25 * b.getStat(t,Speed) / b.getStat(s,Speed);
	bp = std::max(2,std::min(bp,150));

	turn(b,s)["Power"] = bp;
    }
};

struct MMWeather : public MM
{
    MMWeather() {
	functions["UponAttackSuccessful"] = &uas;
    }

    struct WI : public QMap<int,int> {
	WI() {
	    insert(3,33); /* Soft Rock */
	    insert(1,155); /* Icy Rock */
	    insert(2,160); /* Damp Rock */
	    insert(4, 161); /* Heat Rock */
	}
    };
    static WI weather_items;

    static void uas(int s, int, BS &b) {
	int weather = turn(b,s)["Weather_Arg"].toInt();

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
	if (b.isWeatherWorking(BattleSituation::Hail)) {
	    turn(b,s)["Accuracy"] = turn(b,s)["Accuracy"].toInt() * 10 / 7;
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
	    turn(b,s)["Accuracy"] = turn(b,s)["Accuracy"].toInt() * 10 / 7;
	} else if (b.isWeatherWorking(BattleSituation::Sunny)) {
	    turn(b,s)["Accuracy"] = turn(b,s)["Accuracy"].toInt() * 5 / 7;
	}
    }
};

struct MMUnThawing : public MM
{
    MMUnThawing() {
	functions["EvenWhenCantMove"] = &ewcm;
    }

    static void ewcm(int s, int, BS &b) {
	if (b.poke(s).status() == Pokemon::Frozen) {
	    b.notify(BattleSituation::All, BattleSituation::StatusMessage, s, qint8(BattleSituation::FreeFrozen));
	    b.healStatus(s, Pokemon::Frozen);
	}
    }
};

struct MMWeatherBall : public MM
{
    MMWeatherBall() {
	functions["MoveSettings"] = &ms;
    }

    static void ms (int s, int, BS &b) {
	int weather = b.weather();

	if (weather != BattleSituation::NormalWeather && b.isWeatherWorking(weather)) {
	    turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * 2;
            turn(b,s)["Type"] = TypeInfo::TypeForWeather(weather);
	}
    }
};

struct MMHealingWish : public MM
{
    MMHealingWish() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	addFunction(turn(b,s), "AfterSwitchIn", "HealingWish", &asi);
    }

    static void asi(int s, int, BS &b) {
	if (!b.koed(s)) {
	    b.sendMoveMessage(61,0,s,type(b,s));
	    b.healLife(s,b.poke(s).totalLifePoints());
	    b.changeStatus(s, Pokemon::Fine);
	}
    }
};

struct MMPowerTrick : public MM
{
    MMPowerTrick() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	std::swap(poke(b,s)["Stat1"], poke(b,s)["Stat2"]);
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
	functions["UponAttackSuccessful"] = &uas;
    }
    static void uas(int s, int t, BS &b) {
	poke(b,t)["HealBlockCount"] = 5;
	addFunction(poke(b,t), "EndTurn", "HealBlock", &et);
	addFunction(turn(b,t), "MovePossible", "HealBlock", &mp);
	addFunction(poke(b,t), "MovesPossible", "HealBlock", &msp);
	b.sendMoveMessage(59,0,s,type(b,s),t);
    }
    static void et(int s, int , BS &b) {
	inc(poke(b,s)["HealBlockCount"], -1);
	int count = poke(b,s)["HealBlockCount"].toInt();

	if (count == 0) {
	    b.sendMoveMessage(59,2,s,Move::Psychic);
	    removeFunction(poke(b,s), "EndTurn", "HealBlock");
	    removeFunction(poke(b,s), "MovesPossible", "HealBlock");
	}
    }

    struct FM : public QSet<int> {
	FM() {
	    /* Heal Order, Milk Drink, Moonlight, Morning Sun, Recover, Rest, Roost, Slack Off, Softboiled, Swallow, Synthesis, and Wish; */
	    (*this) << 178 << 246 << 256 << 257 << 312 << 316 << 333 << 363 << 375 << 405 << 411 << 458;
	}
    };
    static FM forbidden_moves;

    static void msp(int s, int, BS &b) {
	for (int i = 0; i < 4; i++) {
	    if (forbidden_moves.contains(b.move(s,i))) {
		turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
	    }
	}
    }

    static void mp(int s, int, BS &b) {
	if(forbidden_moves.contains(b.move(s,poke(b,s)["MoveSlot"].toInt()))) {
	    turn(b,s)["ImpossibleToMove"] = true;
	    b.sendMoveMessage(59,1,s,Move::Psychic,s,b.move(s, poke(b,s)["MoveSlot"].toInt()));
	}
    }
};

MMHealBlock::FM MMHealBlock::forbidden_moves;

struct MMFling : public MM
{
    MMFling() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
	functions["BeforeTargetList"] = &btl;
    }

    static void daf(int s, int, BS &b) {
	if (!turn(b,s).contains("FlingItem")) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void btl(int s, int, BS &b) {
	if (b.poke(s).item() != 0 && b.hasWorkingItem(s, b.poke(s).item())) {
	    turn(b,s)["FlingItem"] = b.poke(s).item();
	    turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * ItemInfo::Power(b.poke(s).item());
	    b.disposeItem(s);
	}
    }

    static void uas (int s, int t, BS &b) {
	int item = turn(b,s)["FlingItem"].toInt();
	switch (item) {
            case 71: b.inflictStatus(t, Pokemon::Burnt, s); break; /*flame orb*/
            case 141: b.inflictStatus(t, Pokemon::DeeplyPoisoned, s); break; /*toxic orb*/
	    case 87: case 118: turn(b,t)["Flinched"] = true; break; /* king rock, razor fang */
            case 92: b.inflictStatus(t, Pokemon::Paralysed, s); break; /* light ball */
            case 166: b.inflictStatus(t, Pokemon::Poisoned, s); break; /* poison barb */
	    case 17: case 37: /* mental herb, white herb */
		int oppitem = b.poke(t).item();
		ItemEffect::activate("AfterSetup", item, t,s,b);
		b.poke(t).item() = oppitem; /* the effect of mental herb / white herb may have disposed of the foes item */
		break;
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
	    typemod = TypeInfo::Eff(type, typeadv[1]);
	} else if (typeadv[1] == Pokemon::Ghost) {
	    typemod = TypeInfo::Eff(type, typeadv[0]);
	} else {
	    typemod = TypeInfo::Eff(type, typeadv[0]) * TypeInfo::Eff(type, typeadv[1]);
	}
	turn(b,s)["TypeMod"] = typemod;
	int damage = std::min(b.calculateDamage(s,t), b.poke(t).totalLifePoints()/2);
	b.sendMoveMessage(64,0,s,Move::Fighting);
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
	if (poke(b,s).contains("DefenseCurl")) {
	    turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * 2;
	}
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * (1+poke(b,s)["IceBallCount"].toInt());
    }

    static void uas(int s, int, BS &b) {
	int count = poke(b,s)["IceBallCount"].toInt();
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
	    MoveEffect::setup(poke(b,s)["LastMoveUsed"].toInt(),s,t,b);
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
	int foe = b.rev(s);

	if (b.koed(foe)) {
	    turn(b,s)["Failed"] = true;
	    return;
	}

	bool success = false;

	for (int i = 0; i < 4; i++)
	    if (b.move(s,i) != 0)
		for (int j = 0; j < 4; j++)
		    if (b.move(foe,j) == b.move(s,i))
			success = true;

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
	int foe = b.rev(s);

	if (!poke(b,foe).value("Imprisoner").toBool()) {
	    return;
	}

	int attack = move(b,s);

	for (int i = 0; i < 4; i++) {
	    if (b.move(foe,i) == attack) {
		turn(b,s)["ImpossibleToMove"] = true;
		b.sendMoveMessage(67,1,s,Pokemon::Psychic,foe,attack);
		return;
	    }
	}
    }

    static void msp(int s, int, BS &b) {
	/* let's just see if there are moves to imprison */
	int foe = b.rev(s);

	if (!poke(b,foe).value("Imprisoner").toBool()) {
	    return;
	}

	for (int i = 0; i < 4; i++)
	    if (b.move(s,i) != 0)
		for (int j = 0; j < 4; j++)
		    if (b.move(foe,j) == b.move(s,i))
			turn(b,s)["Move"+QString::number(i) + "Blocked"] = true;

    }
};

struct MMMagnetRise : public MM
{
    MMMagnetRise() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int, BS &b) {
	if (b.hasWorkingAbility(s,48) /* levitate */ || poke(b,s).value("Rooted").toBool()) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(68,0,s,Pokemon::Electric);
	poke(b,s)["MagnetRiseCount"] = 5;
	addFunction(poke(b,s), "EndTurn", "MagnetRise", &et);
    }

    static void et(int s, int, BS &b) {
	inc(poke(b,s)["MagnetRiseCount"], -1);
	int count = poke(b,s)["MagnetRiseCount"].toInt();

	if (count == 0) {
	    b.sendMoveMessage(68,1,s,Move::Electric);
	    removeFunction(poke(b,s), "EndTurn", "MagnetRise");
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
	    turn(b,s)["Type"] = poke(b,s)["ItemArg"];
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
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	int nturn;
	if (b.hasWorkingItem(s, 93)) { /* light clay */
	    nturn = 8;
	} else {
	    nturn = 5;
	}
	int cat = turn(b,s)["TeamBarrier_Arg"].toInt();

	b.sendMoveMessage(73,cat,s,type(b,s));
	team(b,s)["Barrier" + QString::number(cat) + "Count"] = nturn;

	addFunction(team(b,s), "EndTurn", "TeamBarrier", &et);
    }

    static void et(int s, int, BS &b) {
	int counts[] = {team(b,s).value("Barrier0Count").toInt(), team(b,s).value("Barrier1Count").toInt()};

	for (int i = 0; i < 2; i++) {
	    if (counts[i] != 0) {
		team(b,s)["Barrier" + QString::number(i) + "Count"] = counts[i] - 1;
		if (counts[i] == 1) {
		    b.sendMoveMessage(73, 2+i,s,Pokemon::Psychic);
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
	if (team(b,t).value("Barrier0Count").toInt() > 0 || team(b,t).value("Barrier1Count").toInt() > 0) {
	    b.sendMoveMessage(14,0,s,Pokemon::Fighting);
	    team(b,t)["Barrier0Count"] = 0;
	    team(b,t)["Barrier1Count"] = 0;
	}
    }
};

struct MMLockOn : public MM
{
    MMLockOn() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	poke(b,t)["LockedOnEnd"] = b.turn() + 2;
	poke(b,s)["LockedOn"] = t;
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
	team(b,s)["LuckyChantCount"] = 5;
	addFunction(team(b,s), "EndTurn", "LuckyChant", &et);
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
	addFunction(b.battlelong, "DetermineGeneralAttackFailure", "MagicCoat", &dgaf);
	turn(b,s)["MagicCoated"] = true;
	b.sendMoveMessage(76,0,s,Pokemon::Psychic);
    }

    /*	# Attract
	# Block
	# Gastro Acid
	# Leech Seed
	# Mean Look
	# Spider Web
	# Worry Seed
	# Yawn
    */
    struct BM : public QSet<int> {
	BM() { (*this) << 21 << 36 << 155 << 216 << 234 << 461 << 465; }
    };

    static BM bounced_moves;

    static void dgaf(int s, int t, BS &b) {
	if (turn(b,t).value("MagicCoated").toBool()) {
	    if (turn(b,s)["Power"].toInt() == 0 && turn(b,s)["PossibleTargets"].toInt() == Move::ChosenTarget) {
		int move = MM::move(b,s);
		/* Typically, the moves that are bounced back are moves that only induce status / boost mods and nothing else,
		    therefore having no "SpecialEffect". Exceptions are stored in bounced_moves */
		if (MoveInfo::SpecialEffect(move).size() == 0|| bounced_moves.contains(move)) {
		    b.fail(s,76,1,Pokemon::Psychic);
		    /* Now Bouncing back ... */
		    removeFunction(turn(b,t), "UponAttackSuccessful", "MagicCoat");
		    MoveEffect::setup(move,t,s,b);
		    b.useAttack(t,move,true,false);
		}
	    }
	}
    }
};

MMMagicCoat::BM MMMagicCoat::bounced_moves;

struct MMDefog : public MM
{
    MMDefog() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
	team(b,t).remove("Barrier0Count");
	team(b,t).remove("Barrier1Count");
	team(b,t).remove("Spikes");
	team(b,t).remove("ToxicSpikes");
	team(b,t).remove("StealthRock");
	team(b,t).remove("MistCount");
	b.sendMoveMessage(77,0,s,type(b,s),t);
    }
};

struct MMMagnitude: public MM
{
    MMMagnitude() {
	functions["BeforeCalculatingDamage"] = &bcd;
	functions["BeforeHitting"] = &bh;
    }

    static void bcd(int s, int, BS &b) {
        int randnum = true_rand()%20;

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
	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * pow;
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
	int num = turn(b,t).value("Attack").toInt();
	if (MoveInfo::Power(num) == 0) {
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
	b.useAttack(s,move,true,true);
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
            int move = true_rand() % MoveInfo::NumberOfMoves();

	    if (!b.hasMove(s,move) && !MMAssist::forbidden_moves.contains(move)) {
		MoveEffect::setup(move,s,t,b);
		b.useAttack(s,move,true,true);
		break;
	    }
	}
    }
};

struct MMMimic : public MM
{
    MMMimic() {
	functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
	if (!poke(b,t).contains("LastMoveSuccessfullyUsedTurn")) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int tu = poke(b,t)["LastMoveSuccessfullyUsedTurn"].toInt();
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMoved").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
	int move = poke(b,t)["LastMoveSuccessfullyUsed"].toInt();
	if (b.hasMove(s,move) || move == 245 /*metronome*/) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas(int s, int t, BS &b) {
	int move = poke(b,t)["LastMoveSuccessfullyUsed"].toInt();
	poke(b,s)["Move" + QString::number(poke(b,s)["MoveSlot"].toInt())] = move;
	b.changePP(s,move,5);
	b.sendMoveMessage(81,0,s,type(b,s),t,move);
    }
};

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
	poke(b,t)[poke(b,s)["MiracleEye_Arg"].toString() + "Sleuthed"] = true;
	poke(b,t)["Sleuthed"] = true;
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
	int t = b.rev(s);

	removeFunction(turn(b,s), "DetermineAttackFailure", "MirrorMove");
	removeFunction(turn(b,s), "UponAttackSuccessful", "MirrorMove");

	int move = poke(b,s)["MirrorMoveMemory"].toInt();
	MoveEffect::setup(move,s,t,b);
	b.useAttack(s,move,true,true);
    }
};

struct MMMist : public MM
{
    MMMist() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(86,0,s,Pokemon::Ice);
	turn(b,s)["MistCount"] = 5;
	addFunction(team(b,s), "EndTurn", "Mist", &et);
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
	int weather = b.weather();

	if (weather == BattleSituation::NormalWeather || !b.isWeatherWorking(weather)) {
	    b.sendMoveMessage(87,0,s,type(b,s));
	    b.healLife(s, b.poke(s).totalLifePoints()/2);
	} else if (b.isWeatherWorking(BattleSituation::Sunny)) {
	    b.sendMoveMessage(87,0,s,type(b,s));
	    b.healLife(s, b.poke(s).totalLifePoints()*2/3);
	} else {
	    b.sendMoveMessage(87,0,s,type(b,s));
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
	b.sendMoveMessage(88, move == 260 ? 0 : 1, s, type(b,s));
	int type = poke(b,s)["MudSport_Arg"].toInt();
	poke(b,s)["Sported" + QString::number(type)] = true;
	b.battlelong["Sported"+ QString::number(type)] = s;
    }
};

struct MMNightMare : public MM
{
    MMNightMare() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int, int t, BS &b) {
	b.sendMoveMessage(92, 0, t, Pokemon::Ghost);
	poke(b,t)["HavingNightmares"] = true;
	addFunction(poke(b,t),"AfterStatusChange", "NightMare", &asc);
	addFunction(poke(b,t),"EndTurn", "NightMare", &et);
    }

    static void asc(int s, int, BS &b) {
	removeFunction(poke(b,s),"AfterStatusChange", "NightMare");
	removeFunction(poke(b,s),"EndTurn", "NightMare");
    }

    static void et(int s, int, BS &b) {
	if (!b.koed(s) && b.poke(s).status() == Pokemon::Asleep) {
	    b.sendMoveMessage(92,0,s,Pokemon::Ghost);
            b.inflictPercentDamage(s, 25, s, false);
	}
    }
};

struct MMOutrage : public MM
{
    MMOutrage() {
	functions["UponAttackSuccessful"] = &uas;
	functions["AfterAttackSuccessful"] = &aas;
    }

    static void uas(int s, int, BS &b) {
	int count = poke(b,s)["OutrageCount"].toInt();
	if (count == 0) {
            poke(b,s)["OutrageCount"] = 1 + (true_rand() % 2);
	    addFunction(poke(b,s), "TurnSettings", "Outrage", &ts);
	    poke(b,s)["OutrageMove"] = move(b,s);
	} else {
	    inc(poke(b,s)["OutrageCount"], -1);
	}
    }

    static void aas(int s, int, BS &b) {
	int count = poke(b,s)["OutrageCount"].toInt();
	if (count == 0) {
	    removeFunction(poke(b,s), "TurnSettings", "Outrage");
	    b.sendMoveMessage(93,0,s,type(b,s));
	    b.inflictConfused(s);
	} else {
	    poke(b,s)["NextOutrageTurn"] = b.turn() + 1;
	}
    }

    static void ts(int s, int, BS &b) {
	int count = poke(b,s)["OutrageCount"].toInt();
	if (count > 0 && poke(b,s).value("NextOutrageTurn").toInt() == b.turn()) {
	    turn(b,s)["NoChoice"] = true;
	    MoveEffect::setup(poke(b,s)["OutrageMove"].toInt(),s,b.rev(s),b);
	}
    }
};

struct MMPresent : public MM
{
    MMPresent() {
	functions["BeforeTargetList"] = &btl;
	functions["CustomAttackingDamage"] = &uas;
    }

    static void btl(int s, int, BS &b) {
        turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * 40 * (true_rand() % 4);
    }

    static void uas(int s, int t, BS &b) {
	b.sendMoveMessage(96,0,s,type(b,s),t);
	b.healLife(t, 80);
    }
};

struct MMPsychup : public MM
{
    MMPsychup() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b ) {
	b.sendMoveMessage(97,0,s,type(b,s),t);
	for (int i = 1; i <= 7; i++) {
	    QString boost = "Boost" + QString::number(i);
	    poke(b,s)[boost] = poke(b,t)[boost];
	}
    }
};

struct MMPsychoShift : public MM
{
    MMPsychoShift() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
	if (b.poke(s).status() == Pokemon::Fine || b.poke(t).status() != Pokemon::Fine)
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
        b.inflictDamage(t, poke(b,s)["Level"].toInt() * (5 + (true_rand() % 11)) / 10,s,true);
    }
};

struct MMRazorWind : public MM
{
    MMRazorWind() {
	functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
	if (!poke(b,s).contains("ReleaseTurn") || poke(b,s)["ReleaseTurn"].toInt() != b.turn()) {
	    if (b.hasWorkingItem(s, 22)) {
		//Power Herb
		b.sendItemMessage(11,s);
		b.disposeItem(s);
	    } else {
		int mv = move(b,s);
		/* razor wind, sky attack, skull bash */
		b.sendMoveMessage(104, mv == 311 ? 0 : (mv == 361 ? 1 : 2), s, type(b,s));
		/* Skull bash */
		if (mv == 360) {
		    b.gainStatMod(s,Defense,1);
		}
		poke(b,s)["ChargingMove"] = mv;
		poke(b,s)["ReleaseTurn"] = b.turn() + 1;
		turn(b,s)["TellPlayers"] = false;
		turn(b,s)["Power"] = 0;
		turn(b,s)["PossibleTargets"] = Move::None;
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
	MoveEffect::setup(poke(b,s)["ChargingMove"].toInt(),s,b.rev(s),b);
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
	    int temp = poke(b,t)["Boost" + QString::number(i)].toInt();
	    if (temp > 0) {
		boostsum += temp;
	    }
	}

	turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * std::min(60 + 20 * boostsum, 200);
    }
};

struct MMRage : public MM
{
    MMRage() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	poke(b,s)["RageTurn"] = b.turn();
	addFunction(poke(b,s), "UponOffensiveDamageReceived", "Rage", &uodr);
    }

    static void uodr(int s, int, BS &b) {
	int tt = poke(b,s)["RageTurn"].toInt();

	if (!b.koed(s) && (tt = b.turn() || (tt +1 == b.turn() && !turn(b,s).value("HasMoved").toBool()))) {
	    b.gainStatMod(s,Attack,1);
	}
    }
};

struct MMSafeGuard : public MM
{
    MMSafeGuard() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	b.sendMoveMessage(109,0,s,type(b,s));
	turn(b,s)["SafeGuardCount"] = 5;
	addFunction(team(b,s), "EndTurn", "SafeGuard", &et);
    }

    static void et(int s, int, BS &b) {
	if (team(b,s).value("SafeGuardCount") == 0) {
	    return;
	}

	inc(team(b,s)["SafeGuardCount"], -1);
	int count = team(b,s)["SafeGuardCount"].toInt();
	if (count == 0) {
	    b.sendMoveMessage(109,1,s,Pokemon::Psychic);
	}
    }
};

struct MMSketch : public MM
{
    MMSketch() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
	int move = poke(b,t)["LastMoveUsed"].toInt();
	/* Struggle, chatter */
        if (b.koed(t) || move == 394 || move == 146 || move == 0) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int t, BS &b) {
	int mv = poke(b,t)["LastMoveUsed"].toInt();
	b.sendMoveMessage(111,0,s,type(b,s),t,mv);
	int slot = poke(b,s)["MoveSlot"].toInt();
	b.poke(s).move(slot).num() = mv;
	b.poke(s).move(slot).load();
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
       * Assist
    * Bide
    * Bounce
    * Chatter
    * Copycat
    * Dig
    * Dive
    * Fly
    * Focus Punch
    * Me First
    * Metronome
    * Mirror Move
    * Shadow Force
    * Skull Bash
    * Sky Attack
    * Sleep Talk
    * SolarBeam
    * Razor Wind
    * Uproar
    * Any move the user cannot choose for use, including moves with zero PP
*/
	    (*this) << 17 << 30 << 41 << 58 << 67 << 87 << 90 << 142 << 145 << 233 << 245 << 252 << 349 << 360 << 361 << 367 << 376 << 311 << 441;
	}
    };

    static FM forbidden_moves;

    static void daf(int s, int, BS &b) {
	b.callpeffects(s, s, "MovesPossible");
	QList<int> mp;
	for (int i = 0; i < 4; i++) {
	    if (b.isMovePossible(s,i) && !forbidden_moves.contains(b.move(s,i))) {
		mp.push_back(i);
	    }
	}
	if (mp.size() == 0) {
	    turn(b,s)["Failed"] = true;
	} else {
            turn(b,s)["SleepTalkedMove"] = b.move(s, mp[true_rand()%mp.size()]);
	}
    }

    static void uas(int s, int, BS &b) {
	removeFunction(turn(b,s), "DetermineAttackFailure", "SleepTalk");
	removeFunction(turn(b,s), "UponAttackSuccessful", "SleepTalk");
	int mv = turn(b,s)["SleepTalkedMove"].toInt();
	MoveEffect::setup(mv,s,b.rev(s),b);
	b.useAttack(s, mv, true);
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
	if (b.poke(t).status() == turn(b,s)["SmellingSalt_Arg"].toInt()) {
	    turn(b,s)["Power"] = turn(b,s)["Power"].toInt();
	}
    }

    static void aas(int s, int t, BS &b) {
	if (!b.koed(t)) {
	    b.healStatus(t,turn(b,s)["SmellingSalt_Arg"].toInt());
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
    }

    /*	* All self-affecting stat ups (including Belly Drum and Defense Curl, but excluding Curse)

    * Aromatherapy
    * BellyDrum
    * Camouflage
    * Charge
    * Defense Curl
    * Heal Bell
    * Heal Order
    * Ingrain
    * Light Screen
    * Milk Drink
    * Minimize
    * Mist
    * Moonlight
    * Morning Sun
    * Psych Up
    * Recover
    * Reflect
    * Refresh
    * Rest
    * Roost
    * Safeguard
    * Slack Off
    * Softboiled
    * Stockpile
    * Substitute
    * Swallow
    * Synthesis
    * Tailwind
    */
    struct SM : public QSet<int> {
	SM() { (*this) << 16 << 29 << 53 << 55 << 82 << 176 << 178 << 202 << 219 << 246 << 256 << 257 << 298 << 312 << 314 << 315 << 316 << 333 << 335 << 363 << 375 << 389 << 397 << 405 << 411 << 415; }
    };

    static SM snatched_moves;

    static void dgaf(int s, int , BS &b) {
	if (b.battlelong.contains("Snatcher")) {
	    if (turn(b,s)["Power"].toInt() == 0) {
		int move = MM::move(b,s);
		/* Typically, the moves that are bounced back are moves that only induce status / boost mods and nothing else,
		    therefore having no "SpecialEffect". Exceptions are stored in bounced_moves */
		if (( turn(b,s)["PossibleTarget"].toInt() == Move::User && MoveInfo::SpecialEffect(move).size() == 0 )|| snatched_moves.contains(move)) {
		    int snatcher = b.battlelong["Snatcher"].toInt();
		    b.fail(s,118,0,type(b,snatcher));
		    /* Now Snatching ... */
		    removeFunction(turn(b,snatcher), "UponAttackSuccessful", "Snatch");
		    turn(b,snatcher).remove("Snatcher");
		    b.battlelong.remove("Snatcher");
		    MoveEffect::setup(move,snatcher,s,b);
		    b.useAttack(snatcher,move,true);
		}
	    }
	}
    }
};

MMSnatch::SM MMSnatch::snatched_moves;

struct MMSolarBeam : public MM
{
    MMSolarBeam() {
	functions["MoveSettings"] = &ms;
    }

    static void ms (int s, int, BS &b) {
	int count = poke(b,s).value("BeamChargingCount").toInt();
	if (count == 0) {
	    int weather = b.weather();
	    if (b.isWeatherWorking(weather) && weather != BattleSituation::NormalWeather) {
		if (weather == BattleSituation::Sunny) {
		    return;
		}
		poke(b,s)["BeamChargingCount"] = 2;
	    } else {
		poke(b,s)["BeamChargingCount"] = 1;
	    }
	    if (b.hasWorkingItem(s, 22)) {
		//Power Herb
		b.sendItemMessage(11,s);
		b.disposeItem(s);
		turn(b,s).remove("BeamChargingCount");
	    } else {
		turn(b,s)["TellPlayers"] = false;
		poke(b,s)["ReleaseTurn"] = b.turn() + poke(b,s)["BeamChargingCount"].toInt();
		turn(b,s)["Power"] = 0;
		turn(b,s)["PossibleTargets"] = Move::None;
		addFunction(poke(b,s), "TurnSettings", "SolarBeam", &ts);
		b.sendMoveMessage(119,0,s,Pokemon::Grass);
	    }
	} else if (count > 1) {
	    inc(poke(b,s)["BeamChargingCount"], -1);
	}
    }

    static void ts (int s, int, BS &b) {
	int count = poke(b,s).value("BeamChargingCount").toInt();
	if (count > 0 &&
		poke(b,s).value("ReleaseTurn").toInt() == b.turn() + count -1)
	{
	    turn(b,s)["NoChoice"] = true;
	    MoveEffect::setup(376,s,b.rev(s),b);
	} else {
	    poke(b,s).remove("BeamChargingCount");
	}
    }
};

struct MMSpite : public MM
{
    MMSpite(){
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf (int s, int t, BS &b) {
        if (!poke(b,t).contains("LastMoveSuccessfullyUsedTurn")) {
            turn(b,s)["Failed"] = true;
            return;
        }
        int tu = poke(b,t)["LastMoveSuccessfullyUsedTurn"].toInt();
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
        b.losePP(t,slot,4);
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
            turn(b,s)["Power"] = turn(b,s)["Power"].toInt() * 2;
        }
    }
};

struct MMStruggle : public MM
{
    MMStruggle() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        if (!b.koed(s)) {
            b.sendMoveMessage(127,0,s);
            b.inflictPercentDamage(s,25,s,true); //true cuz likely cuz magic guard doesn't prevent it
        }
    }
};

struct MMSuckerPunch : public MM
{
    MMSuckerPunch(){
        functions["DetermineAttackFailure"] = &daf;
    }

    static void daf(int s, int t, BS &b) {
        if (turn(b,t).value("HasMoved").toBool() || turn(b,t).value("Power").toInt() == 0) {
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
        team(b,s)["TailWindCount"] = 3;
        addFunction(team(b,s), "EndTurn", "TailWind", &et);
    }

    static void et(int s, int, BS &b) {
        if (team(b,s)["TailWindCount"].toInt() == 1) {
            removeFunction(team(b,s), "EndTurn", "TailWind");
            team(b,s)["TailWindCount"] = 0;
            b.sendMoveMessage(133,1,s,Pokemon::Flying);
        }
    }
};

struct MMTorment : public MM {
    MMTorment() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int t, BS &b) {
        poke(b,t)["Tormented"] = true;
        addFunction(poke(b,t), "MovesPossible", "Torment", &msp);
        addFunction(turn(b,t), "MovePossible", "Torment", &mp);
        b.sendMoveMessage(135,1,s,Pokemon::Dark,t);
    }

    static void msp(int s, int, BS &b) {
        for (int i = 0; i < 4; i++) {
            if (b.move(s,i) == poke(b,s)["LastMoveUsed"].toInt()) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }

    static void mp(int s, int, BS &b) {
        int move = turn(b,s)["MoveChosen"].toInt();
        if (move == poke(b,s)["LastMoveUsed"].toInt()) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendMoveMessage(135,0,s,Pokemon::Dark,s,move);
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
            removeFunction(b.battlelong, "EndTurn", "TrickRoom");
        } else {
            b.sendMoveMessage(138,0,s,Pokemon::Psychic);
            b.battlelong["TrickRoomCount"] = 5;
            addFunction(b.battlelong, "EndTurn", "TrickRoom", &et);
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
        functions["AfterAttackSuccessful"] = &aas;
    }

    static void bh(int s, int, BS &b) {
        b.sendMoveMessage(139,0,s,Pokemon::Fighting);
    }

    static void aas(int s, int, BS &b) {
        inc(turn(b,s)["TripleKickCount"], 1);
        int tkc = turn(b,s)["TripleKickCount"].toInt();
        if (tkc < 3) {
            turn(b,s)["Power"] = (tkc+1)*10;
            b.useAttack(s,move(b,s),true,false);
        }
    }
};

struct MMWorrySeed : public MM {
    MMWorrySeed() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Truant & multi-type */
        if (b.ability(t) == 59 || b.ability(t) == 115) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.acquireAbility(t, 42); //Insomnia
        b.sendMoveMessage(143,0,s,Pokemon::Grass,t);
    }
};

struct MMYawn : public MM {
    MMYawn() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        if (b.poke(t).status() != Pokemon::Fine || team(b,t).value("SafeGuardCount").toInt() > 0) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        if (poke(b,t).value("YawnCount").toInt() == 0) {
            b.sendMoveMessage(147,1,s,Pokemon::Normal,t);
            poke(b,t)["YawnCount"] = 2;
            addFunction(poke(b,t), "EndTurn", "Yawn", &et);
        }
    }

    static void et(int s, int, BS &b) {
        inc(poke(b,s)["YawnCount"], -1);
        int count = poke(b,s)["YawnCount"].toInt();
        if (count > 0) {
            b.sendMoveMessage(147,0,s);
        } else {
            b.inflictStatus(s, Pokemon::Asleep, s);
            removeFunction(poke(b,s),"EndTurn", "Yawn");
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

    static void ms(int s, int t, BS &b) {
        //Damp
        if (b.hasWorkingAbility(t,16)) {
            b.sendMoveMessage(114,0,t);
        } else if (b.hasWorkingAbility(s,16)) {
            b.sendMoveMessage(114,0,s);
        } else {
            return;
        }

        turn(b,s)["FaintActivationPrevented"] = true;
        turn(b,s)["Power"] = 0;
        turn(b,s)["PossibleTargets"] = Move::None;
    }
};

struct MMCamouflage : public MM {
    MMCamouflage() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas (int s, int, BS &b) {
        poke(b,s)["Type1"] = Pokemon::Normal;
        poke(b,s)["Type2"] = Pokemon::Curse;
        b.sendMoveMessage(17,0,s,0);
    }
};

struct MMNaturePower : public MM
{
    MMNaturePower() {
        functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
        int t = b.rev(s);

        removeFunction(turn(b,s), "UponAttackSuccessful", "NaturePower");

        int move = 433;
        MoveEffect::setup(move,s,t,b);
        b.useAttack(s,move,true,true);
    }
};

struct MMRolePlay : public MM {
    MMRolePlay() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Wonder Guard & multi-type */
        if (b.ability(t) == 123 || b.ability(t) == 115) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        b.acquireAbility(s, b.ability(t));
        b.sendMoveMessage(108,0,s,Pokemon::Psychic,t,b.ability(t));
    }
};

struct MMSkillSwap : public MM {
    MMSkillSwap() {
        functions["DetermineAttackFailure"] = &daf;
        functions["UponAttackSuccessful"] = &uas;
    }

    static void daf(int s, int t, BS &b) {
        /* Wonder Guard & multi-type */
        if (b.ability(t) == 123 || b.ability(t) == 115 || b.ability(s) == 123 || b.ability(s) == 115) {
            turn(b,s)["Failed"] = true;
        }
    }

    static void uas(int s, int t, BS &b) {
        int tab = b.ability(t);
        int sab = b.ability(s);

        b.acquireAbility(s, tab);
        b.acquireAbility(t, sab);
        b.sendMoveMessage(112,0,s,Pokemon::Psychic,t);
    }
};

struct MMSecretPower : public MM {
    MMSecretPower() {
        functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {

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
    *MovesPossible -- poke: at the beginning of the turn, tells if each move is possible or not
    *AfterKoedByStraightAttack -- poke: when koed by an attack
    *BlockTurnEffects -- poke: Called before calling effects for a turn event, to see if it's blocked. Used by Substitute
    *AttackSomehowFailed -- turn, only offensive moves: When an attack fails, or misses, there may be something to do (jump kick, rollout,..)
    *StatusChange -- poke
*/

#define REGISTER_MOVE(num, name) mechanics[num] = MM##name(); names[num] = #name; nums[#name] = num;

void MoveEffect::init()
{
    REGISTER_MOVE(1, Leech); /* absorb, drain punch, part dream eater, giga drain, leech life, mega drain */
    REGISTER_MOVE(2, AquaRing);
    REGISTER_MOVE(3, AromaTherapy);
    REGISTER_MOVE(4, Assist);
    REGISTER_MOVE(5, Assurance);
    REGISTER_MOVE(6, BatonPass);
    /* Beat up */
    REGISTER_MOVE(8, BellyDrum);
    REGISTER_MOVE(9, Bide);
    REGISTER_MOVE(10, Bind);
    REGISTER_MOVE(11, BlastBurn); /* BlastBurn, Hyper beam, rock wrecker, giga impact, frenzy plant, hydro cannon, roar of time */
    REGISTER_MOVE(12, Block);
    REGISTER_MOVE(13, Bounce);
    REGISTER_MOVE(14, BrickBreak);
    REGISTER_MOVE(15, Brine);
    /* Bug Bite */
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
    /* Follow Me -- does nothing in singles */
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
    REGISTER_MOVE(60, HealHalf);
    REGISTER_MOVE(61, HealingWish);
    REGISTER_MOVE(62, PowerTrick);
    /* Helping Hand does nothing in singles */
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
    /* Natural Gift */
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
    /* Recycle */
    REGISTER_MOVE(106, Rest);
    REGISTER_MOVE(107, Roar);
    REGISTER_MOVE(108, RolePlay);
    REGISTER_MOVE(109, SafeGuard);
    /* Secret Power */
    REGISTER_MOVE(111, Sketch);
    REGISTER_MOVE(112, SkillSwap);
    REGISTER_MOVE(113, WeatherBall);
    REGISTER_MOVE(114, Explosion);
    REGISTER_MOVE(115, SleepingUser);
    REGISTER_MOVE(116, SleepTalk);
    REGISTER_MOVE(117, SmellingSalt);
    REGISTER_MOVE(118, Snatch);
    REGISTER_MOVE(119, SolarBeam);
    REGISTER_MOVE(120, ThunderWave);
    REGISTER_MOVE(121, Spikes);
    /* Spit up */
    REGISTER_MOVE(123, Spite);
    REGISTER_MOVE(124, StealthRock);
    /* Stockpile */
    REGISTER_MOVE(126, Stomp);
    REGISTER_MOVE(127, Struggle);
    REGISTER_MOVE(128, Substitute);
    REGISTER_MOVE(129, SuckerPunch);
    REGISTER_MOVE(130, SuperFang);
    /* Swallow */
    REGISTER_MOVE(132, Switcheroo);
    REGISTER_MOVE(133, TailWind)
    REGISTER_MOVE(134, Taunt);
    REGISTER_MOVE(135, Torment);
    REGISTER_MOVE(136, ToxicSpikes);
    //transform
    REGISTER_MOVE(138, TrickRoom);
    REGISTER_MOVE(139, TripleKick);
    REGISTER_MOVE(140, UTurn);
    //Uproar
    REGISTER_MOVE(142, Wish);
    REGISTER_MOVE(143, WorrySeed);
    REGISTER_MOVE(144, Yawn);
    //Free move
    REGISTER_MOVE(146, Avalanche); /* avalanche, revenge */
    //Chatter
    REGISTER_MOVE(148, TrumpCard);
    REGISTER_MOVE(149, Haze);
    REGISTER_MOVE(150, Roost);
    REGISTER_MOVE(151, Ingrain);
    REGISTER_MOVE(152, Thunder);
    REGISTER_MOVE(153, UnThawing);
    REGISTER_MOVE(154, DefenseCurl);
}


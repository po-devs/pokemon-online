#include "moves.h"
#include "../PokemonInfo/pokemoninfo.h"

QHash<int, MoveMechanics> MoveEffect::mechanics;
QHash<int, QString> MoveEffect::names;
QHash<QString, int> MoveEffect::nums;

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
		b.healLife(s, recovered);
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
	c.remove("ChoiceMemory"); /* choice band etc. would force the same move*
		if on both the passed & the passer */
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
	    turn(b,s)["ConversionType"] = poss[rand()%poss.size()];
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
		turn(b,s)["ConversionType"] = poss[rand()%poss.size()];
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
	    turn(b,s)["Conversion2Type"] = poss[rand()%poss.size()];
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
	    b.inflictDamage(s, b.poke(s).totalLifePoints()/2, s);
	    addFunction(poke(b,t), "EndTurn", "Cursed", &et);
	    b.sendMoveMessage(25, 0, s, Pokemon::Curse, t);
	}
    }

    static void et(int s, int, BS &b) {
	b.inflictDamage(s, b.poke(s).totalLifePoints()/4, s);
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
	    if (rand()%2 == 0) {
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
	functions["MoveSettings"] = &ms;
    }

    static void ms(int s, int, BS &b) {
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

struct MM0HKO : public MM
{
    MM0HKO() {
	functions["DetermineAttackFailure"] = &daf;
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int t, BS &b) {
	b.inflictDamage(t, b.poke(t).totalLifePoints(), s, true);
    }

    static void daf(int s, int t, BS &b) {
	if (b.poke(s).level() < b.poke(t).level()) {
	    turn(b,s)["Failed"] = true;
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
	b.inflictDamage(t, b.poke(t).lifePoints()/2, s, true);
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
	b.sendMoveMessage(95,1,s,0,0,count);
	if (count > 0) {
	    poke(b,s)["PerishSongCount"] = count - 1;
	} else {
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
	if (b.hasType(t, Pokemon::Grass) || (poke(b,t).value("Seeded").toBool() == true)) {
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
	if (b.poke(s).status() == Pokemon::Asleep || b.poke(s).isFull()) {
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
	if (poke(b,s).contains("WishCount") && poke(b,s)["WishCount"].toInt() >= 0) {
	    turn(b,s)["Failed"] = true;
	}
    }

    static void uas(int s, int, BS &b) {
	poke(b,s)["WishCount"] = 1;
	poke(b,s)["Wisher"] = b.poke(s).nick();
	b.sendMoveMessage(142, 0, s);
	addFunction(poke(b,s), "EndTurn", "Wish", &et);
    }

    static void et(int s, int, BS &b) {
	if (b.koed(s)) {
	    return;
	}
	int count = poke(b,s)["WishCount"].toInt();
	if (count < 0) {
	    return;
	}
	if (count == 0) {
	    b.sendMoveMessage(142, 1, 0, 0, 0, 0, poke(b,s)["Wisher"].toString());
	    b.healLife(s, b.poke(s).totalLifePoints()/2);
	}
	poke(b,s)["WishCount"] = count - 1;
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
	if (poke(b,t).value("Rooted").toBool()) {
	    turn(b,s)["Failed"] = true;
	} else {
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
	b.sendPoke(t, switches[rand()%switches.size()]);
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
	int spikeslevel = team(b,s)["ToxicSpikes"].toInt();
	switch (spikeslevel) {
	    case 0: return;
	    case 1: b.inflictStatus(s, Pokemon::Poisoned); break;
	    default: b.inflictStatus(s, Pokemon::DeeplyPoisoned); break;
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
	team(b,s).remove("Seeded");
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
	addFunction(poke(b,s), "BlockTurnEffects", "Substitute", &bte);
    }

    static void bte(int s, int t, BS &b) {
	if (s == t || t==-1) {
	    return;
	}
	if (!b.hasSubstitute(t)) {
	    return;
	}
	QString effect = turn(b,s)["EffectActivated"].toString();

	if (effect == "Bind" || effect == "Block" || effect == "Covet" || effect == "Curse" || effect == "Embargo" || effect == "GastroAcid" || effect == "Grudge"
	    || effect == "HealBlock" || effect == "KnockOff" || effect == "LeechSeed"
	    || effect == "LockOn" || effect == "Mimic" || effect == "PsychoShift" || effect == "Sketch" || effect == "Switcheroo"
	    || effect == "WorrySeed" || effect == "Yawn")
	{
	    turn(b,s)["EffectBlocked"] = true;
	    if (turn(b,s)["Power"].toInt() == 0)
		b.sendMoveMessage(128, 2, s,0,t,turn(b,s)["Attack"].toInt());
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
	    if (!b.poke(s,i).ko()) {
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
	if (b.poke(s).gender() == Pokemon::Neutral || b.poke(t).gender() == Pokemon::Neutral || b.poke(s).gender() == b.poke(t).gender()) {
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
	    b.disposeItem(t);
	}
    }

    static void pda(int s, int, BS &b) {
	if (poke(b,s).contains("AttractedTo")) {
	    int seducer = poke(b,s)["AttractedTo"].toInt();
	    if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
		b.sendMoveMessage(58,0,s,0,seducer);
		if (rand() % 2 == 0) {
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
	if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, 101) && b.poke(t).num() != 493) /* Sticky Hold, Arceus */
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
	if (!b.koed(t) && b.poke(t).item() != 0 && !b.hasWorkingAbility(t, 101) && b.poke(t).num() != 493 && b.poke(s).item() == 0) /* Sticky Hold, Arceus */
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
	if (b.koed(t) || (b.poke(t).item() == 0 && b.poke(s).item() == 0) || b.hasWorkingAbility(t, 101) || b.poke(t).num() == 493 || b.poke(s).item() == 0) /* Sticky Hold, Arceus */
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
	    turn(b,s)["AssistMove"] = *(possible_moves.begin() + (rand() %possible_moves.size()));
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
		addFunction(turn(b,s), "MoveSettings", "Bide", &ms);
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
	poke(b,t)["TrappedCount"] = b.poke(s).item() == 190 ? 5 : (rand()%4) + 2; /* Grip claw = 5 turns */
	poke(b,t)["TrappedMove"] = move(b,s);
	poke(b,s)["Trapped"] = t;
	addFunction(poke(b,t), "EndTurn", "Bind", &et);
    }

    static void et (int s, int, BS &b) {
	int count = poke(b,s)["TrappedCount"].toInt() - 1;
	int move = poke(b,s)["TrappedMove"].toInt();
	if (count <= 0) {
	    poke(b,s).remove("TrappedBy");
	    removeFunction(poke(b,s),"EndTurn", "Bind");
	    b.sendMoveMessage(10,1,s,MoveInfo::Type(move),s,move);
	} else {
	    poke(b,s)["TrappedCount"] = count;
	    b.sendMoveMessage(10,0,s,MoveInfo::Type(move),s,move);
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

    static void uas (int, int t, BS &b) {
	poke(b,t)["TauntsUntil"] = b.turn() + 2 + (rand()%3);
	addFunction(poke(b,t), "MovesPossible", "Taunt", &msp);
	addFunction(poke(b,t), "MovePossible", "Taunt", &mp);
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
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMove").toBool())) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas (int s, int t, BS &b) {
	poke(b,t)["DisablesUntil"] = b.turn() + 3 + (rand()%4);
	poke(b,t)["DisabledMove"] = poke(b,t)["MoveSlot"];
	addFunction(poke(b,t), "MovesPossible", "Disable", &msp);
	addFunction(poke(b,t), "MovePossible", "Disable", &mp);
	addFunction(poke(b,t), "EndTurn", "Disable", &et);
	b.sendMoveMessage(28,0,s,0,t,b.move(t,poke(b,t)["MoveSlot"].toInt()));
    }

    static void et (int s, int, BS &b)
    {
	int tt = poke(b,s)["DisablesUntil"].toInt();
	if (tt <= b.turn()) {
	    removeFunction(poke(b,s), "MovesPossible", "Disable");
	    removeFunction(poke(b,s), "MovePossible", "Disable");
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
		b.inflictDamage(s,team(b,s)["DoomDesireDamage"].toInt(), s, true);
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
	if (tu + 1 < b.turn() || (tu + 1 == b.turn() && turn(b,t).value("HasMove").toBool())) {
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
	    if (b.move(s, i) == move) {
		sl = i;
	    }
	}
	if (sl == -1 || b.poke(s).move(sl).PP() == 0 ) {
	    turn(b,s)["Failed"] = true;
	    return;
	}
    }

    static void uas (int s, int t, BS &b) {
	poke(b,t)["EncoresUntil"] = b.turn() + 3 + (rand()%5);
	poke(b,t)["EncoresMove"] = poke(b,t)["LastMoveSuccessfullyUsed"];
	addFunction(poke(b,t), "MovesPossible", "Encore", &msp);
	addFunction(poke(b,t), "MovePossible", "Encore", &mp);
	addFunction(poke(b,t), "EndTurn", "Encore", &et);
	b.sendMoveMessage(33,2,s,0,t,poke(b,t)["LastMoveSuccessfullyUsed"].toInt());
    }

    static void et (int s, int, BS &b)
    {
	int tt = poke(b,s)["EncoresUntil"].toInt();
	if (tt <= b.turn()) {
	    removeFunction(poke(b,s), "MovesPossible", "Encore");
	    removeFunction(poke(b,s), "MovePossible", "Encore");
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
	if(b.move(s,poke(b,s)["MoveSlot"].toInt()) != poke(b,s)["DisabledMove"].toInt()) {
	    turn(b,s)["ImpossibleToMove"] = true;
	    b.sendMoveMessage(33,1,s,0,s,b.move(s, poke(b,s)["MoveSlot"].toInt()));
	}
    }
};
MMEncore::FM MMEncore::forbidden_moves;

#define REGISTER_MOVE(num, name) mechanics[num] = MM##name(); names[num] = #name; nums[#name] = num;

void MoveEffect::init()
{
    REGISTER_MOVE(1, Leech); /* absorb, drain punch, part dream eater, giga drain, leech life, mega drain */
    REGISTER_MOVE(2, AquaRing);
    REGISTER_MOVE(3, AromaTherapy);
    REGISTER_MOVE(4, Assist);
    REGISTER_MOVE(5, Assurance);
    REGISTER_MOVE(6, BatonPass);
    REGISTER_MOVE(8, BellyDrum);
    REGISTER_MOVE(9, Bide);
    REGISTER_MOVE(10, Bind);
    REGISTER_MOVE(11, BlastBurn); /* BlastBurn, Hyper beam, rock wrecker, giga impact, frenzy plant, hydro cannon, roar of time */
    REGISTER_MOVE(12, Block);
    REGISTER_MOVE(13, Bounce);
    REGISTER_MOVE(15, Brine);
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
    REGISTER_MOVE(36, Eruption); /* Eruption, Water sprout */
    REGISTER_MOVE(37, FaintUser); /* Memento, part explosion, selfdestruct, lunar dance, healing wish... */
    REGISTER_MOVE(39, Facade);
    REGISTER_MOVE(40, FakeOut);
    REGISTER_MOVE(42, Feint);
    REGISTER_MOVE(43, 0HKO); /* Fissure, Guillotine, Horn Drill, Sheer cold */
    REGISTER_MOVE(44, Flail); /* Flail, Reversal */
    REGISTER_MOVE(47, FocusPunch);
    REGISTER_MOVE(49, Frustration); /* Frustration, Return */
    REGISTER_MOVE(58, Attract);
    REGISTER_MOVE(60, HealHalf);
    REGISTER_MOVE(65, HiddenPower);
    REGISTER_MOVE(70, KnockOff);
    REGISTER_MOVE(72, LeechSeed);
    REGISTER_MOVE(91, NightShade);
    REGISTER_MOVE(94, PainSplit);
    REGISTER_MOVE(95, PerishSong);
    REGISTER_MOVE(103, RapidSpin);
    REGISTER_MOVE(106, Rest);
    REGISTER_MOVE(107, Roar);
    REGISTER_MOVE(120, ThunderWave);
    REGISTER_MOVE(121, Spikes);
    REGISTER_MOVE(124, StealthRock);
    REGISTER_MOVE(128, Substitute);
    REGISTER_MOVE(130, SuperFang);
    REGISTER_MOVE(132, Switcheroo);
    REGISTER_MOVE(134, Taunt);
    REGISTER_MOVE(136, ToxicSpikes);
    REGISTER_MOVE(140, UTurn);
    REGISTER_MOVE(142, Wish);
    REGISTER_MOVE(146, Avalanche); /* avalanche, revenge */
    REGISTER_MOVE(148, TrumpCard);
    REGISTER_MOVE(149, Haze);
    REGISTER_MOVE(150, Roost);
    REGISTER_MOVE(151, Ingrain);
}


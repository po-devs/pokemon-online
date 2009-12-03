#include "moves.h"
#include "../PokemonInfo/pokemoninfo.h"

MoveMechanics* MoveEffect::mechanics[MOVE_MECHANICS_SIZE];
QString MoveEffect::names[MOVE_MECHANICS_SIZE];

MoveMechanics::MoveMechanics()
{
}

BattleSituation::context & MoveMechanics::turn(BattleSituation &b, int player)
{
	return b.turnlong[player];
}

BattleSituation::context & MoveMechanics::poke(BattleSituation &b, int player)
{
	return b.pokelong[player];
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
}

/* There's gonna be tons of structures inheriting it,
    so let's do it fast */
typedef MoveMechanics MM;
typedef BattleSituation BS;

void addFunction(BattleSituation::context &c, const QString &effect, const QString &name, MoveMechanics::function f)
{
    if (!c.contains(effect)) {
	/* Those three steps are absolutely required, cuz of fucktard lack of QVariant template constuctor/ template operator =
		and fucktard QSharedPointer implicit conversion */
	QVariant v;
	v.setValue(QSharedPointer<QSet<QString> >(new QSet<QString>()));
	c.insert(effect, v);
    }
    c[effect].value<QSharedPointer<QSet<QString> > >()->insert(name);

    QVariant v;
    v.setValue(f);
    c.insert(effect + "_" + name,v);
}

void MoveEffect::setup(int num, int source, int target, BattleSituation &b)
{
    MoveEffect e(num);

    /* first the basic info */
    merge(b.turnlong[source], e);

    qDebug() << "B power: " << b.turnlong[source]["Power"].toInt();
    qDebug() << "E power: " << e["Power"].toInt();

    /* then the hard info */
    int specialEffect = MoveInfo::SpecialEffect(num).toInt();

    qDebug() << "Effect: " << specialEffect;

    /* if the effect is invalid or not yet implemented then no need to go further */
    if (specialEffect < 0 || specialEffect >= MOVE_MECHANICS_SIZE || mechanics[specialEffect] == NULL) {
	return;
    }

    MoveMechanics &m = *mechanics[specialEffect];
    QString &n = names[specialEffect];

    QMap<QString, MoveMechanics::function>::iterator i;

    for(i = m.functions.begin(); i != m.functions.end(); ++i) {
	addFunction(b.turnlong[source], i.key(), n, i.value());
    }

    (void) target;
}

struct MMLeech : public MM
{
    MMLeech() {
	functions["UponDamageInflicted"] = &aad;
    }

    static void aad(int s, int, BS &b) {
	if (!b.koed(s)) {
	    int damage = turn(b, s)["DamageInflicted"].toInt();

	    if (damage != 0) {
		int recovered = std::max(1, damage/2);
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
    }

    static void et(int s, int, BS &b) {
	if (!b.koed(s) && !b.poke(s).isFull()) {
	    b.healLife(s, std::max(1, b.poke(s).totalLifePoints()/16));
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
{ /*POSSIBLE GLITCH: if the poke switchs in and dies then maybe the next will inherit the changes */
    MMBatonPass() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	/* first we copy the temp effects, then put them to the next poke */
	BS::context c = poke(b, s);
	b.requestSwitch(s);
	c.remove("Type1");
	c.remove("Type2");
	c.remove("Minimize");
	c.remove("DefenseCurl");
	for (int i = 1; i < 6; i++) {
	    c.remove(QString("Stat%1").arg(i));
	}
	c.remove("Level");
	merge(poke(b,s), c);
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
	}
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
	    if (MoveInfo::Type(b.poke(s).move(i)) != Move::Curse) {
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
		if (MoveInfo::Type(b.poke(s).move(i)) != Move::Curse) {
		    poss.push_back(b.poke(s).move(i));
		}
	    }
	    turn(b,s)["ConversionType"] = poss[rand()%poss.size()];
	} else {
	    QList<int> poss;
	    for (int i = 0; i < 4; i++) {
		if (MoveInfo::Type(b.poke(s).move(i)) != Move::Curse && MoveInfo::Type(b.poke(s).move(i)) != poke(b,s)["Type1"].toInt()) {
		    poss.push_back(b.poke(s).move(i));
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
	poke(b,s)["Type1"] = turn(b,s)["ConversionType"];
	poke(b,s)["Type2"] = Pokemon::Curse;
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
	    if (!(poke(b,s)["Type1"].toInt() == i && poke(b,s)["Type2"].toInt() == Pokemon::Curse) && TypeInfo::Eff(attackType, i) < TypeInfo::Effective) {
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
	poke(b,s)["Type1"] = turn(b,s)["Conversion2Type"];
	poke(b,s)["Type2"] = Pokemon::Curse;
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
	if (!b.battlelong.contains("LastMoveSuccessfullyUsed") || b.battlelong["LastMoveSuccessfullyUsed"].toInt() == 68) {
	    turn(b,s)["Failed"] = true;
	} else {
	    turn(b,s)["CopycatMove"] = b.battlelong["LastMoveSuccessfullyUsed"];
	}
    }

    static void uas(int s, int t, BS &b) {
	int attack = turn(b,s)["CopycatMove"].toInt();
	MoveEffect::setup(attack, s, t, b);
	b.useAttack(s, turn(b,s)["CopycatMove"].toInt(), true);
    }
};

struct MMCrushGrip : public MM
{
    MMCrushGrip() {
	functions["BeforeCalculatingDamage"] = &bcd;
    }

    static void bcd(int s, int t, BS &b) {
	turn(b, s)["Power"] = turn(b, s)["Power"].toInt() * 120 * b.poke(t).lifePoints()/b.poke(t).totalLifePoints();
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
	}
    }

    static void et(int s, int, BS &b) {
	b.inflictDamage(s, b.poke(s).totalLifePoints()/4, s);
    }
};

#define REGISTER_MOVE(num, name) mechanics[num] = new MM##name; names[num] = #name;

void MoveEffect::init()
{
    for (int i = 0; i < 200; i++) {
	mechanics[i] = NULL;
    }

    REGISTER_MOVE(1, Leech); /* absorb, drain punch, giga drain, leech life, mega drain */
    REGISTER_MOVE(2, AquaRing);
    REGISTER_MOVE(5, Assurance);
    REGISTER_MOVE(6, BatonPass);
    REGISTER_MOVE(11, BlastBurn); /* BlastBurn, Hyper beam, rock wrecker, giga impact, frenzy plant, hydro cannon, roar of time */
    REGISTER_MOVE(15, Brine);
    REGISTER_MOVE(18, Charge);
    REGISTER_MOVE(19, Conversion);
    REGISTER_MOVE(20, Conversion2);
    REGISTER_MOVE(21, Copycat);
    REGISTER_MOVE(24, CrushGrip); /* Crush grip, Wring out */
    REGISTER_MOVE(25, Curse);
    REGISTER_MOVE(146, Avalanche); /* avalanche, revenge */
}


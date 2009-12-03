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
	    turn(b,s)["Power"] = 100;
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
	    turn(b,s)["Power"] = 120;
	}
    }
};

struct MMBatonPass : public MM
{
    MMBatonPass() {
	functions["UponAttackSuccessful"] = &uas;
    }

    static void uas(int s, int, BS &b) {
	/* first we copy the temp effects, then put them to the next poke */
	BS::context c = poke(b, s);
	b.requestSwitch(s);
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
	    turn(b, s)["Power"] = 130;
	}
    }
};

#define REGISTER_MOVE(num, name) mechanics[num] = new MM##name; names[num] = #name;

void MoveEffect::init()
{
    for (int i = 0; i < 200; i++) {
	mechanics[i] = NULL;
    }

    REGISTER_MOVE(1, Leech);
    REGISTER_MOVE(2, AquaRing);
    REGISTER_MOVE(5, Assurance);
    REGISTER_MOVE(6, BatonPass);
    REGISTER_MOVE(11, BlastBurn);
    REGISTER_MOVE(15, Brine);
    REGISTER_MOVE(146, Avalanche);
}


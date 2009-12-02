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
	if (!b.turnlong[source].contains(i.key())) {
	    /* Those three steps are absolutely required, cuz of fucktard lack of QVariant template constuctor/ template operator =
		    and fucktard QSharedPointer implicit conversion */
	    QVariant v;
	    v.setValue(QSharedPointer<QSet<QString> >(new QSet<QString>()));
	    b.turnlong[source].insert(i.key(), v);
	}

	b.turnlong[source][i.key()].value<QSharedPointer<QSet<QString> > >()->insert(n);

	QVariant v;
	v.setValue(i.value());
	b.turnlong[source].insert(i.key() + "_" + n,v);
    }

    (void) target;
}

struct MMLeech : public MM
{
    MMLeech() {
	functions["UponDamageInflicted"] = &aad;
    }

    static void aad(int s, int, BS &b) {
	int damage = turn(b, s)["DamageInflicted"].toInt();

	if (damage != 0) {
	    int recovered = std::max(1, damage/2);
	    b.healLife(s, recovered);
	}
    }
};

void MoveEffect::init()
{
    for (int i = 0; i < 200; i++) {
	mechanics[i] = NULL;
    }

    mechanics[1] = new MMLeech();
}


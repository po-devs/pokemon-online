#include "mechanics.h"


BattleSituation::context & Mechanics::turn(BattleSituation &b, int player)
{
    return b.turnlong[player];
}

BattleSituation::context & Mechanics::poke(BattleSituation &b, int player)
{
    return b.pokelong[player];
}

BattleSituation::context & Mechanics::team(BattleSituation &b, int player)
{
    return b.teamzone[player];
}

int Mechanics::type(BattleSituation &b, int source)
{
    return turn(b,source)["Type"].toInt();
}

int Mechanics::move(BattleSituation &b, int source)
{
    return turn(b, source)["LastMoveUsed"].toInt();
}

void Mechanics::addFunction(BattleSituation::context &c, const QString &effect, const QString &name, Mechanics::function f)
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

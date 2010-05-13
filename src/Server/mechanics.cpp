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

BattleSituation::context & Mechanics::slot(BattleSituation &b, int player)
{
    return b.slotzone[player];
}

int Mechanics::type(BattleSituation &b, int source)
{
    return turn(b,source)["Type"].toInt();
}

int Mechanics::move(BattleSituation &b, int source)
{
    return turn(b, source)["Attack"].toInt();
}

void Mechanics::addFunction(BattleSituation::context &c, const QString &effect, const QString &name, Mechanics::function f)
{
    if (!c.contains("Effect_" + effect)) {
        /* Those three steps are absolutely required, cuz of lack of QVariant template constuctor/ template operator =
                and QSharedPointer implicit conversion */
	QVariant v;
	v.setValue(QSharedPointer<QSet<QString> >(new QSet<QString>()));
        c.insert("Effect_" + effect, v);
    }
    c["Effect_"+effect].value<QSharedPointer<QSet<QString> > >()->insert(name);

    QVariant v;
    v.setValue(f);
    c.insert("Effect_" + effect + "_" + name,v);
}

void Mechanics::removeFunction(BattleSituation::context &c, const QString &effect, const QString &name)
{
    if (!c.contains("Effect_" + effect)) {
	return;
    }
    c["Effect_" + effect].value<QSharedPointer<QSet<QString> > >()->remove(name);
    c.remove("Effect_" + effect + "_" + name);
}

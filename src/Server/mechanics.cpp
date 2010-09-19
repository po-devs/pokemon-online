#include "mechanics.h"


BattleSituation::context & Mechanics::turn(BattleSituation &b, int player)
{
    return b.turnlong[player];
}

BattleSituation::context & Mechanics::poke(BattleSituation &b, int player)
{
    return b.pokelong[player];
}

BattleSituation::BasicPokeInfo & Mechanics::fpoke(BattleSituation &b, int player)
{
    return b.fieldpokes[player];
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
    return tmove(b, source).type;
}

int Mechanics::move(BattleSituation &b, int source)
{
    return tmove(b, source).attack;
}

BattleSituation::BasicMoveInfo & Mechanics::tmove(BattleSituation &b, int source)
{
    return b.fieldmoves[source];
}

void Mechanics::addFunction(BattleSituation::context &c, const QString &effect, const QString &name, Mechanics::function f)
{
    if (!c.contains("Effect_" + effect)) {
        QVariant v = QVariant::fromValue(QSharedPointer<QSet<QString> >(new QSet<QString>()));
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

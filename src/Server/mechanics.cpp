#include "mechanics.h"


BattleSituation::context & Mechanics::turn(BattleSituation &b, int player)
{
    return b.turnMemory(player);
}

BattleSituation::context & Mechanics::poke(BattleSituation &b, int player)
{
    return b.pokeMemory(player);
}

BattleSituation::BasicPokeInfo & Mechanics::fpoke(BattleSituation &b, int player)
{
    return b.fpoke(player);
}

BattleSituation::context & Mechanics::team(BattleSituation &b, int player)
{
    return b.teamMemory(player);
}

BattleSituation::context & Mechanics::slot(BattleSituation &b, int player)
{
    return b.slotMemory(player);
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
    return b.tmove(source);
}

BattleSituation::priorityBracket Mechanics::makeBracket(int b, int p)
{
    return BattleSituation::priorityBracket(b, p);
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

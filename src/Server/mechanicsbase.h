#ifndef MECHANICSBASE_H
#define MECHANICSBASE_H

#include "battlebase.h"

struct PureMechanicsBase {
    static BattleBase::context & turn(BattleBase &b, int player);
    static BattleBase::TurnMemory & fturn(BattleBase &b, int player);
    static BattleBase::context & poke(BattleBase &b, int player);
    static BattleBase::BasicPokeInfo & fpoke(BattleBase &b, int player);
    static int move(BattleBase &b, int source);
    static int type(BattleBase &b, int source);
    static BattleBase::BasicMoveInfo & tmove(BattleBase &b, int source);

    static void initMove(int num, Pokemon::gen gen, BattleBase::BasicMoveInfo &bmi);
};

template <class function>
struct MechanicsBase : public PureMechanicsBase
{
    QHash<QString, function> functions;

    static void addFunction(BattleBase::context &c, const QString &effect, const QString &name, function f);
    static void removeFunction(BattleBase::context &c, const QString &effect, const QString &name);
};

template <class function>
void MechanicsBase<function>::addFunction(BattleBase::context &c, const QString &effect, const QString &name, function f)
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

template <class function>
void MechanicsBase<function>::removeFunction(BattleBase::context &c, const QString &effect, const QString &name)
{
    if (!c.contains("Effect_" + effect)) {
    return;
    }
    c["Effect_" + effect].value<QSharedPointer<QSet<QString> > >()->remove(name);
    c.remove("Effect_" + effect + "_" + name);
}


/* For use with QVariants */
Q_DECLARE_METATYPE(QSharedPointer<QSet<QString> >)


#endif // MECHANICSBASE_H

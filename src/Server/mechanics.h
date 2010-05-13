#ifndef MECHANICS_H
#define MECHANICS_H

#include "../Utilities/thread_safe_containers.h"
#include "battle.h"

struct Mechanics
{
    /* Returns b.turnlong[player], used for convenience cuz shorter */
    static BattleSituation::context & turn(BattleSituation &b, int player);
    static BattleSituation::context & poke(BattleSituation &b, int player);
    static BattleSituation::context & team(BattleSituation &b, int player);
    static BattleSituation::context & slot(BattleSituation &b, int player);
    static int move(BattleSituation &b, int source);
    static int type(BattleSituation &b, int source);

    typedef void (*function) (int source, int target, BattleSituation &b);

    QTSHash<QString, function> functions;

    static void addFunction(BattleSituation::context &c, const QString &effect, const QString &name, Mechanics::function f);
    static void removeFunction(BattleSituation::context &c, const QString &effect, const QString &name);
};

/* For use with QVariants */
Q_DECLARE_METATYPE(QSharedPointer<QSet<QString> >)
Q_DECLARE_METATYPE(Mechanics::function)

#endif // MECHANICS_H

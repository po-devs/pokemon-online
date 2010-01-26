#ifndef ABILITIES_H
#define ABILITIES_H

#include "mechanics.h"

struct AbilityMechanics : public Mechanics
{
};

/* Used to tell us everything about an item */
struct AbilityEffect : public QVariantHash
{
    AbilityEffect(int num);

    static void setup(int num, int source, BattleSituation &b);
    static void activate(const QString &effect, int num, int source, int target, BattleSituation &b);

    static QTSHash<int, AbilityMechanics> mechanics;
    static QTSHash<int, QString> names;
    static QTSHash<QString, int> nums;

    static void init();
};

#endif // ABILITIES_H

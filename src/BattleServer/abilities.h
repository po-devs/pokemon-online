#ifndef ABILITIES_H
#define ABILITIES_H

#include "mechanics.h"
#include "items.h" //For Klutz

struct AbilityMechanics : public Mechanics
{
};

/* Used to tell us everything about an item */
struct AbilityEffect : public QVariantHash
{
    AbilityEffect(int num);

    static void setup(int num, int source, BattleSituation &b, bool firstAct = false);
    static void activate(const QString &effect, int num, int source, int target, BattleSituation &b);

    static QHash<int, AbilityMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
};

#endif // ABILITIES_H

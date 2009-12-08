#ifndef ITEMS_H
#define ITEMS_H

#include "mechanics.h"

struct ItemMechanics : public Mechanics
{
};

/* Used to tell us everything about an item */
struct ItemEffect : public QVariantHash
{
    ItemEffect(int num);

    static void setup(int num, int source, BattleSituation &b);
    static void activate(const QString &effect, int num, int source, int target, BattleSituation &b);

    static QHash<int, ItemMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
};

#endif // ITEMS_H

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

    /* Beware, that data is used by BugBite so don't modify it directly */
    static QHash<int, ItemMechanics> mechanics;
    static QHash<int, QString> names;
    static QHash<QString, int> nums;

    static void init();
    static void initBerries(); /* in berries.cpp */
};

#endif // ITEMS_H

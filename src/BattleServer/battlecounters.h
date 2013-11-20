#ifndef BATTLECOUNTERS_H
#define BATTLECOUNTERS_H

#include <QHash>

struct BattleCounters
{
    void clear();
    void addCounter(int index, int count);
    void decreaseCounters();
    /* Returns -1 if the counter is no more */
    int count(int index);
    void removeCounter(int index);
    bool hasCounter(int index);

    QHash<int, int> counters;
};

#endif // BATTLECOUNTERS_H

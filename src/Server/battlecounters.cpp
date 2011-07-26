#include "battlecounters.h"

void BattleCounters::clear() {
    counters.clear();
}

void BattleCounters::addCounter(int index, int count) {
    counters.insert(index, count);
}

void BattleCounters::removeCounter(int index) {
    counters.remove(index);
}

void BattleCounters::decreaseCounters() {
    QHash<int, int>::iterator it = counters.begin();

    while (it != counters.end()) {
        it.value() = it.value() - 1;
        ++it;
    }
}

int BattleCounters::count(int index) {
    return counters.value(index, -1);
}

bool BattleCounters::hasCounter(int index) {
    return counters.contains(index);
}

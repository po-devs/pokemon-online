#include <cstdio>
#include "battledata.h"

void testing() {
    BattleData *data = new BattleData();

    try {
        data->entryPoint(battle::Ko, 0);
    } catch (int) {
        puts("processing fine");
    }
}

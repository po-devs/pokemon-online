#include <cstdio>
#include "battledata.h"

void testing() {
    BattleData *data = new BattleData();

    data->entryPoint(battle::SendBack);
    data->entryPoint(battle::Ko, 0);
}

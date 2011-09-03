#include <cstdio>
#include "battledata.h"

using namespace battle;

class TestBattleData : public BattleCommandManager<TestBattleData>
{
public:
    void onKo(int spot) {
        printf("My function onKo called with spot %d\n", spot);
    }

    template <BattleEnum val, typename ...Params>
    void mInvoke(Params&&...) {
        printf("Default handler for command %d called\n", val);
    }

    void unknownEntryPoint(BattleEnum val, va_list) {
        printf("Extracter doesn't know command %d, no handler called\n", val);
    }

private:

};

void testing() {
    TestBattleData *data = new TestBattleData();

    data->entryPoint(battle::SendBack);
    data->entryPoint(battle::Ko, 0);
    data->entryPoint(battle::Turn);
}

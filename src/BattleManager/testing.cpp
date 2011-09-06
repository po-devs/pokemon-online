#include <cstdio>
#include "battledata.h"

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

    std::shared_ptr<ShallowBattlePoke> poke(new ShallowBattlePoke());

    data->entryPoint(BattleEnum::SendBack);
    data->entryPoint(BattleEnum::Ko, 0);
    data->entryPoint(BattleEnum::SendOut,0,0,&poke, false);
    data->entryPoint(BattleEnum::Turn);
}

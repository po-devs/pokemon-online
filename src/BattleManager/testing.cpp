#include <cstdio>
#include "battledata.h"
#include "shareddataptr.h"

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

class TestDetach {
public:
    TestDetach() {
        printf("Constructor of Test Detach called\n");
        i = 0;
    }

    TestDetach(const TestDetach &other) {
        i = other.i;

        printf("Copy constructor of Test Detach called\n");
    }

    int data() const {
        return i;
    }

    int value() const {
        return i;
    }

    int &value() {
        return i;
    }

    int i;
};

typedef data_ptr<TestDetach> detach;

void testing() {
    TestBattleData *data = new TestBattleData();

    data->entryPoint(battle::SendBack);
    data->entryPoint(battle::Ko, 0);
    data->entryPoint(battle::Turn);

    detach a, b, c, d;
    a->i = 2;
    d=b=c=a;
    int x = a->data();
    x = x+1;
    printf("About to change c->i to 5 \n");
    c->value() = 5;
    printf("Value changed.\n");
    printf("a b c d: %d %d %d %d \n", a->value(), b->value(), c->value(), d->value());
}

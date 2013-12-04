#include <PokemonInfo/battlestructs.h>
#include <BattleManager/battleinput.h>

#include "testmeganature.h"

static int turn = 0;
static bool rejected = false;
static bool accepted = false;

class TestBattleMegaNature : public BattleCommandManager<TestBattleMegaNature>
{
public:
    TestBattleMegaNature(BattleAnalyzer *analyzer, int player)
        : analyzer(analyzer), player(player) {

    }

    void onBeginTurn(int turn) {
        ::turn = turn;
    }

    void onChoiceSelection(int p) {
        if (p != player) {
            return;
        }
        if (turn > 0) {
            rejected = true;
            return;
        }
        AttackChoice attack;
        attack.attackTarget = p;
        attack.attackSlot = 0;
        attack.mega = true;

        BattleChoice choice(p, attack);
        analyzer->notifyChoice(1, p, choice);
    }

    void onDynamicStats(int spot, const BattleStats& stats) {
        if (spot != player) {
            return;
        }

        if (turn == 0) {
            return;
        }

        auto speed = stats.stats[Speed-1];

        /* Check speed of timid mega lucario */
        assert (speed == 353);
        accepted = true;
    }

private:
    BattleAnalyzer *analyzer;
    int player;
};

void TestMegaNature::onBattleServerConnected()
{
    BattleServerTest::onBattleServerConnected();

    Team t1;
    t1.importFromTxt(getFileContent("mega.txt").trimmed());

    TeamBattle tb1, tb2;

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Fuzzy";
    tb2.name = "Weavile";

    ChallengeInfo c;

    analyzer->startBattle(1, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);
}

void TestMegaNature::onBattleMessage(int, int p, const QByteArray &mess)
{
    BattleInput input;
    TestBattleMegaNature battle(analyzer, p-1);

    input.addOutput(&battle);
    input.receiveData(mess);

    if (rejected) {
        reject();
    } else if (accepted) {
        accept();
    }
}

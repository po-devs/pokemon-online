#include <BattleManager/battlecommandmanager.h>
#include <BattleManager/battleinput.h>

#include "testimposter.h"

static int done = 0;

class TestBattleImposter : public BattleCommandManager<TestBattleImposter>
{
public:
    TestBattleImposter() {

    }

    void onChoiceSelection(int) {
        done = true;
    }
};

void TestImposter::onBattleServerConnected()
{
    connect(analyzer, SIGNAL(battleMessage(int,int,QByteArray)), SLOT(onBattleMessage(int,int,QByteArray)));

    Team t1;
    t1.importFromTxt(getFileContent("ditto.txt").trimmed());

    TeamBattle tb1, tb2;

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Moogle";
    tb2.name = "Mystra";

    ChallengeInfo c;
    c.rated = true;

    analyzer->startBattle(1, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);
}

/* Needed to forward messages to a battle */
void TestImposter::onBattleMessage(int , int , const QByteArray &message)
{
    BattleInput input;
    TestBattleImposter battle;

    input.addOutput(&battle);
    input.receiveData(message);

    /* Done is set to true by the battle on the onChoiceSelection message.
     *
     * That message being received means that both pokemons were sent out
     * fine without the battle server crashing, so the test is good. */
    if (done) {
        accept();
    }
}

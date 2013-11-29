#include <BattleManager/battlecommandmanager.h>
#include <BattleManager/battleinput.h>

#include "testunrated.h"

static int done = 0;

class TestBattleUnrated : public BattleCommandManager<TestBattleUnrated>
{
public:
    TestBattleUnrated(int id) : id(id), done(false) {

    }

    void onRatedNotification(bool rated) {
        assert(id == 1 || id == 2);

        if (id == 1) {
            assert(rated);
        } else {
            assert(!rated);
        }

        if (!done) {
            ::done++;
        }
        done = true;
    }

    int id;
    bool done;
};

void TestUnrated::onBattleServerConnected()
{
    connect(analyzer, SIGNAL(battleMessage(int,int,QByteArray)), SLOT(onBattleMessage(int,int,QByteArray)));

    Team t1;
    t1.importFromTxt(getFileContent("team1.txt").trimmed());

    TeamBattle tb1, tb2;

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Moogle";
    tb2.name = "Mystra";

    ChallengeInfo c;
    c.rated = true;

    analyzer->startBattle(1, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);

    c.rated = false;
    analyzer->startBattle(2, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);
}

void TestUnrated::onBattleMessage(int battleid, int, const QByteArray &array)
{
    /* We create those every time because the cost is basically nothing, but they
     * could be persistent */
    BattleInput input;
    TestBattleUnrated battle(battleid);

    input.addOutput(&battle);
    input.receiveData(array);

    if (done == 2) {
        accept();
    }
}

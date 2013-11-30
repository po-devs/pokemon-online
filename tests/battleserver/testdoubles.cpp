#include <PokemonInfo/battlestructs.h>
#include "testdoubles.h"

static int done = 0;

void TestDoubles::onBattleServerConnected()
{
    BattleServerTest::onBattleServerConnected();

    Team t1;
    t1.importFromTxt(getFileContent("team1.txt").trimmed());

    TeamBattle tb1, tb2;

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Moogle";
    tb2.name = "Mystra";

    ChallengeInfo c;

    analyzer->startBattle(1, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);

    c.mode = ChallengeInfo::Doubles;
    analyzer->startBattle(2, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);

    c.mode = ChallengeInfo::Triples;
    analyzer->startBattle(3, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 3), c, tb1, tb2);
}

void TestDoubles::onBattleCreated(int b, int p1, int, const TeamBattle &, const BattleConfiguration &conf, const QString &)
{
    /* We create those every time because the cost is basically nothing, but they
     * could be persistent */
    assert(b == conf.mode + 1);

    if (p1 == 1) {
        done++;
    }
    if (done == 3) {
        accept();
    }
}

#include <BattleManager/battlecommandmanager.h>
#include <BattleManager/battleinput.h>
#include <BattleManager/advancedbattledata.h>

#include "testteamcount.h"

static int done = 0;

class TestBattleTeamCount : public BattleCommandManager<TestBattleTeamCount>
{
public:
    TestBattleTeamCount(int id, AdvancedBattleData *data);

    void onChoiceSelection(int) {
        int cnt1 = data->countAlive(0);
        int cnt2 = data->countAlive(1);

        assert(cnt1 == id && cnt2 == id);

        if (!done) {
            ::done++;
        }
        done = true;
    }

    int id;
    bool done;
    AdvancedBattleData *data;
};

TestBattleTeamCount::TestBattleTeamCount(int id, AdvancedBattleData *data) : id(id), done(false), data(data) {

}

void TestTeamCount::onBattleServerConnected()
{
    BattleServerTest::onBattleServerConnected();

    Team t1;
    t1.importFromTxt(getFileContent("team1.txt").trimmed());

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Moogle";
    tb2.name = "Mystra";

    ChallengeInfo c;

    BattlePlayer bp1(tb1.name, 1), bp2(tb1.name, 2);

    for (int i = 1; i <= 6; i++) {
        bp1.teamCount = i;
        analyzer->startBattle(i, bp1, bp2, c, tb1, tb2);
    }
}

void TestTeamCount::onBattleCreated(int b, int p, int, const TeamBattle &team, const BattleConfiguration &conf, const QString &)
{
    if (battles.contains(b)) {
        confs[b]->setTeam(confs[b]->spot(p), new TeamBattle(team));
        datas[b]->reloadTeam(confs[b]->spot(p));
        return;
    }

    for (int i = 0; i < 6; i++) {
        assert(team.poke(i) == tb1.poke(i));
    }

    BattleConfiguration *cf = new BattleConfiguration(conf);

    cf->teamOwnership = true;
    cf->setTeam(cf->spot(p), new TeamBattle(team));

    BattleInput *input = new BattleInput(cf);
    AdvancedBattleData *data = new AdvancedBattleData(cf);

    input->addOutput(data);
    input->addOutput(new TestBattleTeamCount(b, data));

    battles[b] = input;
    confs[b] = cf;
    datas[b] = data;
}

void TestTeamCount::onBattleMessage(int battleid, int, const QByteArray &array)
{
    battles[battleid]->receiveData(array);

    if (done == 6) {
        accept();
    }
}

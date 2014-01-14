#include <PokemonInfo/battlestructs.h>
#include <BattleManager/battleinput.h>
#include <BattleManager/advancedbattledata.h>

#include "testinvalidpokemon.h"

static bool done = false;

class TestBattleInvalidPokemon : public BattleCommandManager<TestBattleInvalidPokemon>
{
public:
    TestBattleInvalidPokemon(AdvancedBattleData *data) : data(data) {

    }

    void onChoiceSelection(int p) {
        assert(!data->poke(p).isKoed());
        assert(data->team(p).poke(5)->num() == 0);

        /* To do: also test with rearrange team that you can't put an invalid pokemon at first place */

        done = true;
    }

    AdvancedBattleData *data;
};


void TestInvalidPokemon::onBattleServerConnected()
{
    BattleServerTest::onBattleServerConnected();

    Team t1;
    t1.importFromTxt(getFileContent("team1.txt").trimmed());

    TeamBattle tb1, tb2;

    tb1 = TeamBattle(t1);
    tb2 = TeamBattle(t1);

    tb1.name = "Moogle";
    tb2.name = "Mystra";

    tb1.poke(0).setNum(0);
    tb2.poke(0).setNum(0);

    ChallengeInfo c;
    c.rated = true;

    analyzer->startBattle(1, BattlePlayer(tb1.name, 1), BattlePlayer(tb2.name, 2), c, tb1, tb2);
}

void TestInvalidPokemon::onBattleCreated(int b, int p, int, const TeamBattle &team, const BattleConfiguration &conf, const QString &)
{
    if (battles.contains(b)) {
        confs[b]->setTeam(confs[b]->spot(p), new TeamBattle(team));
        datas[b]->reloadTeam(confs[b]->spot(p));
        return;
    }

    BattleConfiguration *cf = new BattleConfiguration(conf);

    cf->teamOwnership = true;
    cf->setTeam(cf->spot(p), new TeamBattle(team));

    BattleInput *input = new BattleInput(cf);
    AdvancedBattleData *data = new AdvancedBattleData(cf);

    input->addOutput(data);
    input->addOutput(new TestBattleInvalidPokemon(data));

    battles[b] = input;
    confs[b] = cf;
    datas[b] = data;
}

void TestInvalidPokemon::onBattleMessage(int battleid, int, const QByteArray &array)
{
    battles[battleid]->receiveData(array);

    if (done) {
        accept();
    }
}

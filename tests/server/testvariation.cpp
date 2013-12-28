#include "testvariation.h"

#include <PokemonInfo/battlestructs.h>
#include <PokemonInfo/teamholder.h>
#include <BattleManager/battleinput.h>
#include <BattleManager/battlecommandmanager.h>

static int count = 0;
static bool done = false;

class TestVariationBattle : public BattleCommandManager<TestVariationBattle>
{
public:
    void onVariation(int, int bonus, int malus) {
        assert(!(bonus == 50 && malus == 50));
        done = true;
    }
};

void TestVariation::run()
{
    createAnalyzer();
    createAnalyzer();
}

void TestVariation::onPlayerConnected()
{
    Analyzer *analyzer = sender();

    team.importFromTxt(getFileContent("team1.txt").trimmed());

    TeamHolder holder;
    holder.team() = team;

    if (count == 0) {
        holder.name() = "Weavile";
    } else if (count == 1) {
        holder.name() = "daleck";
    }

    ++count;

    /* Use different IPs to make sure we can find a battle against each other */
    analyzer->notify(NetworkCli::SetIP, holder.name());
    analyzer->login(holder,true);

    /* Timeout on test */
    setTimeout(10);
}

void TestVariation::loggedIn(const PlayerInfo &, const QStringList&)
{
    sender()->notify(NetworkCli::FindBattle, FindBattleData());
}

void TestVariation::onBattleMessage(int, const QByteArray &message)
{
    BattleInput input;
    TestVariationBattle vb;

    input.addOutput(&vb);
    input.receiveData(message);

    if (done) {
        accept();
    }
}

void TestVariation::onBattleStarted(int, const Battle &, const TeamBattle &t, const BattleConfiguration &)
{
    TeamBattle tb(team);

    for(int i = 0; i < 6; i++) {
        assert(tb.poke(i) == t.poke(i));
    }
}

#include <QCoreApplication>

#include "testunrated.h"
#include "testteamcount.h"
#include "testdoubles.h"
#include "testimposter.h"
#include "testloadplugin.h"
#include "testinvalidpokemon.h"
#include "testmeganature.h"
#include "pokemontestrunner.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PokemonTestRunner runner;
    runner.setName("battleserver");
    runner.addTest(new TestUnrated());
    runner.addTest(new TestTeamCount());
    runner.addTest(new TestDoubles());
    runner.addTest(new TestImposter());
    runner.addTest(new TestLoadPlugin());
    runner.addTest(new TestInvalidPokemon());
    runner.addTest(new TestMegaNature());
    runner.start();

    return a.exec();
}

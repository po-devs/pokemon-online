#include <QCoreApplication>

#include "testunrated.h"
#include "testteamcount.h"
#include "testdoubles.h"
#include "testimposter.h"
#include "testloadplugin.h"
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
    runner.start();

    return a.exec();
}

#include <QCoreApplication>
#include "testimportexportteam.h"
#include "testiteminfo.h"
#include "pokemontestrunner.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PokemonTestRunner runner;
    runner.setName("pokemoninfo");
    runner.addTest(new TestImportExportTeam());
    runner.addTest(new TestItemInfo());
    runner.start();

    return a.exec();
}

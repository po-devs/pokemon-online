#include <PokemonInfo/pokemonstructs.h>

#include "testimportexportteam.h"

void TestImportExportTeam::run()
{
    QString t1s = getFileContent("team1.txt").trimmed();

    assert(t1s.length() > 0);

    Team t1;
    t1.importFromTxt(getFileContent("team1.txt")); //tests/data/pokemoninfo/team1.txt

    assert(t1.poke(0).nature() == Pokemon::Calm);
    assert(t1.poke(0).nickname() == "Abomasnow");
    assert(t1.poke(0).EV(Defense) == 4);

    assert(t1.exportToTxt().trimmed() == t1s);

    emit success();
}

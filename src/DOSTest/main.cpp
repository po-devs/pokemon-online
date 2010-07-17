#include <QtGui>
#include "arg.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    GenderInfo::init("db/genders/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");

    srand(time(NULL));

    qDebug() << "Random number generator initialized";

    DosManager d;
    (void) d;

    return a.exec();
}

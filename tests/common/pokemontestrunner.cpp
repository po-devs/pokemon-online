#include <PokemonInfo/pokemoninfo.h>
#include <PokemonInfo/movesetchecker.h>

#include "pokemontestrunner.h"

PokemonTestRunner::PokemonTestRunner(QObject *parent) :
    TestRunner(parent)
{
    loadDatabase();
}

void PokemonTestRunner::loadDatabase()
{
    /* Really useful for headless servers */
    GenInfo::init("db/gens/");
    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
    GenderInfo::init("db/genders/");
}

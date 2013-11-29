#include <PokemonInfo/pokemoninfo.h>
#include "testiteminfo.h"

void TestItemInfo::run()
{
    QStringList usefulItems = ItemInfo::SortedUsefulNames(Pokemon::gen());

    assert(usefulItems.length() > 1);
    assert(usefulItems[0] == ItemInfo::Name(0));
    assert(ItemInfo::Number(usefulItems[1]) > 0);

    emit success();
}

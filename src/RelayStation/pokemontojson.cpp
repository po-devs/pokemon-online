#include "../PokemonInfo/battlestructs.h"
#include "pokemontojson.h"

QVariantMap toJson(const Pokemon::gen &gen)
{
    QVariantMap ret;
    ret.insert("num", int(char(gen.num)));
    ret.insert("subnum", int(char(gen.subnum)));

    return ret;
}

QVariantMap toJson(const BattleConfiguration &c)
{
    QVariantMap ret;
    ret.insert("gen", toJson(c.gen));
    ret.insert("mode", c.mode);
    ret.insert("players", QVariantList() << c.ids[0] << c.ids[1]);
    ret.insert("clauses", c.clauses);
    //ret.insert("avatars", QVariantList() << c.avatar[0] << c.avatar[1]);
    ret.insert("rated", bool(c.flags[0]));

    return ret;
}

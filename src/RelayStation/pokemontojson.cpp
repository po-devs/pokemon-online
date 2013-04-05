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

QVariantMap toJson(const ShallowBattlePoke &poke)
{
    QVariantMap ret;
    ret.insert("num", poke.num().pokenum);
    if (poke.num().subnum) {
        ret.insert("forme", poke.num().subnum);
    }
    ret.insert("name", poke.nick());
    ret.insert("level", poke.level());
    if (poke.gender()) {
        ret.insert("gender", poke.gender());
    }
    if (poke.shiny()) {
        ret.insert("shiny", poke.shiny());
    }
    ret.insert("percent", poke.lifePercent());
    if (poke.status() != Pokemon::Fine) {
        ret.insert("status", poke.status());
    }

    return ret;
}

QVariant toJson(const TeamBattle &team)
{
    QVariantList ret;
    for (int i = 0; i < 6; i++) {
        if (team.poke(i).num() != Pokemon::NoPoke) {
            ret.push_back(toJson(team.poke(i)));
        }
    }

    return ret;
}

QVariantMap toJson(const PokeBattle &poke)
{
    QVariantMap ret;
    ret.insert("num", poke.num().pokenum);
    if (poke.num().subnum) {
        ret.insert("forme", poke.num().subnum);
    }
    ret.insert("name", poke.nick());
    ret.insert("level", poke.level());
    if (poke.gender()) {
        ret.insert("gender", poke.gender());
    }
    if (poke.shiny()) {
        ret.insert("shiny", poke.shiny());
    }
    ret.insert("percent", poke.lifePercent());
    if (poke.status() != Pokemon::Fine) {
        ret.insert("status", poke.status());
    }
    ret.insert("life", poke.lifePoints());
    ret.insert("totalLife", poke.totalLifePoints());
    ret.insert("happiness", poke.happiness());

    QVariantList moves;
    for (int i = 0; i < 4; i++) {
        moves.push_back(toJson(poke.move(i)));
    }
    ret.insert("moves", moves);

    QVariantList evs;
    for (int i = 0; i < 6; i++) {
        evs.push_back(poke.evs()[i]);
    }
    QVariantList ivs;
    for (int i = 0; i < 6; i++) {
        ivs.push_back(poke.dvs()[i]);
    }
    ret.insert("evs", evs);
    ret.insert("ivs", ivs);

    return ret;
}

QVariantMap toJson(const BattleMove &move)
{
    QVariantMap ret;
    ret.insert("move", move.num());
    ret.insert("pp", move.PP());
    ret.insert("totalpp", move.totalPP());

    return ret;
}

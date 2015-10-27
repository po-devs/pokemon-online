#include <utility>
#include <PokemonInfo/battlestructs.h>
#include <PokemonInfo/networkstructs.h>
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
    ret.insert("avatars", QVariantList() << c.avatar[0] << c.avatar[1]);
    ret.insert("rated", bool(c.flags[0]));
    //ret.insert("names", QVariantList() << c.teams[0]->name << c.teams[1]->name);

    return ret;
}

QVariantMap toJson(const BattleChoices &choices)
{
    QVariantMap ret;

    ret.insert("slot", choices.numSlot);
    ret.insert("switch", choices.switchAllowed);
    ret.insert("attack", choices.attacksAllowed);
    ret.insert("mega", choices.mega);

    QVariantList list;
    for (int i = 0; i < 4; i++) {
        list.push_back(choices.attackAllowed[i]);
    }
    ret.insert("attacks", list);

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

QVariant toJson(const ShallowShownTeam &team)
{
    QVariantList ret;
    for (int i = 0; i < 6; i++) {
        if (team.poke(i).num != Pokemon::NoPoke) {
            ret.push_back(toJson(team.poke(i)));
        }
    }

    return ret;
}

QVariantMap toJson(const ShallowShownPoke &poke)
{
    QVariantMap ret;

    ret.insert("num", poke.num.pokenum);
    if (poke.num.subnum) {
        ret.insert("forme", poke.num.subnum);
    }

    ret.insert("level", poke.level);
    if (poke.gender) {
        ret.insert("gender", poke.gender);
    }
    ret.insert("heldItem", poke.item);

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

    ret.insert("item", poke.item());
    ret.insert("ability", poke.ability());

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

template <>
BattleChoice fromJson<BattleChoice>(const QVariantMap &v){
    static QStringList bchoices =  QStringList() << "cancel" << "attack" << "switch" << "rearrange" << "shiftcenter" << "tie" << "item";

    BattleChoice info;

    info.type = std::max(bchoices.indexOf(v.value("type").toString()), 0);
    info.playerSlot = v.value("slot").toInt();

    if (info.type == AttackType) {
        info.choice.attack.attackSlot = v.value("attackSlot").toInt();
        info.choice.attack.mega = v.value("mega").toBool();
        if (v.value("target").isValid()) {
            info.choice.attack.attackTarget = v.value("target").toInt();
        } else {
            info.choice.attack.attackTarget = !info.playerSlot;
        }
    } else if (info.type == SwitchType) {
        info.choice.switching.pokeSlot = v.value("pokeSlot").toInt();
    } else if (info.type == RearrangeType) {
        if (v.value("neworder").canConvert<QVariantList>()) {
            QVariantList list = v.value("neworder").toList();
            for (int i = 0; i < list.length() && i < 6; i++) {
                info.choice.rearrange.pokeIndexes[i] = list.value(i).toInt();
            }
        }
    } else if (info.type == ItemType) {
        info.choice.item.item = v.value("item").toInt();
        info.choice.item.target = v.value("target").toInt();
        info.choice.item.attack = v.value("attack").toInt();
    }

    return info;
}

template <>
TrainerInfo fromJson<TrainerInfo>(const QVariantMap &map){
    TrainerInfo info;

    info.avatar = map.value("avatar").toInt();
    info.info = map.value("info").toString();

    return info;
}

template<>
PersonalTeam fromJson<PersonalTeam>(const QVariantMap &map) {
    PersonalTeam ret;

    bool isIllegal = map.value("illegal").toBool();

    ret.defaultTier() = map.value("tier").toString();
    ret.gen() = fromJson<Pokemon::gen>(map.value("gen").toMap());

    const auto & list = map.value("pokes").toList();

    for (int i = 0; i < std::min(6, list.length()); i++) {
        ret.poke(i) = fromJson<PokePersonal>(list[i].toMap());
        ret.poke(i).gen() = ret.gen();
        ret.poke(i).illegal() = isIllegal;
    }

    return ret;
}

template<>
PokePersonal fromJson<PokePersonal>(const QVariantMap &map) {
    PokePersonal ret;
    ret.reset();
    ret.num().pokenum = map.value("num").toInt();
    ret.num().subnum = map.value("forme").toInt();
    ret.nickname() = map.value("nick", "").toString();
    ret.ability() = map.value("ability").toInt();
    ret.item() = map.value("item").toInt();
    ret.nature() = map.value("nature").toInt();
    ret.level() = map.value("level").toInt();
    ret.happiness() = map.value("happiness").toInt();
    ret.gender() = map.value("gender").toInt();

    const auto &moves = map.value("moves").toList();
    for (int i = 0; i < std::min(4, moves.length()); i++) {
        ret.setMove(moves[i].toInt(), i);
    }

    const auto &evs = map.value("evs").toList();
    for (int i = 0; i < std::min(6, evs.length()); i++) {
        ret.setEV(i, evs[i].toInt(), true);
    }

    const auto &ivs = map.value("ivs").toList();
    for (int i = 0; i < std::min(6, ivs.length()); i++) {
        ret.setDV(i, ivs[i].toInt());
    }

    return ret;
}

template <>
Pokemon::gen fromJson<Pokemon::gen>(const QVariantMap &map)
{
    Pokemon::gen ret;

    if (map.contains("num")) {
        ret.num = map.value("num").toInt();
        ret.subnum = GenInfo::NumberOfSubgens(ret.num) - 1;
    }
    if (map.contains("subnum")) {
        ret.subnum = map.value("subnum").toInt();
    }

    return ret;
}

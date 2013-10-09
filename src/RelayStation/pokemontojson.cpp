#include <utility>
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

static QStringList bchoices =  QStringList() << "cancel" << "attack" << "switch" << "rearrange" << "shiftcenter" << "tie" << "item";

BattleChoice&& fromJson(const QVariantMap &v) {
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

    return std::move(info);
}

#include "battletojson.h"
#include "pokemontojson.h"

#define makeCommand(command) map.insert("command", command); map.insert("spot", spot)

void BattleToJson::onKo(int spot)
{
    makeCommand("ko");
}

void BattleToJson::onSendOut(int spot, int player, ShallowBattlePoke *pokemon, bool silent)
{
    makeCommand("send");
    map.insert("player", player);
    map.insert("silent", silent);
    map.insert("pokemon", toJson(*pokemon));
}

void BattleToJson::onSendBack(int spot, bool silent)
{
    makeCommand("sendback");
    map.insert("silent", silent);
}

void BattleToJson::onUseAttack(int spot, int attack, bool silent)
{
    makeCommand("move");
    map.insert("move", attack);
    map.insert("silent", silent);
}

void BattleToJson::onBeginTurn(int turn)
{
    map.insert("command", "turn");
    map.insert("turn", turn);
}

void BattleToJson::onHpChange(int spot, int newHp)
{
    makeCommand("hpchange");
    map.insert("newHP", newHp);
}

void BattleToJson::onHitCount(int spot, int count)
{
    makeCommand("hitcount");
    map.insert("count", count);
}

void BattleToJson::onEffectiveness(int spot, int effectiveness)
{
    makeCommand("effectiveness");
    map.insert("effectiveness", effectiveness);
}

void BattleToJson::onCriticalHit(int spot)
{
    makeCommand("critical");
}

void BattleToJson::onMiss(int spot)
{
    makeCommand("miss");
}

void BattleToJson::onAvoid(int spot)
{
    makeCommand("miss");
}

void BattleToJson::onStatBoost(int spot, int stat, int boost, bool silent)
{
    makeCommand("boost");
    map.insert("stat", stat);
    map.insert("boost", boost);
    map.insert("silent", silent);
}

void BattleToJson::onMajorStatusChange(int spot, int status, bool multipleTurns, bool silent)
{
    makeCommand("status");
    map.insert("status", status);
    map.insert("multiple", multipleTurns);
    map.insert("silent", silent);
}

void BattleToJson::onDamageDone(int spot, int damage)
{
    makeCommand("damage");
    map.insert("damage", damage);
}

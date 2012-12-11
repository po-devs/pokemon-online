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

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
    map.insert("slot", player);
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

//    void onPokeballStatusChanged(int player, int poke, int status);

void BattleToJson::onStatusAlreadyThere(int spot, int status)
{
    makeCommand("alreadystatus");
    map.insert("status", status);
}

void BattleToJson::onStatusNotification(int spot, int status)
{
    makeCommand("feelstatus");
    map.insert("status", status);
}

void BattleToJson::onStatusOver(int spot, int status)
{
    makeCommand("freestatus");
    map.insert("status", status);
}

void BattleToJson::onStatusDamage(int spot, int status)
{
    makeCommand("statusdamage");
    map.insert("status", status);
}

void BattleToJson::onAttackFailing(int spot, bool silent)
{
    makeCommand("fail");
    map.insert("silent", silent);
}

void BattleToJson::onPlayerMessage(int spot, const QString &message)
{
    makeCommand("playerchat");
    map.insert("message", message);
}

void BattleToJson::onSpectatorJoin(int id, const QString &name)
{
    map.insert("command", "spectatorjoin");
    map.insert("id", id);
    map.insert("name", name);
}

void BattleToJson::onSpectatorLeave(int id)
{
    map.insert("command", "spectatorleave");
    map.insert("id", id);
}

void BattleToJson::onSpectatorChat(int id, const QString &message)
{
    map.insert("command", "spectatorchat");
    map.insert("id", id);
    map.insert("message", message);
}

void BattleToJson::onMoveMessage(int spot, int move, int part, int type, int foe, int other, const QString &data)
{
    makeCommand("movemessage");
    map.insert("move", move);
    map.insert("part", part);
    map.insert("type", type);
    map.insert("foe", foe);
    map.insert("other", other);
    map.insert("data", data);
}

void BattleToJson::onNoTarget(int spot)
{
    makeCommand("notarget");
}

void BattleToJson::onItemMessage(int spot, int item, int part, int foe, int berry, int other)
{
    makeCommand("itemmessage");
    map.insert("item", item);
    map.insert("part", part);
    map.insert("foe", foe);
    map.insert("berry", berry);
    map.insert("other", other);
}

void BattleToJson::onFlinch(int spot)
{
    makeCommand("flinch");
}

void BattleToJson::onRecoil(int spot)
{
    makeCommand("recoil");
}

void BattleToJson::onDrained(int spot)
{
    makeCommand("drain");
}

void BattleToJson::onStartWeather(int spot, int weather, bool ability)
{
    makeCommand("weatherstart");
    map.insert("weather", weather);
    map.insert("permanent", ability);
}

void BattleToJson::onContinueWeather(int weather)
{
    map.insert("command", "feelweather");
    map.insert("weather", weather);
}

void BattleToJson::onEndWeather(int weather)
{
    map.insert("command", "weatherend");
    map.insert("weather", weather);
}

void BattleToJson::onHurtWeather(int spot, int weather)
{
    makeCommand("weatherhurt");
    map.insert("weather", weather);
}

void BattleToJson::onDamageDone(int spot, int damage)
{
    makeCommand("damage");
    map.insert("damage", damage);
}

void BattleToJson::onAbilityMessage(int spot, int ab, int part, int type, int foe, int other)
{
    makeCommand("abilitymessage");
    map.insert("ability", ab);
    map.insert("part", part);
    map.insert("type", type);
    map.insert("foe", foe);
    map.insert("other", other);
}

void BattleToJson::onSubstituteStatus(int spot, bool substitute)
{
    makeCommand("substitute");
    map.insert("substitute", substitute);
}

//    void onBlankMessage();
//    void onClauseActivated(int clause);

void BattleToJson::onRatedNotification(bool rated)
{
    map.insert("command", "rated");
    map.insert("rated", rated);
}

void BattleToJson::onTierNotification(const QString &tier)
{
    map.insert("command", "tier");
    map.insert("tier", tier);
}

//    void onDynamicInfo(int spot, const BattleDynamicInfo &info);
//    void onPokemonVanish(int spot);
//    void onPokemonReappear(int spot);
//    void onSpriteChange(int spot, int newSprite);
//    void onDefiniteFormeChange(int spot, int poke, int newPoke);
//    void onCosmeticFormeChange(int spot, int subforme);
//    void onClockStart(int player, int time);
//    void onClockStop(int player, int time);
//    void onShiftSpots(int player, int spot1, int spot2, bool silent);

void BattleToJson::onBattleEnd(int res, int winner)
{
    map.insert("command", "battleend");
    map.insert("result", res);
    map.insert("winner", winner);
}

void BattleToJson::onOfferChoice(int player, const BattleChoices &choice)
{
    map.insert("command", "offerchoice");
    map.insert("player", player);
    map.insert("choice", toJson(choice));
}

void BattleToJson::onPPChange(int spot, int move, int PP)
{
    makeCommand("ppchange");
    map.insert("move", move);
    map.insert("pp", PP);
}

//    void onOfferChoice(int player, const BattleChoices &choice);
//    void onTempPPChange(int spot, int move, int PP);
//    void onMoveChange(int spot, int slot, int move, bool definite);
void BattleToJson::onRearrangeTeam(int player, const ShallowShownTeam& team)
{
    map.insert("command", "teampreview");
    map.insert("player", player);
    map.insert("team", toJson(team));
}

void BattleToJson::onChoiceSelection(int spot)
{
    makeCommand("choiceselection");
}

void BattleToJson::onChoiceCancellation(int player)
{
    map.insert("command", "choicecancellation");
    map.insert("player", player);
}

void BattleToJson::onVariation(int player, int bonus, int malus)
{
    map.insert("command", "variation");
    map.insert("player", player);
    map.insert("bonus", bonus);
    map.insert("malus", malus);
}

//    void onDynamicStats(int spot, const BattleStats& stats);
//    void onPrintHtml(const QString &html);

void BattleToJson::onReconnect(int player)
{
    map.insert("command", "reconnect");
    map.insert("player", player);
}

void BattleToJson::onDisconnect(int player)
{
    map.insert("command", "disconnect");
    map.insert("player", player);
}

//    void onAttackChosen(int spot, int attackSlot, int target);
//    void onSwitchChosen(int spot, int pokeSlot);
//    void onTeamOrderChosen(int player, const RearrangeChoice &rearrange);
//    void onChoiceCancelled(int player);
//    void onShiftToCenterChosen(int player);
//    void onDrawRequest(int player);
//    void onUseItem(int player, int item);
//    void onItemChangeCount(int player, int item, int count);

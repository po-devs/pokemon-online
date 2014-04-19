#ifndef BATTLESCRIPTING_H
#define BATTLESCRIPTING_H

#include <QScriptEngine>
#include <Utilities/functions.h>
#include <BattleManager/battlecommandmanager.h>

/***
  Todo:
  Convert the following to script value so that functions can be complete:
    makeEvent("onDynamicInfo", spot, const BattleDynamicInfo &info);
    makeEvent("onOfferChoice", player, const BattleChoices &choice);
    makeEvent("onRearrangeTeam", player, const ShallowShownTeam& team);
    makeEvent("onDynamicStats", spot, const BattleStats& stats);
    makeEvent("onTeamOrderChosen", player, const RearrangeChoice &rearrange);
***/

class BaseBattleWindowInterface;

class BattleScripting : public QObject, public BattleCommandManager<BattleScripting>
{
    Q_OBJECT
public:
    BattleScripting(QScriptEngine* engine, BaseBattleWindowInterface *interf);

    void evaluate(const QScriptValue &expr);
    void printLine(const QString &string);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);
    /* To show a move in the animated battle window */
    static QScriptValue simulateMove(QScriptContext *context, QScriptEngine *engine);

    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool silent);

    void onKo(int spot) {
        makeEvent("onKo", spot);
    }
    void onSendBack(int spot, bool silent) {
        makeEvent("onSendBack", spot, silent);
    }
    void onUseAttack(int spot, int attack, bool silent) {
        makeEvent("onUseAttack", spot, attack, silent);
    }
    void onBeginTurn(int turn) {
        makeEvent("onBeginTurn", turn);
    }
    void onHpChange(int spot, int newHp) {
        makeEvent("onHpChange", spot, newHp);
    }
    void onHitCount(int spot, int count) {
        makeEvent("onHitCount", spot, count);
    }
    void onEffectiveness(int spot, int effectiveness) {
        makeEvent("onEffectiveness", spot, effectiveness);
    }
    void onCriticalHit(int spot) {
        makeEvent("onCriticalHit", spot);
    }
    void onMiss(int spot) {
        makeEvent("onMiss", spot);
    }
    void onAvoid(int spot) {
        makeEvent("onAvoid", spot);
    }
    void onStatBoost(int spot, int stat, int boost, bool silent) {
        makeEvent("onStatBoost", spot, stat, boost, silent);
    }
    void onMajorStatusChange(int spot, int status, bool multipleTurns, bool silent) {
        makeEvent("onMajorStatusChange", spot, status, multipleTurns, silent);
    }
    void onPokeballStatusChanged(int player, int poke, int status) {
        makeEvent("onPokeballStatusChanged", player, poke, status);
    }
    void onStatusAlreadyThere(int spot, int status) {
        makeEvent("onStatusAlreadyThere", spot, status);
    }
    void onStatusNotification(int spot, int status) {
        makeEvent("onStatusNotification", spot, status);
    }
    void onStatusDamage(int spot, int status) {
        makeEvent("onStatusDamage", spot, status);
    }
    void onStatusOver(int spot, int status) {
        makeEvent("onStatusOver", spot, status);
    }
    void onAttackFailing(int spot, bool fail) {
        makeEvent("onAttackFailing", spot, fail);
    }
    void onPlayerMessage(int spot, const QString& message) {
        makeEvent("onPlayerMessage", spot, message);
    }
    void onSpectatorJoin(int id, const QString& name) {
        makeEvent("onSpectatorJoin", id, name);
    }
    void onSpectatorLeave(int id) {
        makeEvent("onSpectatorLeave", id);
    }
    void onSpectatorChat(int id, const QString& message) {
        makeEvent("onSpectatorChat", id, message);
    }
    void onMoveMessage(int spot, int move, int part, int type, int foe, int other, const QString &data) {
        makeEvent("onMoveMessage", spot, move, part, type, foe, other, data);
    }
    void onNoTarget(int spot) {
        makeEvent("onNoTarget", spot);
    }
    void onItemMessage(int spot, int item, int part, int foe, int berry, int other) {
        makeEvent("onItemMessage", spot, item, part, foe, berry, other);
    }
    void onFlinch(int spot) {
        makeEvent("onFlinch", spot);
    }
    void onRecoil(int spot) {
        makeEvent("onRecoil", spot);
    }
    void onDrained(int spot) {
        makeEvent("onDrained", spot);
    }
    void onStartWeather(int spot, int weather, bool ability) {
        makeEvent("onStartWeather", spot, weather, ability);
    }
    void onContinueWeather(int weather) {
        makeEvent("onContinueWeather", weather);
    }
    void onEndWeather(int weather) {
        makeEvent("onEndWeather", weather);
    }
    void onHurtWeather(int spot, int weather) {
        makeEvent("onHurtWeather", spot, weather);
    }
    void onDamageDone(int spot, int damage) {
        makeEvent("onDamageDone", spot, damage);
    }
    void onAbilityMessage(int spot, int ab, int part, int type, int foe, int other) {
        makeEvent("onAbilityMessage", spot, ab, part, type, foe, other);
    }
    void onSubstituteStatus(int spot, bool substitute) {
        makeEvent("onSubstituteStatus", spot, substitute);
    }
    void onBlankMessage() {
        makeEvent("onBlankMessage");
    }
    void onClauseActivated(int clause) {
        makeEvent("onClauseActivated", clause);
    }
    void onRatedNotification(bool rated) {
        makeEvent("onRatedNotification", rated);
    }
    void onTierNotification(const QString &tier) {
        makeEvent("onTierNotification", tier);
    }
    void onDynamicInfo(int spot, const BattleDynamicInfo &info) {
        (void) info;
        makeEvent("onDynamicInfo", spot/*, const BattleDynamicInfo &info*/);
    }
    void onPokemonVanish(int spot) {
        makeEvent("onPokemonVanish", spot);
    }
    void onPokemonReappear(int spot) {
        makeEvent("onPokemonReappear", spot);
    }
    void onSpriteChange(int spot, int newSprite) {
        makeEvent("onSpriteChange", spot, newSprite);
    }
    void onDefiniteFormeChange(int spot, int poke, int newPoke) {
        makeEvent("onDefiniteFormeChange", spot, poke, newPoke);
    }
    void onCosmeticFormeChange(int spot, int subforme) {
        makeEvent("onCosmeticFormeChange", spot, subforme);
    }
    void onClockStart(int player, int time) {
        makeEvent("onClockStart", player, time);
    }
    void onClockStop(int player, int time) {
        makeEvent("onClockStop", player, time);
    }
    void onShiftSpots(int player, int spot1, int spot2, bool silent) {
        makeEvent("onShiftSpots", player, spot1, spot2, silent);
    }
    void onBattleEnd(int res, int winner) {
        makeEvent("onBattleEnd", res, winner);
    }
    void onPPChange(int spot, int move, int PP) {
        makeEvent("onPPChange", spot, move, PP);
    }
    void onOfferChoice(int player, const BattleChoices &choice) {
        (void) choice;
        makeEvent("onOfferChoice", player/*, const BattleChoices &choice*/);
    }
    void onTempPPChange(int spot, int move, int PP) {
        makeEvent("onTempPPChange", spot, move, PP);
    }
    void onMoveChange(int spot, int slot, int move, bool definite) {
        makeEvent("onMoveChange", spot, slot, move, definite);
    }
    void onRearrangeTeam(int player, const ShallowShownTeam& team) {
        (void) team;
        makeEvent("onRearrangeTeam", player/*, const ShallowShownTeam& team*/);
    }
    void onChoiceSelection(int player) {
        makeEvent("onChoiceSelection", player);
    }
    void onChoiceCancellation(int player) {
        makeEvent("onChoiceCancellation", player);
    }
    void onVariation(int player, int bonus, int malus) {
        makeEvent("onVariation", player, bonus, malus);
    }
    void onDynamicStats(int spot, const BattleStats& stats) {
        (void) stats;
        makeEvent("onDynamicStats", spot/*, const BattleStats& stats*/);
    }
    void onPrintHtml(const QString &html) {
        makeEvent("onPrintHtml", html);
    }
    void onReconnect(int player) {
        makeEvent("onReconnect", player);
    }
    void onDisconnect(int player) {
        makeEvent("onDisconnect", player);
    }
    void onAttackChosen(int spot, int attackSlot, int target) {
        makeEvent("onAttackChosen", spot, attackSlot, target);
    }
    void onSwitchChosen(int spot, int pokeSlot) {
        makeEvent("onSwitchChosen", spot, pokeSlot);
    }
    void onTeamOrderChosen(int player, const RearrangeChoice &rearrange) {
        (void) rearrange;
        makeEvent("onTeamOrderChosen", player/*, const RearrangeChoice &rearrange*/);
    }
    void onChoiceCancelled(int player) {
        makeEvent("onChoiceCancelled", player);
    }
    void onShiftToCenterChosen(int player) {
        makeEvent("onShiftToCenterChosen", player);
    }
    void onDrawRequest(int player) {
        makeEvent("onDrawRequest", player);
    }

    template <typename ...Params>
    void makeEvent(const QString &event, Params&&... params);
private:
    QScriptEngine *myengine;
    BaseBattleWindowInterface *myinterface;
};


template<typename ...Params>
void BattleScripting::makeEvent(const QString &event, Params &&... params)
{
    QScriptValue myscript = myengine->globalObject().property("script");
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    QScriptValueList l;
    evaluate(myscript.property(event).call(myscript, pack(l, params...)));
}

#endif // BATTLESCRIPTING_H

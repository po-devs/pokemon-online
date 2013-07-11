#ifndef BATTLETOJSON_H
#define BATTLETOJSON_H

#include "../BattleManager/battlecommandmanager.h"
#include "../BattleManager/battledata.h"
#include "battletojsonflow.h"
#include <QObject>

class BattleDynamicInfo;

class BattleToJson : public QObject, public BattleCommandManager<BattleToJson, BattleToJsonFlow<BattleEnum, BattleToJson> >
{
    friend class BattleToJsonFlow<BattleEnum, BattleToJson>;

    Q_OBJECT
public:
    void onKo(int spot);
    void onSendOut(int spot, int slot, ShallowBattlePoke* pokemon, bool silent);
    void onSendBack(int spot, bool silent);
    void onUseAttack(int spot, int attack, bool silent);
    void onBeginTurn(int turn);
    void onHpChange(int spot, int newHp);
    void onHitCount(int spot, int count);
    void onEffectiveness(int spot, int effectiveness);
    void onCriticalHit(int spot);
    void onMiss(int spot);
    void onAvoid(int spot);
    void onStatBoost(int spot, int stat, int boost, bool silent);
    void onMajorStatusChange(int spot, int status, bool multipleTurns, bool silent);
//    void onPokeballStatusChanged(int player, int poke, int status);
    void onStatusAlreadyThere(int spot, int status);
    void onStatusNotification(int spot, int status);
    void onStatusDamage(int spot, int status);
    void onStatusOver(int spot, int status);
    void onAttackFailing(int spot, bool silent);
    void onPlayerMessage(int spot, const QString &message);
    void onSpectatorJoin(int id, const QString &name);
    void onSpectatorLeave(int id);
    void onSpectatorChat(int id, const QString &message);
    void onMoveMessage(int spot, int move, int part, int type, int foe, int other, const QString &data);
    void onNoTarget(int spot);
    void onItemMessage(int spot, int item, int part, int foe, int berry, int other);
    void onFlinch(int spot);
    void onRecoil(int spot);
    void onDrained(int spot);
    void onStartWeather(int spot, int weather, bool ability);
    void onContinueWeather(int weather);
    void onEndWeather(int weather);
    void onHurtWeather(int spot, int weather);
    void onDamageDone(int spot, int damage);
    void onAbilityMessage(int spot, int ab, int part, int type, int foe, int other);
    void onSubstituteStatus(int spot, bool substitute);
//    void onBlankMessage();
//    void onClauseActivated(int clause);
    void onRatedNotification(bool rated);
    void onTierNotification(const QString &tier);
//    void onDynamicInfo(int spot, const BattleDynamicInfo &info);
//    void onPokemonVanish(int spot);
//    void onPokemonReappear(int spot);
//    void onSpriteChange(int spot, int newSprite);
//    void onDefiniteFormeChange(int spot, int poke, int newPoke);
//    void onCosmeticFormeChange(int spot, int subforme);
//    void onClockStart(int player, int time);
//    void onClockStop(int player, int time);
//    void onShiftSpots(int player, int spot1, int spot2, bool silent);
    void onBattleEnd(int res, int winner);
//    void onPPChange(int spot, int move, int PP);
    void onOfferChoice(int player, const BattleChoices &choice);
//    void onTempPPChange(int spot, int move, int PP);
//    void onMoveChange(int spot, int slot, int move, bool definite);
//    void onRearrangeTeam(int player, const ShallowShownTeam& team);
    void onChoiceSelection(int player);
//    void onChoiceCancellation(int player);
//    void onVariation(int player, int bonus, int malus);
//    void onDynamicStats(int spot, const BattleStats& stats);
//    void onPrintHtml(const QString &html);
//    void onReconnect(int player);
//    void onDisconnect(int player);
//    void onAttackChosen(int spot, int attackSlot, int target);
//    void onSwitchChosen(int spot, int pokeSlot);
//    void onTeamOrderChosen(int player, const RearrangeChoice &rearrange);
//    void onChoiceCancelled(int player);
//    void onShiftToCenterChosen(int player);
//    void onDrawRequest(int player);
//    void onUseItem(int player, int item);
//    void onItemChangeCount(int player, int item, int count);

    QVariantMap getCommand() const {if (updated) {updated = false; return map;} return QVariantMap();}
    void sendCommand() {emit message(map); }
signals:
    /* JSon conversion of the message */
    void message(const QVariant&);
protected:
    QVariantMap map;
    mutable bool updated;
};

#endif // BATTLETOJSON_H

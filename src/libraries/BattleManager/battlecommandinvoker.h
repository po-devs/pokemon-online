#ifndef BATTLECOMMANDINVOKER_H
#define BATTLECOMMANDINVOKER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <QString>
#include "test.h"
#include "battleenum.h"
#include "../PokemonInfo/battlestructs.h"

/*
class ShallowBattlePoke;
class BattleDynamicInfo;
class RearrangeChoice;
*/

template <class Underling>
class BattleCommandInvoker
{
public:
    typedef BattleEnum enumClass;
    typedef Underling workerClass;

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        /* Since no function partial specialisation, using overload */
        invoke2(Param<val>(), std::forward<Params>(params)...);
    }

protected:
    typedef std::shared_ptr<QString> *string_ptr;

    template <BattleEnum val>
    struct Param
    {

    };

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }

#define start(en, func, ...) \
    template<class Y=workerClass> \
    typename test<decltype(&Y::func)>::type invoke2(Param<BattleEnum::en>, ##__VA_ARGS__) {

#define end(func, ...) \
       wc()->func(__VA_ARGS__); \
    }

    /* This is what the macros below look like - we check if the workerClass has a function,
      and if it does, we call it */
    template<class Y=workerClass>
    typename test<decltype(&Y::onKo)>::type invoke2(Param<BattleEnum::Ko>, uint8_t spot) {
        wc()->onKo(spot);
    }

    template<class Y=workerClass>
    typename test<decltype(&Y::onSendOut)>::type
    invoke2(Param<BattleEnum::SendOut>, uint8_t spot, uint8_t prevIndex, std::shared_ptr<ShallowBattlePoke> *ptr, bool silent) {
        wc()->onSendOut(spot, prevIndex, ptr->get(), silent);
    }

    template<class Y=workerClass>
    typename test<decltype(&Y::onDynamicInfo)>::type
    invoke2(Param<BattleEnum::StatBoostsAndField>, int &spot, BattleDynamicInfo &info) {
        wc()->onDynamicInfo(spot, info);
    }

    /* Todo: expand following macros */
    start(SendBack, onSendBack, int spot, bool silent) end(onSendBack, spot, silent)
    start(UseAttack, onUseAttack, int spot, int attack, bool silent) end(onUseAttack, spot, attack, silent)
    start(Turn, onBeginTurn, int turn) end(onBeginTurn, turn)
    start(NewHp, onHpChange, int spot, int newHp) end(onHpChange, spot, newHp)
    start(Hits, onHitCount, int spot, int count) end(onHitCount, spot, count)
    start(Effectiveness, onEffectiveness, int spot, int effectiveness) end(onEffectiveness, spot, effectiveness)
    start(CriticalHit, onCriticalHit, int spot) end(onCriticalHit, spot)
    start(Miss, onMiss, int spot) end(onMiss, spot)
    start(Avoid, onAvoid, int spot) end(onAvoid, spot)
    start(StatChange, onStatBoost, int spot, int stat, int boost, bool silent) end(onStatBoost, spot, stat, boost, silent)
    start(ClassicStatusChange, onMajorStatusChange, int spot, int status, bool multipleTurns, bool silent) end(onMajorStatusChange, spot, status, multipleTurns, silent)
    start(AbsoluteStatusChange, onPokeballStatusChanged, int player, int poke, int status) end(onPokeballStatusChanged, player, poke, status)
    start(AlreadyStatusMessage, onStatusAlreadyThere, int spot, int status) end(onStatusAlreadyThere, spot, status)
    start(StatusFeel, onStatusNotification, int spot, int status) end(onStatusNotification, spot, status)
    start(StatusHurt, onStatusDamage, int spot, int status) end(onStatusDamage, spot, status)
    start(StatusFree, onStatusOver, int spot, int status) end(onStatusOver, spot, status)
    start(Fail, onAttackFailing, int spot, bool silent) end(onAttackFailing, spot, silent)
    start(PlayerMessage, onPlayerMessage, int spot, string_ptr message) end(onPlayerMessage, spot, *message->get())
    start(SpectatorEnter, onSpectatorJoin, int id, string_ptr name) end(onSpectatorJoin, id, *name->get())
    start(SpectatorLeave, onSpectatorLeave, int id) end(onSpectatorLeave, id)
    start(SpectatorMessage, onSpectatorChat, int id, string_ptr message) end(onSpectatorChat, id, *message->get())
    start(MoveMessage, onMoveMessage, int spot, int move, int part, int type, int foe, int other, string_ptr data)
        end(onMoveMessage, spot, move, part, type, foe, other, *data->get())
    start(NoTargetMessage, onNoTarget, int spot) end(onNoTarget, spot)
    start(ItemMessage, onItemMessage, int spot, int item, int part, int foe, int berry, int other)
        end(onItemMessage, spot, item, part, foe, berry, other)
    start(Flinch, onFlinch, int spot) end(onFlinch, spot)
    start(Recoil, onRecoil, int spot) end(onRecoil, spot)
    start(Drained, onDrained, int spot) end(onDrained, spot)
    start(WeatherMessage, onContinueWeather, int weather) end(onContinueWeather, weather)
    start(EndWeather, onEndWeather, int weather) end(onEndWeather, weather)
    start(StartWeather, onStartWeather, int spot, int weather, bool ability) end(onStartWeather, spot, weather, ability)
    start(WeatherDamage, onHurtWeather, int spot, int weather) end(onHurtWeather, spot, weather)
    start(Damaged, onDamageDone, int spot, int damage) end(onDamageDone, spot, damage)
    start(AbilityMessage, onAbilityMessage, int spot, int ab, int part, int type, int foe, int other)
        end(onAbilityMessage, spot, ab, part, type, foe, other)
    start(SubstituteStatus, onSubstituteStatus, int spot, bool substitute) end(onSubstituteStatus, spot, substitute)
    start(BlankMessage, onBlankMessage) end (onBlankMessage)
    start(BattleEnd, onBattleEnd, int res, int winner) end(onBattleEnd, res, winner)
    start(ClauseMessage, onClauseActivated, int clause) end(onClauseActivated, clause)
    start(RatedInfo, onRatedNotification, bool rated) end(onRatedNotification, rated)
    start(TierInfo, onTierNotification, string_ptr tier) end(onTierNotification, *tier->get())
    start(PokemonVanish, onPokemonVanish, int spot) end(onPokemonVanish, spot)
    start(PokemonReappear, onPokemonReappear, int spot) end(onPokemonReappear, spot)
    start(SpriteChange, onSpriteChange, int spot, int newSprite) end(onSpriteChange, spot, newSprite)
    start(DefiniteFormeChange, onDefiniteFormeChange, int spot, int poke, int newPoke) end(onDefiniteFormeChange, spot, poke, newPoke)
    start(CosmeticFormeChange, onCosmeticFormeChange, int spot, int subforme) end(onCosmeticFormeChange, spot, subforme)
    start(ClockStart, onClockStart, int player, int time) end(onClockStart, player, time)
    start(ClockStop, onClockStop, int player, int time) end(onClockStop, player, time)
    start(ShiftSpots, onShiftSpots, int player, int spot1, int spot2, bool silent) end(onShiftSpots, player, spot1, spot2, silent)
    start(PPChange, onPPChange, int spot, int move, int PP) end(onPPChange, spot, move, PP)
    start(OfferChoice, onOfferChoice, int player, std::shared_ptr<BattleChoices>* choice) end (onOfferChoice, player, *choice->get())
    start(TempPPChange, onTempPPChange, int spot, int move, int PP) end (onTempPPChange, spot, move, PP)
    start(MoveChange, onMoveChange, int spot, int slot, int move, bool definite) end (onMoveChange, spot, slot, move, definite)
    start(RearrangeTeam, onRearrangeTeam, int player, std::shared_ptr<ShallowShownTeam>* team) end (onRearrangeTeam, player, *team->get())
    start(ChoiceSelection, onChoiceSelection, int player) end (onChoiceSelection, player)
    start(ChoiceCancelled, onChoiceCancellation, int player) end (onChoiceCancellation, player)
    start(Variation, onVariation, int player, int bonus, int malus) end (onVariation, player, bonus, malus)
    start(DynamicStats, onDynamicStats, int spot, std::shared_ptr<BattleStats>* stats) end (onDynamicStats, spot, *stats->get())
    start(PrintHtml, onPrintHtml, string_ptr data) end (onPrintHtml, *data->get())
    start(Reconnect, onReconnect, int player) end (onReconnect, player)
    start(Disconnect, onDisconnect, int player) end (onDisconnect, player)
    start(ChooseAttack, onAttackChosen, int spot, int attackSlot, int target) end(onAttackChosen, spot, attackSlot, target)
    start(ChooseSwitch, onSwitchChosen, int spot, int pokeSlot) end(onSwitchChosen, spot, pokeSlot)
    start(ChooseRearrangeTeam, onTeamOrderChosen, int player, std::shared_ptr<RearrangeChoice> *choice) end(onTeamOrderChosen, player, *choice->get())
    start(ChooseCancel, onChoiceCancelled, int player) end(onChoiceCancelled, player)
    start(ChooseShiftToCenter, onShiftToCenterChosen, int player) end(onShiftToCenterChosen, player)
    start(ChooseDraw, onDrawRequest, int player) end(onDrawRequest, player)
    start(UseItem, onUseItem, int player, int item) end(onUseItem, player, item)
    start(ItemCountChange, onItemChangeCount, int player, int item, int count) end (onItemChangeCount, player, item, count)

#undef start
#undef end

    template<enumClass val,typename...Params>
    void invoke2(Param<val>, Params&&...params) {
        wc()->template mInvoke<val, Params...>(std::forward<Params>(params)...);
    }
};

/* Functions to create in your class :
    void onKo(int spot);
    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool silent);
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
    void onPokeballStatusChanged(int player, int poke, int status);
    void onStatusAlreadyThere(int spot, int status);
    void onStatusNotification(int spot, int status);
    void onStatusDamage(int spot, int status);
    void onStatusOver(int spot, int status);
    void onAttackFailing(int spot, bool fail);
    void onPlayerMessage(int spot, const QString& message);
    void onSpectatorJoin(int id, const QString& name);
    void onSpectatorLeave(int id);
    void onSpectatorChat(int id, const QString& message);
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
    void onBlankMessage();
    void onClauseActivated(int clause);
    void onRatedNotification(bool rated);
    void onTierNotification(const QString &tier);
    void onDynamicInfo(int spot, const BattleDynamicInfo &info);
    void onPokemonVanish(int spot);
    void onPokemonReappear(int spot);
    void onSpriteChange(int spot, int newSprite);
    void onDefiniteFormeChange(int spot, int poke, int newPoke);
    void onCosmeticFormeChange(int spot, int subforme);
    void onClockStart(int player, int time);
    void onClockStop(int player, int time);
    void onShiftSpots(int player, int spot1, int spot2, bool silent);
    void onBattleEnd(int res, int winner);
    void onPPChange(int spot, int move, int PP);
    void onOfferChoice(int player, const BattleChoices &choice);
    void onTempPPChange(int spot, int move, int PP);
    void onMoveChange(int spot, int slot, int move, bool definite);
    void onRearrangeTeam(int player, const ShallowShownTeam& team);
    void onChoiceSelection(int player);
    void onChoiceCancellation(int player);
    void onVariation(int player, int bonus, int malus);
    void onDynamicStats(int spot, const BattleStats& stats);
    void onPrintHtml(const QString &html);
    void onReconnect(int player);
    void onDisconnect(int player);
    void onAttackChosen(int spot, int attackSlot, int target);
    void onSwitchChosen(int spot, int pokeSlot);
    void onTeamOrderChosen(int player, const RearrangeChoice &rearrange);
    void onChoiceCancelled(int player);
    void onShiftToCenterChosen(int player);
    void onDrawRequest(int player);
    void onUseItem(int player, int item);
    void onItemChangeCount(int player, int item, int count);
*/

#endif // BATTLECOMMANDINVOKER_H

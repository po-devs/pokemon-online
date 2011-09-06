#ifndef BATTLECOMMANDINVOKER_H
#define BATTLECOMMANDINVOKER_H

#include <cstdint>
#include <functional>
#include <memory>
#include "test.h"
#include "battleenum.h"

class ShallowBattlePoke;
class BattleDynamicInfo;

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
        wc()->onSendOut(spot, prevIndex, *ptr, silent);
    }

    /* Todo: expand following macros */
    start(SendBack, onSendBack, int spot) end(onSendBack, spot)
    start(UseAttack, onUseAttack, int spot, int attack) end(onUseAttack, spot, attack)
    start(Turn, onBeginTurn, int turn) end(onBeginTurn, turn)
    start(NewHp, onHpChange, int spot, int newHp) end(onHpChange, spot, newHp)
    start(Hits, onHitCount, int spot, int count) end(onHitCount, spot, count)
    start(Effectiveness, onEffectiveness, int spot, int effectiveness) end(onEffectiveness, spot, effectiveness)
    start(CriticalHit, onCriticalHit, int spot) end(onCriticalHit, spot)
    start(Miss, onMiss, int spot) end(onMiss, spot)
    start(Avoid, onAvoid, int spot) end(onAvoid, spot)
    start(StatChange, onStatBoost, int spot, int stat, int boost) end(onStatBoost, spot, stat, boost)
    start(ClassicStatusChange, onMajorStatusChange, int spot, int status) end(onMajorStatusChange, spot, status)
    start(AbsoluteStatusChange, onPokeballStatusChanged, int player, int poke, int status) end(onPokeballStatusChanged, player, poke, status)
    start(AlreadyStatusMessage, onStatusAlreadyThere, int spot, int status) end(onStatusAlreadyThere, spot, status)
    start(StatusFeel, onStatusNotification, int spot, int status) end(onStatusNotification, spot, status)
    start(StatusHurt, onStatusDamage, int spot, int status) end(onStatusDamage, spot, status)
    start(StatusFree, onStatusOver, int spot, int status) end(onStatusOver, spot, status)
    start(Fail, onAttackFailing, int spot) end(onAttackFailing, spot)
    start(PlayerMessage, onPlayerMessage, int spot, char* message) end(onPlayerMessage, spot, QString::fromUtf8(QByteArray(message)))
    start(SpectatorEnter, onSpectatorJoin, int id, char* name) end(onSpectatorJoin, id, QString::fromUtf8(QByteArray(name)))
    start(SpectatorLeave, onSpectatorLeave, int id) end(onSpectatorLeave, id)
    start(SpectatorMessage, onSpectatorChat, int id, char * message) end(onSpectatorChat, id, QString::fromUtf8(QByteArray(message)))
    start(MoveMessage, onMoveMessage, int spot, int move, int part, int type, int foe, int other, char *data)
        end(onMoveMessage, spot, move, part, type, foe, other, QString::fromUtf8(QByteArray(data)))
    start(NoTargetMessage, onNoTarget, int spot) end(onNoTarget, spot)
    start(ItemMessage, onItemMessage, int item, int part, int foe, int berry, int other) end(onItemMessage, item, part, foe, berry, other)
    start(Flinch, onFlinch, int spot) end(onFlinch, spot)
    start(Recoil, onRecoil, int spot) end(onRecoil, spot)
    start(Drained, onDrained, int spot) end(onDrained, spot)
    start(WeatherMessage, onContinueWeather, int weather) end(onContinueWeather, weather)
    start(EndWeather, onEndWeather, int weather) end(onEndWeather, weather)
    start(StartWeather, onStartWeather, int weather, bool ability) end(onStartWeather, weather, ability)
    start(WeatherDamage, onHurtWeather, int spot, int weather) end(onHurtWeather, spot, weather)
    start(Damaged, onDamageDone, int spot, int damage) end(onDamageDone, spot, damage)
    start(AbilityMessage, onAbilityMessage, int spot, int ab, int part, int type, int foe, int other)
        end(onAbilityMessage, spot, ab, part, type, foe, other)
    start(SubstituteStatus, onSubstituteStatus, int spot, bool substitute) end(onSubstituteStatus, spot, substitute)
    start(BlankMessage, onBlankMessage) end (onBlankMessage)
    start(BattleEnd, onBattleEnd, int res, int winner) end(onBattleEnd, res, winner)
    start(ClauseMessage, onCauseActivated, int clause) end(onCauseActivated, clause)
    start(RatedInfo, onRatedNotification, bool rated) end(onRatedNotification, rated)
    start(TierInfo, onTierNotification, QString tier) end(onTierNotification, tier)
    start(StatBoostsAndField, onDynamicInfo, int spot, BattleDynamicInfo *info) end(onDynamicInfo, spot, *info)
    start(PokemonVanish, onPokemonVanish, int spot) end(onPokemonVanish, spot)
    start(PokemonReappear, onPokemonReappear, int spot) end(onPokemonReappear, spot)
    start(SpriteChange, onSpriteChange, int spot, int newSprite) end(onSpriteChange, spot, newSprite)
    start(DefiniteFormeChange, onDefiniteFormeChange, int spot, int poke, int newPoke) end(onDefiniteFormeChange, spot, poke, newPoke)
    start(CosmeticFormeChange, onCosmeticFormeChange, int spot, int subforme) end(onCosmeticFormeChange, spot, subforme)
    start(ClockStart, onClockStart, int player, int time) end(onClockStart, player, time)
    start(ClockStop, onClockStop, int player, int time) end(onClockStop, player, time)
    start(ShiftSpots, onShiftSpots, int spot1, int spot2, bool silent) end(onShiftSpots, spot1, spot2, silent)

#undef start
#undef end

    template<enumClass val,typename...Params>
    void invoke2(Param<val>, Params&&...params) {
        wc()->template mInvoke<val, Params...>(std::forward<Params>(params)...);
    }
};

#endif // BATTLECOMMANDINVOKER_H

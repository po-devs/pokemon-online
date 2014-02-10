#ifndef BATTLEEXTRACTER_H
#define BATTLEEXTRACTER_H

#include <QHash>
#include <cstdarg>
#include <stdint.h>
#include <unordered_map>
#include <memory>
#include "battleenum.h"
#include <PokemonInfo/battlestructs.h>
/*
namespace std {
    template<class T> class shared_ptr;
}

class ShallowBattlePoke;
class BattleDynamicInfo;
class BattleChoices;
class ShallowShownTeam;
class BattleStats;
class RearrangeChoice;
*/

template <class Current>
class BattleExtracter
{
public:
    typedef Current workerClass;
    typedef BattleEnum enumClass;

    typedef void (BattleExtracter<Current>::*extrac_func)(va_list&);

    BattleExtracter();
    void entryPoint_v(enumClass, va_list&);

    template <enumClass val, typename ...Params>
    void forwardCommand(Params&&...params) {
        wc()->template receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    void forwardUnknownCommand(enumClass val, va_list &args) {
        wc()->unknownEntryPoint(val, args);
    }

    typedef std::shared_ptr<QString>* string_ptr;

protected:
    QHash<enumClass, extrac_func> callbacks;

    void extractArgument(va_list &args, bool &arg) {
        arg = va_arg(args, int);
    }

    template<class T>
    void extractArgument(va_list &args, T &arg) {
        arg = va_arg(args, T);
    }

    void extractArguments(va_list &args) {
        Q_UNUSED(args);
    }

    template<class T>
    void extractArguments(va_list &args, T&arg)
    {
        extractArgument(args, arg);
    }

    template <typename T, typename ...Params>
    void extractArguments(va_list &args, T &head, Params&... params) {
        extractArgument(args, head);
        extractArguments(args, params...);
    }

    void extractKo(va_list&);
    void extractSendOut(va_list&);
    void extractBlankMessage(va_list &args);
    void extractSpectatorEnter(va_list &args);

#define start(en, ...) \
    void extract##en(va_list &args) {\
        __VA_ARGS__; \
        const BattleEnum val = BattleEnum::en;

#define end(...) \
        extractArguments(args, ##__VA_ARGS__); \
        forwardCommand<val>(__VA_ARGS__); \
    }

    /* Todo: expand following macros */
    start(SendBack, int spot; bool silent) end(spot, silent)
    start(UseAttack, int spot; int attack; bool silent) end(spot, attack, silent)
    start(Turn, int turn) end(turn)
    start(NewHp, int spot; int newHp) end(spot, newHp)
    start(Hits, int spot; int count) end(spot, count)
    start(Effectiveness, int spot; int effectiveness) end(spot, effectiveness)
    start(CriticalHit, int spot) end(spot)
    start(Miss, int spot) end(spot)
    start(Avoid, int spot) end(spot)
    start(StatChange, int spot; int stat; int boost; bool silent) end(spot, stat, boost, silent)
    start(CappedStat, int spot; int stat; bool maxi) end(spot, stat, maxi)
    start(ClassicStatusChange, int spot; int status; bool multipleTurns; bool silent) end(spot, status, multipleTurns, silent)
    start(AbsoluteStatusChange, int player; int poke; int status) end(player, poke, status)
    start(AlreadyStatusMessage, int spot; int status) end(spot, status)
    start(StatusFeel, int spot; int status) end(spot, status)
    start(StatusHurt, int spot; int status) end(spot, status)
    start(StatusFree, int spot; int status) end(spot, status)
    start(Fail, int spot; bool silent) end(spot, silent)
    start(PlayerMessage, int spot; string_ptr message) end(spot, message)
    start(SpectatorLeave, int id) end(id)
    start(SpectatorMessage, int id; string_ptr message) end(id, message)
    start(MoveMessage, int spot; int move; int part; int type; int foe; int other; string_ptr data)
        end(spot, move, part, type, foe, other, data)
    start(NoTargetMessage, int spot) end(spot)
    start(ItemMessage, int spot; int item; int part; int foe; int berry; int other) end(spot, item, part, foe, berry, other)
    start(Flinch, int spot) end(spot)
    start(Recoil, int spot) end(spot)
    start(Drained, int spot) end(spot)
    start(WeatherMessage, int weather) end(weather)
    start(EndWeather, int weather) end(weather)
    start(StartWeather, int spot; int weather; bool ability) end(spot, weather, ability)
    start(WeatherDamage, int spot; int weather) end(spot, weather)
    start(Damaged, int spot; int damage) end(spot, damage)
    start(AbilityMessage, int spot; int ab; int part; int type; int foe; int other)
        end(spot, ab, part, type, foe, other)
    start(SubstituteStatus, int spot; bool substitute) end(spot, substitute)
    start(BattleEnd, int res; int winner) end(res, winner)
    start(ClauseMessage, int clause) end(clause)
    start(RatedInfo, bool rated) end(rated)
    start(TierInfo, string_ptr tier) end(tier)
    start(StatBoostsAndField, int spot; BattleDynamicInfo info) end(spot, info)
    start(PokemonVanish, int spot) end(spot)
    start(PokemonReappear, int spot) end(spot)
    start(SpriteChange, int spot; int newSprite) end(spot, newSprite)
    start(DefiniteFormeChange, int spot; int poke; int newPoke) end(spot, poke, newPoke)
    start(CosmeticFormeChange, int spot; int subforme) end(spot, subforme)
    start(ClockStart, int player; int time) end(player, time)
    start(ClockStop, int player; int time) end(player, time)
    start(ShiftSpots, int player; int slot1; int slot2; bool silent) end(player, slot1, slot2, silent)
    start(PPChange, int spot; int move; int PP) end(spot, move, PP)
    start(OfferChoice, int player; std::shared_ptr<BattleChoices>* choice) end (player, choice)
    start(TempPPChange, int spot; int move; int PP) end (spot, move, PP)
    start(MoveChange, int spot; int slot; int move; bool definite) end (spot, slot, move, definite)
    start(RearrangeTeam, int player; std::shared_ptr<ShallowShownTeam>* team) end (player, team)
    start(ChoiceSelection, int player) end (player)
    start(ChoiceCancelled, int player) end (player)
    start(Variation, int player; int bonus; int malus) end (player, bonus, malus)
    start(DynamicStats, int spot; std::shared_ptr<BattleStats>* stats) end (spot, stats)
    start(PrintHtml, string_ptr data) end(data)
    start(Reconnect, int player) end (player)
    start(Disconnect, int player) end (player)
    start(ChooseAttack, int spot; int attackSlot; int target) end(spot, attackSlot, target)
    start(ChooseSwitch, int spot; int pokeSlot) end(spot, pokeSlot)
    start(ChooseRearrangeTeam, int player; std::shared_ptr<RearrangeChoice> *choice) end(player, choice)
    start(ChooseCancel, int player) end(player)
    start(ChooseShiftToCenter, int player) end(player)
    start(ChooseDraw, int player) end(player)
    start(UseItem, int player; int item) end(player, item)
    start(ItemCountChange, int player; int item; int count) end(player, item, count)

#undef start
#undef end

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};

template <class C>
void BattleExtracter<C>::extractKo(va_list &args)
{
    uint8_t spot = va_arg(args, int);

    forwardCommand<BattleEnum::Ko>(spot);
}

template <class C>
void BattleExtracter<C>::extractBlankMessage(va_list&)
{
    forwardCommand<BattleEnum::BlankMessage>();
}

template <class C>
void BattleExtracter<C>::extractSendOut(va_list &args)
{
    uint8_t spot = va_arg(args, int);
    uint8_t prevIndex = va_arg(args, int);
    std::shared_ptr<ShallowBattlePoke> *poke = va_arg(args, std::shared_ptr<ShallowBattlePoke> *);
    bool silent = va_arg(args, int);

    forwardCommand<BattleEnum::SendOut>(spot, prevIndex, poke, silent);
}

template <class C>
void BattleExtracter<C>::extractSpectatorEnter(va_list &args)
{
    int id = va_arg(args, int);
    string_ptr name = va_arg(args, string_ptr);

    forwardCommand<BattleEnum::SpectatorEnter>(id, name);
}

template<class C>
BattleExtracter<C>::BattleExtracter()
{
#define addCallback(en) \
    callbacks.insert(BattleEnum::en, &BattleExtracter<workerClass>::extract##en)

    addCallback(NewHp);
    addCallback(Damaged);
    addCallback(Ko);
    addCallback(SendOut);
    addCallback(SendBack);
    addCallback(UseAttack);
    addCallback(Turn);
    addCallback(Hits);
    addCallback(Effectiveness);
    addCallback(CriticalHit);
    addCallback(Miss);
    addCallback(Avoid);
    addCallback(StatChange);
    addCallback(CappedStat);
    addCallback(ClassicStatusChange);
    addCallback(AbsoluteStatusChange);
    addCallback(AlreadyStatusMessage);
    addCallback(StatusFeel);
    addCallback(StatusFree);
    addCallback(StatusHurt);
    addCallback(Fail);
    addCallback(PlayerMessage);
    addCallback(SpectatorEnter);
    addCallback(SpectatorLeave);
    addCallback(SpectatorMessage);
    addCallback(MoveMessage);
    addCallback(NoTargetMessage);
    addCallback(ItemMessage);
    addCallback(Flinch);
    addCallback(Recoil);
    addCallback(Drained);
    addCallback(StartWeather);
    addCallback(WeatherMessage);
    addCallback(EndWeather);
    addCallback(WeatherDamage);
    addCallback(AbilityMessage);
    addCallback(SubstituteStatus);
    addCallback(BattleEnd);
    addCallback(BlankMessage);
    addCallback(ClauseMessage);
    addCallback(RatedInfo);
    addCallback(TierInfo);
    addCallback(StatBoostsAndField);
    addCallback(PokemonVanish);
    addCallback(PokemonReappear);
    addCallback(SpriteChange);
    addCallback(DefiniteFormeChange);
    addCallback(CosmeticFormeChange);
    addCallback(ClockStart);
    addCallback(ClockStop);
    addCallback(ShiftSpots);
    addCallback(PPChange);
    addCallback(OfferChoice);
    addCallback(TempPPChange);
    addCallback(MoveChange);
    addCallback(RearrangeTeam);
    addCallback(ChoiceSelection);
    addCallback(ChoiceCancelled);
    addCallback(Variation);
    addCallback(DynamicStats);
    addCallback(PrintHtml);
    addCallback(Reconnect);
    addCallback(Disconnect);
    addCallback(ChooseAttack);
    addCallback(ChooseSwitch);
    addCallback(ChooseShiftToCenter);
    addCallback(ChooseRearrangeTeam);
    addCallback(ChooseCancel);
    addCallback(ChooseDraw);
    addCallback(UseItem);
    addCallback(ItemCountChange);

#undef addCallback
}

template <class C>
void BattleExtracter<C>::entryPoint_v(enumClass val, va_list &args)
{
    if (callbacks.find(val) == callbacks.end()) {
        forwardUnknownCommand(val, args);
        return;
    }
    (this->*callbacks[val])(args);
}


#endif // BATTLEEXTRACTER_H

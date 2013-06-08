#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>
#include "battle.h"

#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "battlefunctions.h"
#include "battlecounterindex.h"
#include "../Shared/battlecommands.h"

using namespace BattleCommands;

typedef BattlePStorage BP;
typedef BattleSituation::TurnMemory TM;

Q_DECLARE_METATYPE(QList<int>)

BattleSituation::BattleSituation(Player &p1, Player &p2, const ChallengeInfo &c, int id, int nteam1, int nteam2, PluginManager *pluginManager)
{
    init(p1, p2, c, id, nteam1, nteam2, pluginManager);

    for (int i = 0; i < numberOfSlots(); i++) {
        slotzone.push_back(context());
        contexts.push_back(PokeContext());
        indexes.push_back(i);
    }
}

BattleSituation::~BattleSituation()
{
    onDestroy();
}

void BattleSituation::engageBattle()
{
    BattleBase::engageBattle();

    /* For example, if two pokemons are brought out
       with a weather ability, the slower one acts last */
    std::vector<int> pokes = sortedBySpeed();

    foreach(int p, pokes)
        callEntryEffects(p);
}

void BattleSituation::initializeEndTurnFunctions()
{
    /* Gen 2:
       1 Future Sight (non-leader first)
       2 Sandstorm/Sunny Day/Rain Dance
       3 Multi-turn attacks
       4 Perish Song
       5 Recover with Leftovers
       6 Check if defrosted
       7.0 Check end of Reflect/Light Screen
       7.2 Safeguard
       7.3 Mist
       8 Check for condition based on Berry (for both players)
       9 Check end of Encore
       10 Switch-ins
    */
    if (gen() <= 2) {
        ownEndFunctions.push_back(QPair<int, VoidFunction>(2, &BattleSituation::endTurnWeather));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(6, &BattleSituation::endTurnDefrost));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(10, &BattleSituation::requestEndOfTurnSwitchIns));

        addEndTurnEffect(ItemEffect, 5, 0); /* Black Sludge, Leftovers */
    }
    /* Gen 4:
    1.0 Reflect wears off: "your team's reflect wore off"
    1.1 Light Screen wears off: "your team's light screen wore off"
    1.2 Mist wears off: "your team's mist wore off"
    1.3 Safeguard fades: "your team is no longer protected by safeguard"
    1.4 Tailwind ends: "your team's tailwind petered out"
    1.5 Lucky Chant: your team's lucky chant wore off"

    2.0 Wish: "pokemon's wish came true"

    3.0 Hail, Rain, Sandstorm, or Sun message

    4.0 Dry Skin, Hydration, Ice Body, Rain Dish

    5.0 Gravity

    6.0 Ingrain
    6.1 Aqua Ring
    6.2 Speed Boost, Shed Skin
    6.3 Black Sludge, Leftovers: "pokémon restored a little HP using its leftovers"
    6.4 Leech Seed: "pokémon's health is sapped by leech seed"
    6.5 Burn, Poison Heal, Poison: "pokémon is hurt by poison"
    6.6 Nightmare
    6.7 Flame Orb activation, Toxic Orb activation
    6.8 Curse (from a Ghost)
    6.9 Bind, Clamp, Fire Spin, Magma Storm, Sand Tomb, Whirlpool, Wrap
    6.10 Bad Dreams Damage
    6.11 End of Outrage, Petal Dance, Thrash, Uproar: "pokémon caused an uproar" & "pokémon calmed down"
    6.12 Disable ends: "pokémon is no longer disabled"
    6.13 Encore ends
    6.14 Taunt wears off
    6.15 Magnet Rise
    6.16 Heal Block: "the foe pokémon's heal block wore off"
    6.17 Embargo
    6.18 Yawn
    6.19 Sticky Barb

    7.0 Doom Desire, Future Sight

    8.0 Perish Song

    9.0 Trick Room

    10.0 Pokemon is switched in (if previous Pokemon fainted)
    10.1 Toxic Spikes
    10.2 Spikes
    10.3 Stealth Rock

    11.0 Healing Heart
    12.0 Slow start
    */
    else if (gen() <= 4) {
        ownEndFunctions.push_back(QPair<int, VoidFunction>(3, &BattleSituation::endTurnWeather));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(10, &BattleSituation::requestEndOfTurnSwitchIns));

        addEndTurnEffect(AbilityEffect, 6, 2); /* Shed Skin, Speed Boost */
        addEndTurnEffect(ItemEffect, 6, 3); /* Black Sludge, Leftovers */

        addEndTurnEffect(OwnEffect, 6, 5, 0, "", NULL, &BattleSituation::endTurnStatus);

        addEndTurnEffect(ItemEffect, 6, 7); /* Orbs */
        addEndTurnEffect(AbilityEffect, 6, 10); /* Bad Dreams */
        addEndTurnEffect(ItemEffect, 6, 19); /* Sticky Barb */


        addEndTurnEffect(AbilityEffect, 12, 0); /* Slow Start */
    } else {
        /* 1.0 weather ends

        2.0 Sandstorm damage, Hail damage, Rain Dish, Dry Skin, Ice Body

        3.0 Future Sight, Doom Desire

        4.0 Wish

        5.0 Fire Pledge + Grass Pledge damage
        5.1 Shed Skin, Hydration, Healer
        5.2 Leftovers, Black Sludge

        6.0 Aqua Ring

        7.0 Ingrain

        8.0 Leech Seed

        9.0 (bad) poison damage, burn damage, Poison Heal
        9.1 Nightmare

        10.0 Curse (from a Ghost-type)

        11.0 Bind, Wrap, Fire Spin, Clamp, Whirlpool, Sand Tomb, Magma Storm

        12.0 Taunt ends

        13.0 Encore ends

        14.0 Disable ends, Cursed Body ends

        15.0 Magnet Rise ends

        16.0 Telekinesis ends

        17.0 Heal Block ends

        18.0 Embargo ends

        19.0 Yawn

        20.0 Perish Song

        21.0 Reflect ends
        21.1 Light Screen ends
        21.2 Safeguard ends
        21.3 Mist ends
        21.4 Tailwind ends
        21.5 Lucky Chant ends
        21.6 Water Pledge + Fire Pledge ends, Fire Pledge + Grass Pledge ends, Grass Pledge + Water Pledge ends

        22.0 Gravity ends

        23.0 Trick Room ends

        24.0 Wonder Room ends

        25.0 Magic Room ends

        26.0 Uproar message
        26.1 Speed Boost, Bad Dreams, Harvest, Moody
        26.2 Toxic Orb activation, Flame Orb activation, Sticky Barb
        26.3 pickup

        27.0 Zen Mode

        28.0 Pokémon is switched in (if previous Pokémon fainted)
        28.1 Healing Wish, Lunar Dance
        28.2 Spikes, Toxic Spikes, Stealth Rock (hurt in the order they are first used)

        29.0 Slow Start
        */
        ownEndFunctions.push_back(QPair<int, VoidFunction>(1, &BattleSituation::endTurnWeather));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(28, &BattleSituation::requestEndOfTurnSwitchIns));

        addEndTurnEffect(AbilityEffect, 5, 1); /* Shed skin, Hydration, Healer */
        addEndTurnEffect(ItemEffect, 5, 2); /* Leftovers, Black sludge */

        addEndTurnEffect(OwnEffect, 9, 0, 0, "", NULL, &BattleSituation::endTurnStatus);

        addEndTurnEffect(AbilityEffect, 26, 1); /* Speed Boost, Bad Dreams, Harvest, Pickup Moody */
        addEndTurnEffect(ItemEffect, 26, 2); /* Orbs, sticky barb */

        addEndTurnEffect(AbilityEffect, 27, 0); /* Daruma Mode */

        addEndTurnEffect(AbilityEffect, 29, 0); /* Slow Start */
    }
}

void BattleSituation::getVectorRef(priorityBracket b)
{
    int i;
    for (i = 0; i < endTurnEffects.size(); i++) {
        if (b <= endTurnEffects[i]) {
            break;
        }
    }

    if (i == endTurnEffects.size() || b < endTurnEffects[i]) {
        endTurnEffects.insert(i, b);
        bracketToEffect[b] = QString("EndTurn%1.%2").arg(b.bracket).arg(b.priority);
    }
}

void BattleSituation::addEndTurnEffect(EffectType type, priorityBracket bracket, int slot,
                                       const QString &function, MechanicsFunction f, IntFunction f2)
{
    addEndTurnEffect(type, bracket.bracket, bracket.priority, slot, function, f, f2);
}

void BattleSituation::addEndTurnEffect(EffectType type, int bracket, int priority, int slot,
                                       const QString &function, MechanicsFunction f, IntFunction f2)
{
    priorityBracket b(bracket, priority);

    getVectorRef(b);
    bracketType[b] = type;

    QString effect = bracketToEffect[b];

    if (f && !effectToBracket.contains(function)) {
        effectToBracket[function] = b;
    }

    switch(type) {
    case PokeEffect:
        Mechanics::addFunction(pokeMemory(slot), effect, function, f);
        break;
    case SlotEffect:
        Mechanics::addFunction(slotMemory(slot), effect, function, f);
        break;
    case TurnEffect:
        Mechanics::addFunction(turnMemory(slot), effect, function, f);
        break;
    case ZoneEffect:
        Mechanics::addFunction(teamMemory(slot), effect, function, f);
        break;
    case FieldEffect:
        Mechanics::addFunction(battleMemory(), effect, function, f);
        break;
    case AbilityEffect:
    case ItemEffect:
        break;
    case OwnEffect:
        ownSEndFunctions[b] = f2;
        break;
    };

    bracketCount[b] += 1;
}

/*
  Note: in the end turn loop, when a pokemon is koed,
  sometimes this function doesn't get called and so trailing
  effects are kept in the vector. Not such a big deal but meh.

  Also if several effects share the same bracket, only one will have its
  function removed
  */
void BattleSituation::removeEndTurnEffect(EffectType type, int slot, const QString &function)
{
    priorityBracket b = effectToBracket[function];
    QString effect = bracketToEffect[b];

    switch(type) {
    case PokeEffect:
        Mechanics::removeFunction(pokeMemory(slot), effect, function);
        break;
    case SlotEffect:
        Mechanics::removeFunction(slotMemory(slot), effect, function);
        break;
    case TurnEffect:
        Mechanics::removeFunction(turnMemory(slot), effect, function);
        break;
    case ZoneEffect:
        Mechanics::removeFunction(teamMemory(slot), effect, function);
        break;
    case FieldEffect:
        Mechanics::removeFunction(battleMemory(), effect, function);
        break;
    case AbilityEffect:
    case ItemEffect:
    case OwnEffect:
        /* Error ?!! */
        break;
    };

    bracketCount[b] -= 1;

    if (bracketCount[b] <= 0) {
        effectToBracket.remove(function);
    }
}

void BattleSituation::endTurn()
{
    testWin();

    /* Gen3 switches pokemons in before endturn effects */
    if (gen() <= 3)
        requestSwitchIns();

    speedsVector = sortedBySpeed();

    /* Counters */
    if (gen() < 5) {
        for(unsigned i = 0; i < speedsVector.size(); i++) {
            counters(speedsVector[i]).decreaseCounters();
        }
    }

    int i, ownBracket(0);

    for (i = 0; i < endTurnEffects.size(); i++) {
        /* First the functions that deal with the whole turn */
        while (ownBracket < ownEndFunctions.size() && ownEndFunctions[ownBracket].first <= endTurnEffects[i].bracket) {
            VoidFunction f = ownEndFunctions[ownBracket].second;
            (this->*f)();
            ownBracket++;

            testWin();
        }

        priorityBracket b = endTurnEffects[i];

        if (b.priority == 0) {
            int flags = bracketType[b];

            if (flags == FieldEffect) {
                QString effect = bracketToEffect[b];
                callbeffects(Player1, Player1, effect);
                continue;
            }
        }

        int beginning = i;
        for (i = beginning; i < endTurnEffects.size() && endTurnEffects[i].bracket == b.bracket; i++) {

        }
        i -= 1;

        quint32 side1(0), side2(0);
        bool fullLoop[2] = {false, false};

        for(int z = 0; z < int(speedsVector.size()); z++) {
            int player = speedsVector[z];

            if (koed(player)) {
                continue;
            }
            for (int j = beginning; j <= i; j++) {
                priorityBracket b = endTurnEffects[j];
                QString effect = bracketToEffect[b];
                int flags = bracketType[b];

                /* TODO: make a vector of function pointers with those in,
                  allowing to automate the calling process instead of doing
                  plenty of if */
                if (flags == ItemEffect) {
                    callieffects(player, player, effect);
                } else if (flags == AbilityEffect) {
                    callaeffects(player, player, effect);
                } else if (flags == SlotEffect) {
                    callseffects(player, player, effect);
                } else if (flags == PokeEffect) {
                    /* Otherwise called in personal end turn */
                    if (!(b.bracket == 6 && gen() < 3)) {
                        callpeffects(player, player, effect);
                    }
                } else if (flags == TurnEffect) {
                    calleffects(player, player, effect);
                } else if (flags == OwnEffect) {
                    IntFunction f = ownSEndFunctions[b];
                    (this->*f)(player);
                } else if (flags == ZoneEffect) {
                    int p = this->player(player);
                    quint32 mask = 1 << b.priority;

                    if (p == Player1 && !(side1&mask)) {
                        side1 |= mask;
                        callzeffects(p, p, effect);
                    } else if (p == Player2 && !(side2&mask)) {
                        side2 |= mask;
                        callzeffects(p, p, effect);
                    }
                }

                if (koed(player)) {
                    speedsVector.erase(speedsVector.begin()+z, speedsVector.begin()+z+1);
                    z -= 1;
                    break;
                }
            }
            int p = this->player(player);
            if (p >= 0 && p < int(sizeof(fullLoop)/sizeof(*fullLoop))) {
                fullLoop[p] = true;
            }
        }

        for (int p = Player1; p <= Player2; p++) {
            if (!fullLoop[p]) {
                for (int j = beginning; j <= i; j++) {
                    priorityBracket b = endTurnEffects[j];

                    int flags = bracketType[b];
                    if (flags == ZoneEffect) {
                        QString effect = bracketToEffect[b];

                        callzeffects(p, p, effect);
                    }
                }
            }
        }

        testWin();
    }
    while (ownBracket < ownEndFunctions.size()) {
        VoidFunction f = ownEndFunctions[ownBracket].second;
        (this->*f)();
        ownBracket++;

        testWin();
    }

    /* Remove all useless end turn brackets */
    for (int i = endTurnEffects.size()-1; i >= 0; i--) {
        priorityBracket b = endTurnEffects[i];

        if (bracketCount[b] <= 0) {
            bracketCount.remove(b);
            bracketToEffect.remove(b);
            bracketType.remove(b);
            endTurnEffects.remove(i);
        }
    }
}

void BattleSituation::endTurnDefrost()
{
    // RBY freeze is forever unless hit by fire moves.
    // We think both stadium and cart have permafreeze.
    if (gen().num == 1) {
        return;
    }
    foreach(int player, speedsVector) {
        if (poke(player).status() == Pokemon::Frozen)
        {
            if (coinflip(26, 255))
            {
                unthaw(player);
            }
        }
    }
}

void BattleSituation::requestEndOfTurnSwitchIns()
{
    requestSwitchIns();

    /* Now, in triples, if pokemon are on far ends they're
       put back to the center */
    if (mode() == ChallengeInfo::Triples) {
        if (countAlive(Player1) == 1 && countAlive(Player2) == 1) {
            int p1(0);
            for (int i = 0; i < numberOfSlots(); i++) {
                if (!koed(i)) {
                    p1 = i;
                    break;
                }
            }
            int p2(0);
            for (int i = p1+1; i < numberOfSlots(); i++) {
                if (!koed(i)) {
                    p2 = i;
                    break;
                }
            }

            if (!areAdjacent(p1, p2)) {
                shiftSpots(p1, slot(player(p1), 1));
                shiftSpots(p2, slot(player(p2), 1));
            }
        }
    }

    speedsVector = sortedBySpeed();
}

void BattleSituation::personalEndTurn(int player)
{
    if (koed(player))
        return;
    endTurnStatus(player);
    callpeffects(player, player, "EndTurn6.4");//leech seed
    callpeffects(player, player, "EndTurn6.6");//nightmare
    callpeffects(player, player, "EndTurn6.8");//curse
    callpeffects(player, player, "EndTurn6.9");//bind

    testWin();
}

void BattleSituation::endTurnStatus(int player)
{
    if (koed(player))
        return;

    switch(poke(player).status())
    {
    case Pokemon::Burnt:
        if (hasWorkingAbility(player, Ability::MagicGuard))
            return;

        notify(All, StatusMessage, player, qint8(HurtBurn));
        //HeatProof: burn does only 1/16, also Gen 1 only does 1/16
        inflictDamage(player, poke(player).totalLifePoints()/(8*(1+(hasWorkingAbility(player,Ability::Heatproof) || gen().num == 1))), player);
        break;
    case Pokemon::Poisoned:
        //PoisonHeal
        if (hasWorkingAbility(player, Ability::PoisonHeal)) {
            sendAbMessage(45,0,player,0,Pokemon::Poison);
            healLife(player, poke(player).totalLifePoints()/8);
        } else {
            if (hasWorkingAbility(player, Ability::MagicGuard)) {
                /* Toxic still increases under magic guard */
                if (poke(player).statusCount() != 0)
                    poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
                return;
            }
            notify(All, StatusMessage, player, qint8(HurtPoison));

            if (poke(player).statusCount() == 0)
                inflictDamage(player, poke(player).totalLifePoints()/ (gen().num == 1 ? 16 : 8), player); // 1/16 in gen 1
            else {
                inflictDamage(player, poke(player).totalLifePoints() * (15-poke(player).statusCount()) / 16, player);
                poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
            }
        }
        break;
    }
}

BattleChoices BattleSituation::createChoice(int slot)
{
    /* First let's see for attacks... */
    if (koed(slot)) {
        return BattleChoices::SwitchOnly(slot);
    }

    BattleChoices ret;
    ret.numSlot = slot;

    /* attacks ok, lets see which ones then */
    callpeffects(slot, slot, "MovesPossible");
    callieffects(slot, slot, "MovesPossible");
    callbeffects(slot, slot,"MovesPossible");

    for (int i = 0; i < 4; i++) {
        if (!isMovePossible(slot,i)) {
            ret.attackAllowed[i] = false;
        }
    }

    if (!hasWorkingItem(slot, Item::ShedShell)) {
        /* Shed Shell */
        if (linked(slot, "Blocked") || linked(slot, "Trapped")) {
            ret.switchAllowed = false;
        }

        if (pokeMemory(slot).contains("Rooted")) {
            ret.switchAllowed = false;
        }

        QList<int> opps = revs(slot);
        foreach(int opp, opps){
            callaeffects(opp, slot, "IsItTrapped");
            if (turnMemory(slot).value("Trapped").toBool()) {
                ret.switchAllowed = false;
                break;
            }
        }
    }

    if (linked(slot, "FreeFalled")) {
        ret.switchAllowed = false;
    }

    return ret;
}

bool BattleSituation::isMovePossible(int player, int move)
{
    return (BattleBase::isMovePossible(player, move) && turnMemory(player).value("Move" + QString::number(move) + "Blocked").toBool() == false);
}

void BattleSituation::analyzeChoice(int slot)
{
    int player = this->player(slot);

    attackCount() += 1;
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice(slot).attackingChoice()) {
        turnMemory(slot)["Target"] = choice(slot).target();
        if (!wasKoed(slot)) {
            if (turnMem(slot).contains(TM::NoChoice))
                if (turnMemory(slot).contains("AutomaticMove")) {
                    useAttack(slot, turnMemory(slot)["AutomaticMove"].toInt(), true);
                } else {
                    /* Automatic move */
                    useAttack(slot, fpoke(slot).lastMoveUsed, true);
                }
            else {
                if (options[slot].struggle()) {
                    MoveEffect::setup(Move::Struggle,slot,0,*this);
                    useAttack(slot, Move::Struggle, true);
                } else {
                    useAttack(slot, choice(slot).attackSlot());
                }
            }
        }
    } else if (choice(slot).switchChoice()){
        bool wasKoed = koed(slot);
        if (!koed(slot)) /* if the pokemon isn't ko, it IS sent back */
            sendBack(slot);

        /* In gen 4 & previous, pursuit doesn't allow you to choose a new pokemon.
           In 5th gen, it's like a normal KO */
        if (gen() <= 4 || !koed(slot) || wasKoed) {
            sendPoke(slot, choice(slot).pokeSlot());
        } else {
            /* Entry effects are already called by analyzeChoices */
            requestSwitch(slot, false);
        }
    } else if (choice(slot).moveToCenterChoice()) {
        if (!wasKoed(slot)) {
            int target = this->slot(player, 1);

            shiftSpots(slot, target);
        }
    } else if (choice(slot).itemChoice()) {
        useItem(slot, choice(slot).item(), choice(slot).itemTarget(), choice(slot).itemAttack());
    } else {
        /* FATAL FATAL */
    }

    if (gen() >= 3) {
        notify(All, BlankMessage, Player1);
    } else {

    }
}

void BattleSituation::shiftSpots(int spot1, int spot2, bool silent)
{
    if (player(spot1) != player(spot2))
        return; //Avoid doing anything risky

    int sl1 = slotNum(spot1);
    int sl2 = slotNum(spot2);
    int p = player(spot1);

    notify(All, SpotShifting, p, qint8(sl1), qint8(sl2), silent);

    for (unsigned i = 0; i < speedsVector.size(); i++) {
        if (speedsVector[i] == spot1) {
            speedsVector[i] = spot2;
        } else if (speedsVector[i] == spot2) {
            speedsVector[i] = spot1;
        }
    }

    team(p).switchPokemon(sl1, sl2);

    std::swap(indexes[spot1], indexes[spot2]);
    std::swap(slotMemory(spot1)["SwitchCount"], slotMemory(spot2)["SwitchCount"]);

    if (attacking()) {
        if (attacked() == spot2) {
            attacked() = spot1;
        }
        attacker() = spot2;
    }
}

std::vector<int> BattleSituation::sortedBySpeed() {
    std::vector<int> ret = BattleBase::sortedBySpeed();

    if (battleMemory().value("TrickRoomCount").toInt() > 0) {
        std::reverse(ret.begin(),ret.end());
        if (gen().num == 5) { // gen 5 ignores trick room for pokemon with speed>=1809
            std::vector<int> temp;
            for (int it = ret.size()-1; it >= 0; it--) {
                if (getStat(ret[it], Speed) >= 1809) {
                    temp.push_back(ret[it]);
                    ret.erase(ret.begin()+it);
                }
            }
            ret.insert(ret.begin(), temp.begin(), temp.end());
        }
    }

    return std::move(ret);
}

void BattleSituation::analyzeChoices()
{
    setupChoices();

    std::map<int, std::vector<int>, std::greater<int> > priorities;
    std::vector<int> items;
    std::vector<int> switches;

    std::vector<int> playersByOrder = sortedBySpeed();

    foreach(int i, playersByOrder) {
        if (choice(i).itemChoice()) {
            items.push_back(i);
        } else if (choice(i).switchChoice()) {
            switches.push_back(i);
        } else if (choice(i).attackingChoice()){
            if (gen() >= 5) {
                callaeffects(i, i, "PriorityChoice");
            }
            priorities[tmove(i).priority].push_back(i);
        } else {
            /* Shifting choice */
            priorities[0].push_back(i);
        }
    }

    foreach(int player, items) {
        analyzeChoice(player);
    }

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;
    std::vector<int> &players = speedsVector;
    players.clear();

    for (it = priorities.begin(); it != priorities.end(); ++it) {
        /* There's another priority system: Ability stall, and Item lagging tail */
        std::map<int, std::vector<int>, std::greater<int> > secondPriorities;

        foreach (int player, it->second) {
            callaeffects(player,player, "TurnOrder"); //Stall
            callieffects(player,player, "TurnOrder"); //Lagging tail & ...
            secondPriorities[turnMemory(player)["TurnOrder"].toInt()].push_back(player);
        }

        for(std::map<int, std::vector<int> >::iterator it = secondPriorities.begin(); it != secondPriorities.end(); ++it) {
            foreach(int p, it->second) {
                players.push_back(p);
            }
        }
    }

    /* In case of pursuit + analyze, the speeds vector need to be set first, which is why
     * this code is below */
    foreach(int player, switches) {
        analyzeChoice(player);
        callEntryEffects(player);
        if (gen() <= 2) {
            personalEndTurn(player);
            notify(All, BlankMessage, Player1);
        }
    }

    /* The loop is separated, cuz all TurnOrders must be called at the beggining of the turn,
       cf custap berry */
    if (gen() >= 4) {
        for(currentSlot = 0; currentSlot < players.size(); currentSlot += 1) {
            int p = players[currentSlot];
            if (!hasMoved(p))
                analyzeChoice(p);
            testWin();
            selfKoer() = -1;
        }
    } else { // gen <= 3
        for(unsigned i = 0; i < players.size(); i++) {
            if (!multiples()) {
                if (koed(0) || koed(1))
                    break;
            } else {
                requestSwitchIns();
            }

            if (!hasMoved(players[i])) {
                analyzeChoice(players[i]);

                if (!multiples() && (koed(0) || koed(1))) {
                    testWin();
                    selfKoer() = -1;
                    break;
                }

                if (gen() <= 2) {
                    personalEndTurn(players[i]);
                    notify(All, BlankMessage, Player1);
                }
            }
            testWin();
            selfKoer() = -1;
        }
    }
}

/* Battle functions! Yeah! */

void BattleSituation::sendPoke(int slot, int pok, bool silent)
{
    int player = this->player(slot);
    int snum = slotNum(slot);

    if (poke(player,pok).num() == Pokemon::Giratina_O && poke(player,pok).item() != Item::GriseousOrb)
        changeForme(player,pok,Pokemon::Giratina);

    /* reset temporary variables */
    pokeMemory(slot).clear();

    /* Reset counters */
    counters(slot).clear();

    if (poke(player, pok).ability() == Ability::Illusion) {
        for (int i = 5; i >= 0; i--) {
            if (poke(player, i).num() != 0 && !poke(player, i).ko()) {
                if (i == pok)
                    break;
                pokeMemory(slot)["IllusionTarget"] = team(player).internalId(team(player).poke(i));
                break;
            }
        }
    }

    notify(All, SendOut, slot, silent, quint8(pok), opoke(slot, player, pok));

    team(player).switchPokemon(snum, pok);

    PokeBattle &p = poke(slot);

    //Clears secondary statuses
    int st = p.status();
    p.fullStatus() = 0;
    p.changeStatus(st);

    /* Give new values to what needed */
    fpoke(slot).init(p, gen());

    if (p.statusCount() > 0) {
        if (p.status() == Pokemon::Poisoned)
            p.statusCount() = gen() <= 2 ? 0 : 14;
        else if (p.status() != Pokemon::Asleep)
            p.statusCount() = 0;
    }
    if (p.status() == Pokemon::Asleep && gen() >= 5) {
        p.statusCount() = p.oriStatusCount();
    }

    /* Increase the "switch count". Switch count is used to see if the pokemon has switched
       (like for an attack like attract), it is imo more effective that other means */
    inc(slotMemory(slot)["SwitchCount"], 1);

    if (p.num() == Pokemon::Arceus && ItemInfo::isPlate(p.item())) {
        int type = ItemInfo::PlateType(p.item());

        if (type != Type::Normal) {
            changeAForme(slot, type);
        }
    }
    if (p.num() == Pokemon::Genesect && ItemInfo::isDrive(p.item())) {
        int forme = ItemInfo::DriveForme(p.item());

        if (forme != 0) {
            changeAForme(slot, forme);
        }
    }

    turnMem(slot).add(TurnMemory::Incapacitated);

    if (gen() >= 2)
        ItemEffect::setup(poke(slot).item(), slot, *this);

    calleffects(slot, slot, "UponSwitchIn");
    callseffects(slot, slot, "UponSwitchIn");
    callzeffects(player, slot, "UponSwitchIn");
}

void BattleSituation::callEntryEffects(int player)
{
    if (!koed(player)) {
        /* Various problems with item setting up are:
           - WhiteHerb restoring baton pass stats
           - Chesto Berry not having an argument and curing Toxic Spikes
           - Choice Scarf not activating right at weather starting

           So All those must be taken in account when changing something to
           how the items are set up. */
        callieffects(player, player, "UponSetup");
        if (gen() >= 3)
            acquireAbility(player, poke(player).ability(), true);
        calleffects(player, player, "AfterSwitchIn");
    }
}

void BattleSituation::calleffects(int source, int target, const QString &name)
{
    if (!isOut(source)) {
        return;
    }
    context &turn = turnMemory(source);
    if (turn.contains("Effect_" + name)) {
        turn["TurnEffectCall"] = true;
        turn["TurnEffectCalled"] = name;
        QSet<QString> &effects = *turn.value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = turn.value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            if (f)
                f(source, target, *this);
        }
        turn["TurnEffectCall"] = false;
    }
}

void BattleSituation::callpeffects(int source, int target, const QString &name)
{
    if (pokeMemory(source).contains("Effect_" + name)) {
        turnMemory(source)["PokeEffectCall"] = true;
        QSet<QString> &effects = *pokeMemory(source).value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = pokeMemory(source).value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            /* If a pokemons dies from leechseed,its status changes, and so nightmare function would be removed
               but still be in the foreach, causing a crash */
            if(f)
                f(source, target, *this);
        }
        turnMemory(source)["PokeEffectCall"] = false;
    }
}

void BattleSituation::callbeffects(int source, int target, const QString &name, bool stopOnFail)
{
    if (battleMemory().contains("Effect_" + name)) {
        QSet<QString> &effects = *battleMemory().value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = battleMemory().value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            if(f) f(source, target, *this);

            if(stopOnFail && testFail(source)) return;
        }
    }
}

void BattleSituation::callzeffects(int source, int target, const QString &name)
{
    if (teamMemory(source).contains("Effect_" + name)) {
        QSet<QString> &effects = *teamMemory(source).value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = teamMemory(source).value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            if (f)
                f(source, target, *this);
        }
    }
}

void BattleSituation::callseffects(int source, int target, const QString &name)
{
    if (slotMemory(source).contains("Effect_" + name)) {
        QSet<QString> &effects = *slotMemory(source).value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = slotMemory(source).value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            if (f)
                f(source, target, *this);
        }
    }
}

void BattleSituation::callieffects(int source, int target, const QString &name)
{
    if (gen() <= 1 || !isOut(source))
        return;
    //Klutz
    if (hasWorkingItem(source, poke(source).item())) {
        ItemEffect::activate(name, poke(source).item(), source, target, *this);
    }
}

void BattleSituation::callaeffects(int source, int target, const QString &name)
{
    if (gen() <= 2 || !isOut(source))
        return;
    if (hasWorkingAbility(source, ability(source)))
        AbilityEffect::activate(name, ability(source), source, target, *this);
}

void BattleSituation::sendBack(int player, bool silent)
{
    /* Just calling pursuit directly here, forgive me for this */
    if (!turnMemory(player).value("BatonPassed").toBool()) {
        QList<int> opps = revs(player);
        bool notified = false;
        foreach(int opp, opps) {
            if (tmove(opp).attack == Move::Pursuit && !turnMem(opp).contains(TurnMemory::HasMoved) && !turnMemory(player).contains("RedCardUser")) {
                if (!notified) {
                    notified = true;
                    sendMoveMessage(171, 0, player);
                }
                tmove(opp).power = tmove(opp).power * 2;
                choice(opp).setTarget(player);
                analyzeChoice(opp);

                if (koed(player)) {
                    Mechanics::removeFunction(turnMemory(player),"UponSwitchIn","BatonPass");
                    break;
                }
            }
        }
    }

    BattleBase::sendBack(player, silent);

    if (!koed(player)) {
        callaeffects(player,player,"UponSwitchOut");
        /* Natural cure bypasses gastro acid (tested in 4th gen, but not role play/skill swap),
           so we don't check if the ability is working, and just make a test
           directly. */
        if (gen() <= 4 && ability(player) == Ability::NaturalCure) {
            healStatus(player, poke(player).status());
        }
    }
}

bool BattleSituation::testAccuracy(int player, int target, bool silent)
{
    int acc = tmove(player).accuracy;
    int tarChoice = tmove(player).targets;
    bool multiTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

    turnMemory(target).remove("EvadeAttack");
    callpeffects(target, player, "TestEvasion"); /*dig bounce  ... */

    if (pokeMemory(player).contains("LockedOn") && pokeMemory(player).value("LockedOnEnd").toInt() >= turn()
            && pokeMemory(player).value("LockedOn") == target &&
            pokeMemory(player).value("LockedOnCount").toInt() == slotMemory(target)["SwitchCount"].toInt()) {
        return true;
    }

    //OHKO
    int move  = turnMemory(player)["MoveChosen"].toInt();

    if (pokeMemory(player).value("BerryLock").toBool()) {
        pokeMemory(player).remove("BerryLock");
        if (gen() <= 4 || !MoveInfo::isOHKO(move, gen()))
            return true;
    }

    //No Guard, as wall as Mimic, Transform & Swift in Gen 1.
    if ((hasWorkingAbility(player, Ability::NoGuard) || hasWorkingAbility(target, Ability::NoGuard)) || (gen().num == 1 && (move == Move::Swift || move == Move::Mimic
                                                                                                                        || move == Move::Transform))) {
        return true;
    }

    /* Miracle skin can make some attacks miss */
    callaeffects(target, player, "TestEvasion");

    if (turnMemory(target).contains("EvadeAttack")) {
        if (!silent) {
            notifyMiss(multiTar, player, target);
        }
        return false;
    }

    if (acc == 0 || acc == 101 ||
            (pokeMemory(target).value("LevitatedCount").toInt() > 0 &&
             !MoveInfo::isOHKO(move, gen()))) {
        return true;
    }

    if (MoveInfo::isOHKO(move, gen())) {
        bool ret = coinflip(unsigned(30 + (gen().num == 1 ? 0 :  poke(player).level() - poke(target).level()) ), 100);
        if (!ret && !silent) {
            notifyMiss(multiTar, player, target);
        }
        return ret;
    }

    turnMemory(player).remove("Stat6ItemModifier");
    turnMemory(player).remove("Stat6AbilityModifier");
    turnMemory(target).remove("Stat7ItemModifier");
    turnMemory(target).remove("Stat7AbilityModifier");
    callieffects(player,target,"StatModifier");
    callaeffects(player,target,"StatModifier");
    callieffects(target,player,"StatModifier");
    callaeffects(target,player,"StatModifier");
    if (multiples()) {
        for (int partner = 0; partner < numberOfSlots(); partner++) {
            if (partner != player && arePartners(partner, player) && !koed(partner))
                callaeffects(partner, player, "PartnerStatModifier");
        }
    }
    /* no *=: remember, we're working with fractions & int, changing the order might screw up by 1 % or so
        due to the ever rounding down to make an int */
    acc = acc * getStatBoost(player, Accuracy) * getStatBoost(target, Evasion)
            * (20+turnMemory(player).value("Stat6ItemModifier").toInt())/20
            * (20-turnMemory(target).value("Stat7ItemModifier").toInt())/20
            * (20+turnMemory(player).value("Stat6AbilityModifier").toInt())/20
            * (20+turnMemory(player).value("Stat6PartnerAbilityModifier").toInt())/20
            * (20-turnMemory(target).value("Stat7AbilityModifier").toInt())/20;

    if (coinflip(unsigned(acc), 100)) {
        return true;
    } else {
        if (!silent) {
            notifyMiss(multiTar, player, target);
        }
        calleffects(player,target,"MissAttack");
        return false;
    }
}

void BattleSituation::testCritical(int player, int target)
{
    /* Shell armor, Battle Armor */
    if (hasWorkingAbility(target, Ability::ShellArmor)
            || hasWorkingAbility(target, Ability::BattleArmor) || teamMemory(this->player(target)).value("LuckyChantCount").toInt() > 0) {
        return;
    }

    bool critical;
    if (gen().num == 1) {
        int baseSpeed = PokemonInfo::BaseStats(fpoke(player).id).baseSpeed() / 2;

        if (tmove(player).critRaise & 1) {
            baseSpeed *= 8;
        }
        /* In RBY, Focus Energy reduces crit by 75% */
        if (tmove(player).critRaise & 2) {
            if (gen() <= Pokemon::gen(Gen::Yellow)) {
                baseSpeed /=4;
            } else {
                baseSpeed *= 4;
            }
        }
        int randnum = randint(256);

        critical = randnum < std::min(255, baseSpeed);
    } else {
        /* Flail/Reversal don't inflict crits in gen 2 */
        if (gen().num == 2 && (tmove(player).attack == Move::Flail || tmove(player).attack == Move::Reversal)) {
            return;
        }

        int minch;
        int craise = tmove(player).critRaise;

        if (hasWorkingAbility(player, Ability::SuperLuck)) { /* Super Luck */
            craise += 1;
        }

        switch(craise) {
        case 0: minch = 3; break;
        case 1: minch = 6; break;
        case 2: minch = 12; break;
        case 3: minch = 16; break;
        case 4: case 5: minch = 24; break;
        case 6: default: minch = 48;
        }

        critical = coinflip(minch, 48);
    }

    if (critical) {
        turnMem(player).add(TM::CriticalHit);
        notify(All, CriticalHit, target); // An attack with multiple targets can have varying critical hits
    } else {
        turnMem(player).remove(TM::CriticalHit);
    }

    /* In GSC, if crit and if you don't got superior boosts in offensive than in their defensive stat, you ignore boosts, burn, and screens,
       otherwise you ignore none of them */
    if (gen().num == 2) {
        int stat = 1 + (tmove(player).category - 1) * 2;
        if (fpoke(player).boosts[stat] <= fpoke(target).boosts[stat+1]) {
            turnMemory(player)["CritIgnoresAll"] = true;
        }
    }
}

bool BattleSituation::testStatus(int player)
{
    /* The second test is for abilities like Magic Mirror to not test status, because technically they are using an attack */
    if (turnMem(player).contains(TM::HasPassedStatus) || battleMemory().contains("CoatingAttackNow")) {
        return true;
    }

    if (poke(player).status() == Pokemon::Asleep) {
        if (poke(player).statusCount() > (gen().num == 1 ? 1 : 0)) {
            //Early bird
            poke(player).statusCount() -= 1 + hasWorkingAbility(player, Ability::EarlyBird);
            notify(All, StatusMessage, player, qint8(FeelAsleep));

            if (!turnMemory(player).value("SleepingMove").toBool())
                return false;
        } else {
            healStatus(player, Pokemon::Asleep);
            notify(All, StatusMessage, player, qint8(FreeAsleep));

            /* In gen 1, pokemon take a full turn to wake up */
            if (gen().num == 1)
                return false;
        }
    }
    if (poke(player).status() == Pokemon::Frozen)
    {
        if (gen() <=2 || (!coinflip(1, 5) && !(tmove(player).flags & Move::UnthawingFlag)) )
        {
            notify(All, StatusMessage, player, qint8(PrevFrozen));

            return false;
        }
        unthaw(player);
    }

    if (turnMem(player).contains(TM::Flinched)) {
        notify(All, Flinch, player);

        //SteadFast
        if (hasWorkingAbility(player, Ability::Steadfast)) {
            sendAbMessage(60,0,player);
            inflictStatMod(player, Speed, 1, player);
        }
        return false;
    }
    if (isConfused(player) && tmove(player).attack != 0) {
        if (pokeMemory(player)["ConfusedCount"].toInt() > 0) {
            inc(pokeMemory(player)["ConfusedCount"], -1);

            notify(All, StatusMessage, player, qint8(FeelConfusion));

            if (coinflip(1, 2)) {
                inflictConfusedDamage(player);
                return false;
            }
        } else {
            healConfused(player);
            notify(All, StatusMessage, player, qint8(FreeConfusion));
        }
    }

    if (poke(player).status() == Pokemon::Paralysed && tmove(player).attack != 0) {
        //MagicGuard
        if ( (gen() > 4 || !hasWorkingAbility(player, Ability::MagicGuard)) && coinflip(1, 4)) {
            notify(All, StatusMessage, player, qint8(PrevParalysed));
            return false;
        }
    }

    return true;
}

void BattleSituation::testFlinch(int player, int target)
{
    //Inner focus, shield dust
    if (hasWorkingAbility(target, Ability::ShieldDust)) {
        return;
    }

    int rate = tmove(player).flinchRate;

    if (hasWorkingAbility(target, Ability::InnerFocus)) {
        if (rate == 100 && gen() <= 4) {
            sendAbMessage(12,0,target);
        }
        return;
    }

    /* Serene Grace */
    if (hasWorkingAbility(player, Ability::SereneGrace)) {
        rate *= 2;
    }

    if (rate && coinflip(rate, 100)) {
        turnMem(target).add(TM::Flinched);
    }

    if (tmove(player).kingRock && (hasWorkingItem(player, Item::KingsRock) || hasWorkingAbility(player, Ability::Stench)
                                   || hasWorkingItem(player, Item::RazorFang))
            /* In 3rd gen, only moves without secondary effects are able to cause King's Rock flinch */
            && (gen() > 4 || (tmove(player).category == Move::StandardMove && tmove(player).flinchRate == 0))) {
        /* King's rock */
        if (coinflip(10, 100)) {
            turnMem(target).add(TM::Flinched);
        }
    }
}

void BattleSituation::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    int oldAttacker = attacker();
    int oldAttacked = attacked();

    heatOfAttack() = true;

    attacker() = player;

    int attack;

    /* Special Occurence could be through the use of Magic Mirror for example,
      that's why it's needed */
    if (!specialOccurence && !pokeMemory(player).contains("HasMovedOnce")) {
        pokeMemory(player)["HasMovedOnce"] = turn();
    }

    if (specialOccurence) {
        attack = move;
    } else {
        //Quick claw, special case
        if (gen() >= 4 && turnMemory(player).value("QuickClawed").toBool()) {
            //The message only shows up if it's not the last pokemon to move
            for (int i = 0; i < numberOfSlots(); i++) {
                if (!hasMoved(i) && !koed(i) && i != player) {
                    sendItemMessage(17, player);
                    break;
                }
            }
        }
        attack = this->move(player,move);
        fpoke(player).lastMoveSlot = move;
    }

    turnMem(player).add(TurnMemory::HasMoved);
    if (gen() >= 5) {
        counters(player).decreaseCounters();
    }

    calleffects(player,player,"EvenWhenCantMove");

    if (!testStatus(player)) {
        goto trueend;
    }

    //Just for truant
    callaeffects(player, player, "DetermineAttackPossible");
    if (turnMemory(player).value("ImpossibleToMove").toBool() == true) {
        goto trueend;
    }

    callpeffects(player, player, "DetermineAttackPossible");
    if (turnMemory(player).value("ImpossibleToMove").toBool() == true) {
        goto trueend;
    }

    turnMem(player).add(TM::HasPassedStatus);

    turnMemory(player)["MoveChosen"] = attack;

    callbeffects(player,player,"MovePossible");
    if (turnMemory(player)["ImpossibleToMove"].toBool()) {
        goto trueend;
    }

    callpeffects(player, player, "MovePossible");
    if (turnMemory(player)["ImpossibleToMove"].toBool()) {
        goto trueend;
    }

    if (!specialOccurence) {
        if (PP(player, move) <= 0) {
            /* Todo : notify of PP absence */
            goto trueend;
        }

        pokeMemory(player)[QString("Move%1Used").arg(move)] = true;

        pokeMemory(player)["LastMoveUsed"] = attack;
        pokeMemory(player)["LastMoveUsedTurn"] = turn();
        pokeMemory(player)["AnyLastMoveUsed"] = attack;
        battleMemory()["AnyLastMoveUsed"] = attack;
    } else if (attack != 0) {
        /* Recharge moves have their attack as 0 on the recharge turn : Blast Burn , ...
            So that's why attack is tested against 0. */
        pokeMemory(player)["AnyLastMoveUsed"] = attack;
        battleMemory()["AnyLastMoveUsed"] = attack;
    }

    //For metronome calling fly / sky attack / ...
    if (attack != 0) {
        fpoke(player).lastMoveUsed = attack;
    }

    calleffects(player, player, "MoveSettings");

    notify(All, UseAttack, player, qint16(attack), !(tellPlayers && !turnMemory(player).contains("TellPlayers")));

    calleffects(player, player, "AfterTellingPlayers");

    /* Lightning Rod & Storm Drain */
    foreach(int poke, sortedBySpeed()) {
        if (poke != player) {
            callaeffects(poke, player, "GeneralTargetChange");
        }
    }

    callbeffects(player,player, "GeneralTargetChange");

    targetList.clear();

    {
        int target = turnMemory(player)["Target"].toInt();

        switch(Move::Target(tmove(player).targets)) {
        case Move::Field: case Move::TeamParty: case Move::OpposingTeam:
        case Move::TeamSide:
        case Move::User: targetList.push_back(player); break;
        case Move::Opponents: {
            int opp = opponent(this->player(player));
            QVector<int> trueTargets;

            for (int i = 0; i < numberOfSlots()/2; i++) {
                if (areAdjacent(slot(opp, i), player) && !koed(slot(opp,i)))
                    trueTargets.push_back(slot(opp, i));
            }
            makeTargetList(trueTargets);
            break;
        }
        case Move::All: {
            int tp = this->player(player);
            int to = opponent(tp);

            QVector<int> trueTargets;

            for (int i = 0; i < numberOfSlots()/2; i++) {
                if (areAdjacent(slot(tp, i), player) && !koed(slot(tp, i)))
                    trueTargets.push_back(slot(tp, i));
            }
            for (int i = 0; i < numberOfSlots()/2; i++) {
                if (areAdjacent(slot(to, i), player) && !koed(slot(to, i)))
                    trueTargets.push_back(slot(to, i));
            }
            makeTargetList(trueTargets);
            break;
        }
        case Move::AllButSelf: {
            int tp = this->player(player);
            int to = opponent(tp);

            QVector<int> trueTargets;

            for (int i = 0; i < numberOfSlots()/2; i++) {
                if (areAdjacent(slot(tp, i), player) && !koed(slot(tp, i)) && player != slot(tp, i))
                    trueTargets.push_back(slot(tp, i));
            }
            for (int i = 0; i < numberOfSlots()/2; i++) {
                if (areAdjacent(slot(to, i), player) && !koed(slot(to, i)))
                    trueTargets.push_back(slot(to, i));
            }

            makeTargetList(trueTargets);
            break;
        }
        case Move::IndeterminateTarget:
        case Move::ChosenTarget: case Move::MeFirstTarget: {
            if (multiples()) {
                if (!koed(target) && target != player && canTarget(attack, player, target)) {
                    targetList.push_back(target);
                    break;
                }
            } else {
                if (!koed(target) && target != player) {
                    targetList.push_back(target);
                    break;
                }
            }
        }
            /* There is no "break" here and it is normal. Do not change the order */
        case Move::RandomTarget :
        {
            if (!turnMemory(player).contains("TargetChanged")) {
                QVector<int> possibilities;

                for (int i = 0; i < numberOfSlots(); i++) {
                    if (this->player(i) != this->player(player) && canTarget(attack, player, i) && !koed(i)) {
                        possibilities.push_back(i);
                    }
                }
                if (possibilities.size() > 0) {
                    targetList.push_back(possibilities[randint(possibilities.size())]);
                }
            } else {
                targetList.push_back(target);
            }
            break;
        }
        case Move::PartnerOrUser:
            if (!multiples()) {
                targetList.push_back(player);
            } else {
                /* Acupressure can be called with sleep talk, so the target needs to be checked */
                if (!koed(target) && arePartners(target, player) && areAdjacent(player, target)) {
                    targetList.push_back(target);
                } else {
                    targetList.push_back(player);
                }
            }
            break;
        case Move::Partner:
            if (!koed(target) && arePartners(target, player) && areAdjacent(target, player) && target != player) {
                targetList.push_back(target);
            } else {
                for (int i = 0; i < numberOfSlots(); i++) {
                    if (arePartners(i, player) && i!=player && !koed(i) && areAdjacent(i, player)) {
                        targetList.push_back(i);
                    }
                }
                if (targetList.size() > 0) {
                    int randp = targetList[randint(targetList.size())];
                    targetList.clear();
                    targetList.push_back(randp);
                }
            }
            break;
        }
    }

    if (!specialOccurence && !turnMem(player).contains(TM::NoChoice)) {
        //Pressure
        int ppsum = 1;

        if (tmove(player).targets != Move::OpposingTeam) {
            foreach(int poke, targetList) {
                if (poke != player && hasWorkingAbility(poke, Ability::Pressure)) {
                    ppsum += 1;
                }
            }
        } else {
            int opp = opponent(this->player(player));
            for (int i = 0; i < numberPerSide(); i++) {
                if (areAdjacent(player, slot(opp, i)) && hasWorkingAbility(slot(opp, i), Ability::Pressure)) {
                    ppsum += 1;
                }
            }
        }

        losePP(player, move, ppsum);
    }

    /* Choice items act before target selection if no target in gen 5 */
    callieffects(player, player, "BeforeTargetList");

    if (targetList.size() == 0) {
        notify(All, NoOpponent, player);
        goto end;
    }

    callaeffects(player,player, "BeforeTargetList");
    calleffects(player, player, "BeforeTargetList");

    /* Here because of jewels :( */
    turnMemory(player).remove("BasePowerItemModifier");

    /* Choice item memory, copycat in gen 4 and less */
    if (!specialOccurence && attack != Move::Struggle) {
        battleMemory()["LastMoveUsed"] = attack;
    }

    foreach(int target, targetList) {
        heatOfAttack() = true;
        attacked() = target;
        if (!specialOccurence && (tmove(player).flags & Move::MemorableFlag) ) {
            pokeMemory(target)["MirrorMoveMemory"] = attack;
        }

        turnMem(player).remove(TM::Failed);
        turnMem(player).add(TM::FailingMessage);

        if (target != player && !testAccuracy(player, target)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }
        //fixme: try to get protect to work on a calleffects(target, player), and wide guard/priority guard on callteffects(this.player(target), player)
        /* Protect, ... */
        callbeffects(player, target, "DetermineGeneralAttackFailure", true);
        if (testFail(player)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }
        /* Coats */
        callbeffects(player, target, "DetermineGeneralAttackFailure2", true);
        if (testFail(player)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }

        if (tmove(player).power > 0)
        {
            calculateTypeModStab();

            calleffects(player, target, "BeforeCalculatingDamage");
            /* For charge */
            callpeffects(player, target, "BeforeCalculatingDamage");

            int typemod = turnMem(player).typeMod;
            if (typemod == 0) {
                /* If it's ineffective we just say it */
                notify(All, Effective, target, quint8(typemod));
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }
            if (target != player) {
                callaeffects(target,player,"OpponentBlock");
            }
            if (turnMemory(target).contains(QString("Block%1").arg(attackCount()))) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            /* Draining moves fail against substitute in gen 2 and earlier */
            if (gen() <= 2 && hasSubstitute(target) && tmove(player).recoil > 0) {
                turnMem(player).add(TM::Failed);
                testFail(player);
                continue;
            }

            callpeffects(player, target, "DetermineAttackFailure");
            if (testFail(player)) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }
            calleffects(player, target, "DetermineAttackFailure");
            if (testFail(player)){
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            int num = repeatNum(player);
            bool hit = num > 1;

            int hitcount = 0;
            bool hitting = false;
            for (repeatCount() = 0; repeatCount() < num && !koed(target) && (repeatCount()==0 || !koed(player)); repeatCount()+=1) {
                heatOfAttack() = true;
                fpoke(target).remove(BasicPokeInfo::HadSubstitute);
                bool sub = hasSubstitute(target);
                if (sub) {
                    fpoke(target).add(BasicPokeInfo::HadSubstitute);
                }

                if (tmove(player).power > 1 && repeatCount() == 0) {
                    notify(All, Effective, target, quint8(typemod));
                }

                if (tmove(player).power > 1) {
                    testCritical(player, target);
                    calleffects(player, target, "BeforeHitting");
                    if (turnMemory(player).contains("HitCancelled")) {
                        turnMemory(player).remove("HitCancelled");
                        continue;
                    }
                    int damage = calculateDamage(player, target);
                    inflictDamage(target, damage, player, true);
                    hitcount += 1;
                    hitting = true;
                } else {
                    turnMemory(player).remove("CustomDamage");
                    calleffects(player, target, "CustomAttackingDamage");

                    if (turnMemory(player).contains("CustomDamage")) {
                        int damage = turnMemory(player).value("CustomDamage").toInt();
                        inflictDamage(target, damage, player, true);
                        hitting = true;
                    }
                }

                calleffects(player, target, "UponAttackSuccessful");
                if (!hasSubstitute(target))
                    calleffects(player, target, "OnFoeOnAttack");

                healDamage(player, target);

                heatOfAttack() = false;
                if (hitting) {
                    if (tmove(player).flags & Move::ContactFlag) {
                        if (!sub) {
                            callieffects(target, player, "UponPhysicalAssault");
                            callaeffects(target,player,"UponPhysicalAssault");
                        }
                        callaeffects(player,target,"OnPhysicalAssault");
                    }

                    if (!sub) {
                        callaeffects(target, player, "UponBeingHit");
                    }
                    callaeffects(target, player, "UponOffensiveDamageReceived");
                    callieffects(target, player, "UponBeingHit");
                }

                if (koed(target))
                    callaeffects(player, target, "AfterKoing");

                /* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
                applyMoveStatMods(player, target);

                /* For berries that activate after taking damage */
                callieffects(target, target, "TestPinch");

                if (!sub && !koed(target)) testFlinch(player, target);

                attackCount() += 1;

                if (poke(player).status() == Pokemon::Asleep && !turnMemory(player).value("SleepingMove").toBool()) {
                    break;
                }
            }

            if (hit) {
                notifyHits(player, hitcount);
            }

            if (gen() >= 5 && !koed(target) && !hasSubstitute(target)) {
                callaeffects(target, player, "AfterBeingPlumetted");
            }

            if (gen() <= 4 && koed(target))
            {
                notifyKO(target);
            }

            if (!koed(player)) {
                callieffects(player, target, "AfterAttackSuccessful");
                calleffects(player, target, "AfterAttackSuccessful");
            }

            fpoke(target).remove(BasicPokeInfo::HadSubstitute);
        } else {

            /* Needs to be called before opponentblock because lightning rod / twave */
            int type = tmove(player).type; /* move type */

            if ( target != player &&
                    ((Move::StatusInducingMove && tmove(player).status == Pokemon::Poisoned && hasType(target, Type::Poison)) ||
                     ((attack == Move::ThunderWave || attack == Move::Toxic || attack == Move::PoisonGas || attack == Move::PoisonPowder)
                      && TypeInfo::Eff(type, getType(target, 1)) * TypeInfo::Eff(type, getType(target, 2)) == 0
                      && !pokeMemory(target).value(QString("%1Sleuthed").arg(type)).toBool()))){
                notify(All, Failed, player);
                continue;
            }

            /* Needs to be called before DetermineAttackFailure because
              of SapSipper/Leech Seed */
            if (target != player) {
                callaeffects(target,player,"OpponentBlock");
            }
            if (turnMemory(target).contains(QString("Block%1").arg(attackCount()))) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            callpeffects(player, target, "DetermineAttackFailure");
            if (testFail(player)) continue;
            calleffects(player, target, "DetermineAttackFailure");
            if (testFail(player)) continue;

            if (target != player && hasSubstitute(target) && !(tmove(player).flags & Move::MischievousFlag) && attack != Move::NaturePower) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
                continue;
            }

            calleffects(player, target, "BeforeHitting");

            applyMoveStatMods(player, target);
            calleffects(player, target, "UponAttackSuccessful");
            /* Side change may switch player & target */
            if (attacker() != player) {
                player = attacker();
                target = attacked();
            }
            calleffects(player, target, "OnFoeOnAttack");
            healDamage(player, target);

            calleffects(player, target, "AfterAttackSuccessful");
        }
        if (tmove(player).type == Type::Fire && poke(target).status() == Pokemon::Frozen) {
            unthaw(target);
        }
        pokeMemory(target)["LastAttackToHit"] = attack;
    }

    heatOfAttack() = false;

    end:
        /* In gen 4, choice items are there - they lock even if the move had no target possible.  */
        callieffects(player, player, "AfterTargetList");
    trueend:

        if (gen() <= 4 && koed(player) && tmove(player).power > 0) {
        notifyKO(player);
    }

    attacker() = oldAttacker;
    attacked() = oldAttacked;

    /* For U-TURN, so that none of the variables of the switchin are afflicted, it's put at the utmost end */
    calleffects(player, player, "AfterAttackFinished");
}

void BattleSituation::useItem(int player, int item, int target, int attack)
{
    targetList.clear();

    int tar = ItemInfo::Target(item, gen());
    int p = this->player(player);

    switch (tar) {
    case Item::Team: case Item::NoTarget: target = p; break;
    case Item::Opponent: target = randomValidOpponent(player);
    default: break;
    }

    notify(All, UseItem, player, quint16(item));

    if (ItemInfo::isBerry(item)) {
        devourBerry(player, item, target);
    } else {
        turnMemory(player)["ItemAttackSlot"] = attack;

        QVariant tempItemStorage = pokeMemory(player).take("ItemArg");

        ItemEffect::setup(item, player, *this);
        ItemEffect::activate("TrainerItem", item, player, target, *this);

        /* Restoring initial conditions */
        pokeMemory(player)["ItemArg"] = tempItemStorage;
    }

    if (!turnMemory(player).value("PermanentItem").toBool()) {
        items(p)[item] -= 1;
        notify(p, ItemCountChange, p, quint16(item), items(p)[item]);

        if (items(p)[item] <= 0) {
            items(p).remove(item);
        }
    }
}

void BattleSituation::calculateTypeModStab(int orPlayer, int orTarget)
{
    int player = orPlayer == - 1 ? attacker() : orPlayer;
    int target = orTarget == - 1 ? attacked() : orTarget;

    int type = tmove(player).type; /* move type */
    int typeadv[] = {getType(target, 1), getType(target, 2)};
    int typepok[] = {getType(player, 1), getType(player, 2)};
    int typeffs[] = {TypeInfo::Eff(type, typeadv[0], gen()),TypeInfo::Eff(type, typeadv[1], gen())};
    int typemod = 1;

    for (int i = 0; i < 2; i++) {
        if (typeffs[i] == 0) {
            /* Check for grounded flying types */
            if (type == Type::Ground && !isFlying(target)) {
                typemod *= 2;
                continue;
            }
            if (pokeMemory(target).value(QString::number(typeadv[i])+"Sleuthed").toBool() || hasWorkingItem(target, Item::BullsEye)) {
                typemod *= 2;
                continue;
            }
            /* Scrappy */
            if (hasType(target, Pokemon::Ghost) && hasWorkingAbility(player,Ability::Scrappy)) {
                typemod *= 2;
                continue;
            }
        }
        typemod *= typeffs[i];
    }

    // Counter hits regardless of type matchups in Gen 1.
    if (gen().num == 1 && tmove(player).attack == Move::Counter) {
        typemod = 2;
    }
    if (type == Type::Ground && isFlying(target)) {
        typemod = 0;
    }

    int stab;
    if(type == Type::Curse) {
        // Not only Curse -- PO also has Struggle and second type
        // of single-typed pkmn as the ???-type
        stab = 2;
    }else{
        stab = 2 + (type==typepok[0] || type==typepok[1]);
    }

    turnMem(player).stab = stab;
    turnMem(player).typeMod = typemod; /* is attack effective? or not? etc. */
}

void BattleSituation::makeTargetList(const QVector<int> &base)
{
    if (gen() >= 5) {
        targetList = base.toStdVector();
        return;
    }
    targetList = sortedBySpeed();
    for (unsigned i = 0; i < targetList.size(); i++) {
        if (!base.contains(targetList[i])) {
            targetList.erase(targetList.begin()+i, targetList.begin() + i + 1);
            i--;
        }
    }
}

bool BattleSituation::hasWorkingAbility(int player, int ab)
{
    if (gen() <= 2)
        return false;

    /* Yes, illusion breaks when hit by mold breaker, right? */
    if (ab == Ability::Illusion)
        return true;
    if (ability(player) != ab)
        return false;

    if (attacking()) {
        // Mold Breaker
        if (heatOfAttack() && player == attacked() && player != attacker() &&
                (hasWorkingAbility(attacker(), ability(attacker()))
                 &&( ability(attacker()) == Ability::MoldBreaker || ability(attacker()) == Ability::TeraVoltage ||  ability(attacker()) == Ability::TurboBlaze))) {
            return false;
        }
    }
    return !pokeMemory(player).value("AbilityNullified").toBool();
}

void BattleSituation::acquireAbility(int play, int ab, bool firstTime) {
    if (gen() <= 2)
        return;

    fpoke(play).ability = ab;

    if (!pokeMemory(play).value("AbilityNullified").toBool())
        AbilityEffect::setup(ability(play),play,*this, firstTime);
}

void BattleSituation::loseAbility(int slot)
{
    if (!pokeMemory(slot).value("AbilityNullified").toBool())
        /* Evil Hack to make illusion pokemons go back to normal */
        if (pokeMemory(slot).contains("IllusionTarget"))
            callaeffects(slot, slot, "UponBeingHit");
}

int BattleSituation::ability(int player) {
    return fpoke(player).ability;
}

int BattleSituation::weight(int player) {
    int ret = fpoke(player).weight;
    if (hasWorkingAbility(player, Ability::HeavyMetal)) {
        ret *= 2;
    } else if (hasWorkingAbility(player, Ability::LightMetal)) {
        ret /= 2;
    }
    if (hasWorkingItem(player, Item::PumiceStone)) {
        ret /= 2;
    }

    if (ret == 0)
        ret = 1;

    return ret;
}

bool BattleSituation::hasWorkingItem(int player, int it)
{
    //Klutz
    return poke(player).item() == it && !pokeMemory(player).value("Embargoed").toBool() && !hasWorkingAbility(player, Ability::Klutz)
            && battleMemory().value("MagicRoomCount").toInt() == 0
            && !(ItemInfo::isBerry(poke(player).item()) && opponentsHaveWorkingAbility(player, Ability::Anxiety));
}

bool BattleSituation::opponentsHaveWorkingAbility(int play, int ability)
{
    QList<int> opponents = revs(play);

    foreach(int opponent, opponents) {
        if (hasWorkingAbility(opponent, ability))
            return true;
    }
    return false;
}

void BattleSituation::inflictRecoil(int source, int target)
{
    int recoil = tmove(source).recoil;

    if (recoil == 0)
        return;

    //Rockhead, MagicGuard
    if (koed(source) ||
            (recoil < 0 && (hasWorkingAbility(source,Ability::RockHead) || hasWorkingAbility(source,Ability::MagicGuard)))) {
        return;
    }

    notify(All, Recoil, recoil < 0 ? source : target, bool(recoil < 0));

    // "33" means one-third
    //if (recoil == -33) recoil = -100 / 3.; -- commented out until ingame confirmation

    int damage = std::abs(int(recoil * turnMemory(source).value("DamageInflicted").toInt() / 100));

    if (recoil < 0) {
        inflictDamage(source, damage, source, false);

        /* Self KO Clause! */
        if (koed(source)) {
            /* In VGC 2011 (gen 5), the user of the recoil move wins instead of losing with the Self KO Clause */
            if (gen() <= 4)
                selfKoer() = source;
            else
                selfKoer() = target;
        }
    } else  {
        if (hasWorkingItem(source, Item::BigRoot)) /* Big root */ {
            damage = damage * 13 / 10;
        }

        if (hasWorkingAbility(target, Ability::LiquidOoze)) {
            sendMoveMessage(1,2,source,Pokemon::Poison,target);
            inflictDamage(source,damage,source,false);

            /* Self KO Clause! */
            if (koed(source)) {
                if (gen() >= 5)
                    selfKoer() = target;
            }
        } else {
            if (pokeMemory(source).value("HealBlockCount").toInt() > 0) {
                sendMoveMessage(60, 0, source);
            } else {
                healLife(source, damage);
            }
        }
    }
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    applyingMoveStatMods = true;
    bool sub = hasSubstitute(target);

    BasicMoveInfo &fm = tmove(player);

    /* Moves with 0 power that came until here bypass sub,
       so we make the function think there's no sub to
       be more simple. */
    if (fm.power == 0) {
        sub = false;
    }

    int cl= fm.classification;

    /* First we check if there's even an effect... */
    if (cl != Move::StatAndStatusMove && cl != Move::StatChangingMove && cl != Move::StatusInducingMove
            && cl != Move::OffensiveSelfStatChangingMove && cl != Move::OffensiveStatusInducingMove
            && cl != Move::OffensiveStatChangingMove)
    {
        applyingMoveStatMods = false;
        return;
    }

    if ( (cl == Move::OffensiveStatChangingMove || cl == Move::OffensiveStatusInducingMove)&& hasWorkingAbility(target, Ability::ShieldDust)) {
        //        sendAbMessage(24,0,targeted,0,Pokemon::Bug);
        applyingMoveStatMods = false;
        return;
    }

    bool statChange = false;
    bool negativeStatChange = false;

    if (cl == Move::OffensiveSelfStatChangingMove) {
        target = player;
    }

    if (koed(target)) {
        applyingMoveStatMods = false;
        return;
    }

    /* Doing Stat Changes */
    for (int i = 2; i >= 0; i--) {
        char stat = fm.statAffected >> (i*8);

        if (!stat)
            break;

        char increase = char (fm.boostOfStat >> (i*8));

        int rate = char (fm.rateOfStat >> (i*8));

        if (increase < 0 && target != player && sub) {
            if (rate == 0 && cl != Move::OffensiveStatusInducingMove) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
            }
            applyingMoveStatMods = false;
            return;
        }

        /* Serene Grace, Rainbow */
        if (hasWorkingAbility(player,Ability::SereneGrace) || teamMemory(this->player(target)).value("RainbowCount").toInt() > 0) {
            rate *= 2;
        }

        if (rate != 0 && !coinflip(rate, 100)) {
            continue;
        }

        if (increase > 0) {
            if (stat == AllStats) {
                for (int i = 1; i <= 5; i++) {
                    inflictStatMod(target, i, increase, player);
                }
            } else {
                inflictStatMod(target, stat, increase, player);
            }
        } else {
            /* If we are blocked by a secondary effect, let's stop here */
            if (!inflictStatMod(target, stat, increase, player)) {
                //return; or not, hyper cutter blocks only a part of tickle
            }
            negativeStatChange = true;
        }

        statChange = true;
    }

    if (statChange == true) {
        callieffects(target, player, "AfterStatChange");
        if (target != player && negativeStatChange && gen() >= 5) {
            callaeffects(target, player, "AfterNegativeStatChange");
        }
    }

    /* Now Status */

    if (cl != Move::StatAndStatusMove && cl != Move::StatusInducingMove && cl != Move::OffensiveStatusInducingMove) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.status > Pokemon::Confused || fm.status == 0) {
        applyingMoveStatMods = false;
        return; // Other status effects than status and confusion are, on PO, dealt as special moves. Should probably be changed
    }

    if (fm.status == -1) {
        switch(randint(3)) {
        case 0: fm.status = Pokemon::Paralysed; break;
        case 1: fm.status = Pokemon::Burnt; break;
        default: case 2: fm.status = Pokemon::Frozen; break;
        }
    }

    int rate = fm.rate;

    if (target != player && sub) {
        if (rate == 0 && cl != Move::OffensiveStatChangingMove) {
            sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
        }
        applyingMoveStatMods = false;
        return;
    }

    /* Then we check if the effect hits */
    /* Serene Grace, Rainbow */
    if (hasWorkingAbility(player,Ability::SereneGrace) || teamMemory(this->player(player)).value("RainbowCount").toInt() > 0) {
        rate *= 2;
    }

    if (rate != 0 && !coinflip(rate, 100)) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.status == Pokemon::Confused)
        inflictConfused(target, player, true);
    else
        inflictStatus(target, fm.status, player, fm.minTurns, fm.maxTurns);

    applyingMoveStatMods = false;
}

bool BattleSituation::canGetStatus(int player, int status) {
    if (!BattleBase::canGetStatus(player, status)) {
        return false;
    }
    if (hasWorkingAbility(player, Ability::LeafGuard) && isWeatherWorking(Sunny))
        return false;
    switch (status) {
    case Pokemon::Paralysed: return !hasWorkingAbility(player, Ability::Limber);
    case Pokemon::Asleep: return !hasWorkingAbility(player, Ability::Insomnia) && !hasWorkingAbility(player, Ability::VitalSpirit) && !isThereUproar();
    case Pokemon::Burnt: return !hasWorkingAbility(player, Ability::WaterVeil);
    case Pokemon::Poisoned: return (gen() < 3 || !hasType(player, Pokemon::Steel)) && !hasWorkingAbility(player, Ability::Immunity);
    case Pokemon::Frozen: return !isWeatherWorking(Sunny) && !hasWorkingAbility(player, Ability::MagmaArmor);
    default:
        return false;
    }
}

bool BattleSituation::inflictStatMod(int player, int stat, int mod, int attacker, bool tell, bool *negative)
{
    bool pos = (mod > 0) ^ hasWorkingAbility(player, Ability::Perversity);
    if (negative)
        *negative = !pos;

    if (gen() >= 5 && hasWorkingAbility(player, Ability::Simple)) {
        mod *= 2;
    }

    return BattleBase::inflictStatMod(player, stat, pos ? std::abs(mod) : - std::abs(mod), attacker, tell);
}

bool BattleSituation::loseStatMod(int player, int stat, int malus, int attacker, bool tell)
{
    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnMemory(player).remove(q);
        turnMemory(player)["StatModType"] = QString("Stat");
        turnMemory(player)["StatModded"] = stat;
        turnMemory(player)["StatModification"] = -malus;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnMemory(player).contains(q)) {
            return false;
        }

        if(teamMemory(this->player(player)).value("MistCount").toInt() > 0 && (!hasWorkingAbility(attacker, Ability::SlipThrough) || this->player(player) == this->player(attacker))) {
            if (canSendPreventMessage(player, attacker)) {
                sendMoveMessage(86, 2, player,Pokemon::Ice,player, tmove(attacker).attack);
            }
            return false;
        }
    }

    int boost = fpoke(player).boosts[stat];
    if (boost > -6) {
        notify(All, StatChange, player, qint8(stat), qint8(-malus), !tell);
        changeStatMod(player, stat, std::max(boost-malus, -6));

        if (!applyingMoveStatMods) {
            callieffects(player, attacker, "AfterStatChange");
            if (player != attacker) {
                callaeffects(player, attacker, "AfterNegativeStatChange");
            }
        }
    } else {
        //fixme: can't decrease message
    }

    return true;
}

void BattleSituation::inflictStatus(int player, int status, int attacker, int minTurns, int maxTurns)
{
    //fixme: mist + intimidate
    if (poke(player).status() != Pokemon::Fine) {
        if (this->attacker() == attacker &&
                (tmove(attacker).classification == Move::StatusInducingMove ||
                 tmove(attacker).classification == Move::StatAndStatusMove) && canSendPreventSMessage(player, attacker)) {
            if (poke(player).status() == status) {
                notify(All, AlreadyStatusMessage, player, quint8(poke(player).status()));
            }
            else {
                notify(All, Failed, player);
            }
        }
        return;
    }
    if (status == Pokemon::Asleep && isThereUproar()) {
        sendMoveMessage(141,4,player);
        return;
    }
    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnMemory(player).remove(q);
        turnMemory(player)["StatModType"] = QString("Status");
        turnMemory(player)["StatusInflicted"] = status;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnMemory(player).contains(q)) {
            return;
        }

        if(teamMemory(this->player(player)).value("SafeGuardCount").toInt() > 0) {
            if (!hasWorkingAbility(attacker, Ability::SlipThrough) || this->player(player) == this->player(attacker)) {
                sendMoveMessage(109, 2, player,Pokemon::Psychic, player, tmove(player).attack);
                return;
            }
        }
    }

    if (!canGetStatus(player, status))
        return;

    if (status == Pokemon::Asleep)
    {
        if (sleepClause() && currentForcedSleepPoke[this->player(player)] != -1) {
            notifyClause(ChallengeInfo::SleepClause);
            return;
        } else {
            currentForcedSleepPoke[this->player(player)] = currentInternalId(player);
        }
    } else if (status == Pokemon::Frozen)
    {
        if (clauses() & ChallengeInfo::FreezeClause) {
            for (int i = 0; i < 6; i++) {
                if (poke(this->player(player),i).status() == Pokemon::Frozen) {
                    notifyClause(ChallengeInfo::FreezeClause);
                    return;
                }
            }
        }
    }

    changeStatus(player, status, true, minTurns == 0 ? 0 : minTurns-1 + randint(maxTurns - minTurns + 1));
    if (status == Pokemon::Frozen && poke(player).num() == Pokemon::Shaymin_S) {
        changeForme(this->player(player), slotNum(player), Pokemon::Shaymin);
    }
    if (attacker != player && status != Pokemon::Asleep && status != Pokemon::Frozen && poke(attacker).status() == Pokemon::Fine && canGetStatus(attacker,status)
            && hasWorkingAbility(player, Ability::Synchronize)) //Synchronize
    {
        sendAbMessage(61,0,player,attacker);

        int minTurns(0), maxTurns(0);

        /* Toxic is synchronized, but only in gen 5 */
        if (gen() >= 5 && status == Pokemon::Poisoned && poke(player).statusCount() > 0) {
            minTurns = maxTurns = MoveInfo::MinTurns(Move::Toxic, gen());
        }
        inflictStatus(attacker, status, player, minTurns, maxTurns);
    }
}

void BattleSituation::inflictConfused(int player, int attacker, bool tell)
{
    //fixme: insomnia/owntempo/...
    if (isConfused(player)) {
        if (this->attacker() == attacker && attacker != player && canSendPreventSMessage(player, attacker))
        {
            notify(All, AlreadyStatusMessage, player, quint8(Pokemon::Confused));
        }
        return;
    }

    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnMemory(player).remove(q);
        turnMemory(player)["StatModType"] = QString("Status");
        turnMemory(player)["StatusInflicted"] = Pokemon::Confused;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnMemory(player).contains(q)) {
            return;
        }

        if(teamMemory(this->player(player)).value("SafeGuardCount").toInt() > 0) {
            sendMoveMessage(109, 2, player,Pokemon::Psychic, player, tmove(attacker).attack);
            return;
        }
    } else if (hasWorkingAbility(player, Ability::OwnTempo)) {
        return;
    }

    poke(player).addStatus(Pokemon::Confused);
    pokeMemory(player)["ConfusedCount"] = randint(4) + 1;

    notify(All, StatusChange, player, qint8(Pokemon::Confused), true, !tell);

    callieffects(player, player,"AfterStatusChange");
}

void BattleSituation::callForth(int weather, int turns)
{
    weatherCount = turns;
    if (weather != this->weather) {
        this->weather = weather;
        foreach (int i, sortedBySpeed()) {
            callaeffects(i,i,"WeatherChange");
        }
    }
}

void BattleSituation::endTurnWeather()
{
    int weather = this->weather;

    if (weather == NormalWeather) {
        return;
    }

    int count = weatherCount - 1;
    if (count == 0) {
        notify(All, WeatherMessage, Player1, qint8(EndWeather), qint8(weather));
        callForth(NormalWeather, -1);
    } else {
        notify(All, WeatherMessage, Player1, qint8(ContinueWeather), qint8(weather));

        /* And now the weather damage! */
        if (isWeatherWorking(weather)) {
            QSet<int> immuneTypes;
            if (weather == Hail) {
                immuneTypes << Pokemon::Ice;
            } else {
                immuneTypes << Pokemon::Rock << Pokemon::Ground << Pokemon::Steel;
            }
            foreach (int i, speedsVector) {
                callaeffects(i,i,"WeatherSpecial");
                if (!turnMemory(i).contains("WeatherSpecialed") && (weather == Hail || weather == SandStorm) &&!immuneTypes.contains(getType(i,1))
                        && !immuneTypes.contains(getType(i,2)) && !hasWorkingAbility(i, Ability::MagicGuard)) {
                    notify(All, WeatherMessage, i, qint8(HurtWeather),qint8(weather));

                    //In GSC, the damage is 1/8, otherwise 1/16
                    inflictDamage(i, poke(i).totalLifePoints()*(gen() > 2 ? 1 : 2)/16, i, false);
                    if (gen() >= 5) {
                        testWin();
                    }
                }
            }
        }
        if (count > 0) {
            weatherCount = count;
        }
    }
}

bool BattleSituation::isWeatherWorking(int weather) {
    if (this->weather != weather)
        return false;

    //Air lock & Cloud nine

    for (int i = 0; i < numberOfSlots(); i++)  {
        if (!koed(i) && (hasWorkingAbility(i, Ability::AirLock) || hasWorkingAbility(i, Ability::CloudNine))) {
            return false;
        }
    }
    return true;
}

bool BattleSituation::isSeductionPossible(int seductor, int naiveone) {
    //Oblivious
    return !hasWorkingAbility(naiveone,Ability::Oblivious) && poke(seductor).gender() != Pokemon::Neutral
            && poke(naiveone).gender() != Pokemon::Neutral && poke(seductor).gender() != poke(naiveone).gender();
}

int BattleSituation::getType(int player, int slot)
{
    int types[] = {fpoke(player).type1,fpoke(player).type2};

    if (types[slot-1] == Pokemon::Flying && pokeMemory(player).value("Roosted").toBool())
    {
        if (types[1] == Pokemon::Curse && gen() >= 5)
            return Pokemon::Normal;
        else
            return Pokemon::Curse;
    }

    return types[slot-1];
}

bool BattleSituation::isFlying(int player)
{
    return !battleMemory().value("Gravity").toBool() && !hasWorkingItem(player, Item::IronBall) &&
            (gen() <= 3 || !pokeMemory(player).value("Rooted").toBool()) &&
            !pokeMemory(player).value("SmackedDown").toBool() &&
            (hasWorkingAbility(player, Ability::Levitate)
             || hasWorkingItem(player, Item::Balloon)
             || ((!attacking() || !hasWorkingItem(player, Item::BullsEye)) && hasType(player, Pokemon::Flying))
             || pokeMemory(player).value("MagnetRiseCount").toInt() > 0
             || pokeMemory(player).value("LevitatedCount").toInt() > 0);
}

void BattleSituation::changeStatus(int player, int status, bool tell, int turns)
{
    if (poke(player).status() == status) {
        return;
    }

    /* Guts needs to know if teh poke rested or not */
    if (pokeMemory(player).value("Rested").toBool()) {
        pokeMemory(player).remove("Rested");
    }

    //Sleep clause
    if (status != Pokemon::Asleep && currentForcedSleepPoke[this->player(player)] == currentInternalId(player)) {
        currentForcedSleepPoke[this->player(player)] = -1;
    }

    notify(All, StatusChange, player, qint8(status), turns > 0, !tell);
    notify(All, AbsStatusChange, this->player(player), qint8(this->slotNum(player)), qint8(status), turns > 0);

    poke(player).addStatus(status);

    if (turns != 0) {
        poke(player).statusCount() = turns;
        if (status == Pokemon::Asleep)
            poke(player).oriStatusCount() = poke(player).statusCount();
    }
    else if (status == Pokemon::Asleep) {
        if (gen() <= 2) {
            poke(player).statusCount() = 1 + (randint(6));
        } else if (gen() <= 4) {
            poke(player).statusCount() = 1 + (randint(4));
        } else {
            poke(player).statusCount() = 1 + (randint(3));
            poke(player).oriStatusCount() = poke(player).statusCount();
        }
    }
    else {
        poke(player).statusCount() = 0;
    }
    callpeffects(player, player,"AfterStatusChange");
    callieffects(player, player,"AfterStatusChange");
}

bool BattleSituation::hasMinimalStatMod(int player, int stat)
{
    return fpoke(player).boosts[stat]  <= -6;
}

bool BattleSituation::hasMaximalStatMod(int player, int stat)
{
    return fpoke(player).boosts[stat]  >= 6;
}

void BattleSituation::preventStatMod(int player, int attacker)
{
    turnMemory(player)[QString("StatModFrom%1Prevented").arg(attacker)] = true;
    turnMemory(player)[QString("StatModFrom%1DPrevented").arg(attacker)] = true;
}

bool BattleSituation::canSendPreventMessage(int defender, int attacker) {
    return attacking() && (!turnMemory(defender).contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
                           tmove(attacker).rateOfStat== 0);
}

bool BattleSituation::canSendPreventSMessage(int defender, int attacker) {
    return attacking() && (!turnMemory(defender).contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
                           tmove(attacker).rate == 0);
}

void BattleSituation::makePokemonNext(int t)
{
    int buf = speedsVector[currentSlot + 1];
    speedsVector[currentSlot+1] = t;
    for (unsigned i = currentSlot + 2; i < speedsVector.size(); i++) {
        if (buf != t) {
            std::swap(speedsVector[i], buf);
        } else {
            break;
        }
    }
}

void BattleSituation::makePokemonLast(int t)
{
    int buf = speedsVector.back();
    speedsVector.back() = t;

    for (int i = speedsVector.size() - 2; i > signed(currentSlot); i--) {
        if (buf != t) {
            std::swap(speedsVector[i], buf);
        } else {
            break;
        }
    }
}

int BattleSituation::calculateDamage(int p, int t)
{
    callaeffects(p,t,"DamageFormulaStart");

    context &move = turnMemory(p);
    PokeBattle &poke = this->poke(p);

    int level = fpoke(p).level;
    int attack, def;
    bool crit = turnMem(p).contains(TM::CriticalHit);

    int attackused = tmove(p).attack;

    int cat = tmove(p).category;
    if (cat == Move::Physical) {
        attack = getStat(p, Attack);
        def = getStat(t, Defense);
    } else {
        attack = getStat(p, SpAttack);
        if(gen().num == 1) {
            def = getStat(t, SpAttack);
        } else {
            def = getStat(t, (attackused == Move::PsychoShock || attackused == Move::PsychoBreak || attackused == Move::SecretSword) ? Defense : SpDefense);
        }
    }


    /* Used by Oaths to use a special attack, the sum of both */
    if (move.contains("AttackStat")) {
        attack = move.value("AttackStat").toInt();
        move.remove("AttackStat");
    }

    attack = std::min(attack, 65535);

    if ( (attackused == Move::Explosion || attackused == Move::Selfdestruct) && gen() <= 4) {
        /* explosion / selfdestruct */
        def/=2;
        if (def == 0)
            // prevent division by zero
            def = 1;
    }

    int stab = turnMem(p).stab;
    int typemod = turnMem(p).typeMod;
    int randnum;
    if (gen().num == 1) {
        randnum = randint(38) + 217;
    } else {
        randnum = randint(16) + 85;
    }
    //Spit Up
    if (attackused == Move::SpitUp) randnum = 100;
    else if (gen().num == 2 && (attackused == Move::Flail || attackused == Move::Reversal)) randnum = 100;

    int ch = 1 + (crit * (1+hasWorkingAbility(p,Ability::Sniper))); //Sniper

    /*** WARNING ***/
    /* The peculiar order here is caused by the fact that helping hand applies before item boosts,
      but item boosts are decided (not applied) before acrobat, and acrobat needs to modify
      move power (not just power variable) because of technician which relies on it */

    callieffects(p,t,"BasePowerModifier");
    /* The Acrobat thing is here because it's supposed to activate after Jewel Consumption */
    if (attackused == Move::Acrobat && poke.item() == Item::NoItem) {
        tmove(p).power *= 2;
    }

    int power = tmove(p).power;
    int type = tmove(p).type;

    /* Calculate the multiplier for two turn attacks */ 
    if (pokeMemory(t).contains("VulnerableMoves") && pokeMemory(t).value("Invulnerable").toBool()) {
        QList<int> vuln_moves = pokeMemory(t)["VulnerableMoves"].value<QList<int> >();
        QList<int> vuln_mults = pokeMemory(t)["VulnerableMults"].value<QList<int> >();

        for (int i = 0; i < vuln_moves.size(); i++) {
            if (vuln_moves[i] == attackused) {
                power = power * vuln_mults[i];
            }
        }
    }

    if (move.contains("HelpingHanded")) {
        power = power * 3 / 2;
    }

    power = power * (10+move["BasePowerItemModifier"].toInt())/10;

    QString sport = "Sported" + QString::number(type);
    if (battleMemory().contains(sport) && pokeMemory(battleMemory()[sport].toInt()).value(sport).toBool()) {
        power /= 2;
    }

    move.remove("BasePowerAbilityModifier");
    move.remove("BasePowerFoeAbilityModifier");
    callaeffects(p,t,"BasePowerModifier");
    callaeffects(t,p,"BasePowerFoeModifier");
    power = power * (20+move.value("BasePowerAbilityModifier").toInt())/20 * (20+move.value("BasePowerFoeAbilityModifier").toInt())/20;

    int oppPlayer = this->player(t);

    for (int i = 0; i < numberPerSide(); i++) {
        int sl = slot(oppPlayer, i);

        if (!koed(sl) && sl != t && hasWorkingAbility(sl, Ability::FriendGuard)) {
            power = power * 3 / 4;
        }
    }

    power = std::min(power, 65535);
    int damage;
    if (gen().num == 1) {
        damage = ((std::min(((level * ch * 2 / 5) + 2) * power, 65535) *
                   attack / def) / 50) + 2;
    } else {
        damage = ((std::min(((level * 2 / 5) + 2) * power, 65535) *
                   attack / 50) / def);
    }
    //Guts, burn
    if (gen() != 2 || !crit || !turnMemory(p).value("CritIgnoresAll").toBool()) {
        damage = damage / (
                    (poke.status() == Pokemon::Burnt && cat == Move::Physical && !hasWorkingAbility(p,Ability::Guts))
                    ? 2 : 1);
    }

    /* Light screen / Reflect */
    if ( (!crit || (gen().num == 2 && !turnMemory(p).value("CritIgnoresAll").toBool()) ) && !hasWorkingAbility(p, Ability::SlipThrough) &&
            (teamMemory(this->player(t)).value("Barrier" + QString::number(cat) + "Count").toInt() > 0 || pokeMemory(t).value("Barrier" + QString::number(cat) + "Count").toInt() > 0)) {
        if (!multiples())
            damage /= 2;
        else {
            damage = damage * 2 / 3;
        }
    }
    /* Damage reduction in doubles, which occur only
       if there's more than one alive target. */
    if (multiples()) {
        /* In gen 3, attacks that hit everyone don't have reduced damage */
        if (gen() >= 4 || (tmove(p).targets != Move::All && tmove(p).targets != Move::AllButSelf) ) {
            if (attackused == Move::Explosion || attackused == Move::Selfdestruct) {
                damage = damage * 3 / 4;
            } else {
                /* In gen 4, if an attack koed all previous pokemon, the last pokemon
                   would eat it at full power. Not in gen 5 */
                if (gen() <= 4) {
                    foreach (int tar, targetList) {
                        if (tar != t && !koed(tar)) {
                            damage = damage * (gen() <= 3 ? 2 : 3)/4;
                            break;
                        }
                    }
                } else {
                    if (targetList.size() > 1)
                        damage = damage * 3/4;
                }
            }
        }
    }
    if (isWeatherWorking(Sunny)) {
        if (type == Type::Fire) {
            damage = damage * 3 /2;
        } else if (type == Type::Water) {
            damage /= 2;
        }
    } else if (isWeatherWorking(Rain)) {
        if (type == Type::Water) {
            damage = damage * 3/2;
        } else if (type == Type::Fire) {
            damage /= 2;
        }
    }
    //FlashFire
    if (type == Type::Fire && pokeMemory(p).contains("FlashFired") && hasWorkingAbility(p, Ability::FlashFire)) {
        damage = damage * 3 / 2;
    }

    if (gen().num == 1) { // Gen 1 has no items and crits are already factored in.
        damage = (((damage * stab/2) * typemod/4) * randnum) / 255;
    } else {
        damage = (damage+2)*ch;
        move.remove("ItemMod2Modifier");
        callieffects(p,t,"Mod2Modifier");
        damage = damage*(10+move["ItemMod2Modifier"].toInt())/10/*Mod2*/;
        damage = damage *randnum/100*stab/2*typemod/4;

        /* Mod 3 */
        // FILTER / SOLID ROCK
        if (typemod > 4 && (hasWorkingAbility(t,Ability::Filter) || hasWorkingAbility(t,Ability::SolidRock))) {
            damage = damage * 3 / 4;
        }

        /* Expert belt */
        damage = damage * ((typemod > 4 && hasWorkingItem(p, Item::ExpertBelt))? 6 : 5)/5;

        move.remove("Mod3Berry");

        /* Berries of the foe */
        callieffects(t, p, "Mod3Items");

        damage = damage * (10 + turnMemory(p).value("Mod3Berry").toInt())/ 10;

        if (gen().num == 2)
            damage += 1;
    }

    return damage;
}

int BattleSituation::repeatNum(int player)
{
    if (turnMemory(player).contains("RepeatCount")) {
        return turnMemory(player)["RepeatCount"].toInt();
    }

    if (tmove(player).repeatMin == 0)
        return 1;

    int min = tmove(player).repeatMin;
    int max = tmove(player).repeatMax;

    if (max == 3) {
        //Triple kick, done differently...
        return 1;
    }
    //Skill link
    if (hasWorkingAbility(player, Ability::SkillLink)) {
        return max;
    }

    return minMax(min, max, gen().num, randint());
}

void BattleSituation::inflictPercentDamage(int player, int percent, int source, bool straightattack) {
    inflictDamage(player,poke(player).totalLifePoints()*percent/100,source, straightattack);
}

void BattleSituation::inflictDamage(int player, int damage, int source, bool straightattack, bool goForSub)
{
    if (koed(player)) {
        return;
    }

    if (straightattack) {
        //Sturdy in gen 5
        callaeffects(player, source, "BeforeTakingDamage");
        callieffects(player, source, "BeforeTakingDamage");
    }

    if (damage == 0) {
        damage = 1;
    }

    bool sub = hasSubstitute(player);

    // Used for Sturdy, Endure(?), Focus Sash, and Focus Band
    bool survivalFactor = false;

    if (sub && (player != source || goForSub) && straightattack) {
        inflictSubDamage(player, damage, source);
    } else {
        damage = std::min(int(poke(player).lifePoints()), damage);

        int hp  = poke(player).lifePoints() - damage;

        if (hp <= 0 && straightattack) {
            if  (   (turnMemory(player).contains("CannotBeKoedAt") && turnMemory(player)["CannotBeKoedAt"].toInt() == attackCount()) ||
                    (turnMemory(player).contains("CannotBeKoedBy") && turnMemory(player)["CannotBeKoedBy"].toInt() == source) ||
                    (turnMemory(player).value("CannotBeKoed").toBool() && source != player)) {
                damage = poke(player).lifePoints() - 1;
                hp = 1;
                survivalFactor = true;
            }
        }

        if (hp <= 0) {
            koPoke(player, source, straightattack);
        } else {

            /* Endure & Focus Sash */
            if (survivalFactor) {
                //Gen 5: sturdy
                if (turnMemory(player).contains("CannotBeKoedAt") && turnMemory(player)["CannotBeKoedAt"].toInt() == attackCount())
                    callaeffects(player, source, "UponSelfSurvival");

                if (turnMemory(player).contains("SurviveReason"))
                    goto end;

                //If it's not an ability it may be false swipe or endure
                if (turnMemory(player).contains("CannotBeKoedAt") && turnMemory(player)["CannotBeKoedAt"].toInt() == attackCount())
                    calleffects(player, source, "UponSelfSurvival");

                if (turnMemory(player).contains("SurviveReason"))
                    goto end;

                //Or, ultimately, it's an item
                callieffects(player, source, "UponSelfSurvival");

                end:
                    turnMemory(player).remove("SurviveReason");
            }

            if (straightattack) {
                notify(this->player(player), StraightDamage,player, qint16(damage));
                notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
            }


            changeHp(player, hp);
        }
    }


    if (straightattack && player != source) {
        if (!sub) {
            /* If there's a sub its already taken care of */
            inc(turnMemory(source)["DamageInflicted"], damage);
            pokeMemory(player)["DamageTakenByAttack"] = damage;
            turnMem(player).damageTaken = damage;
            turnMemory(player)["DamageTakenBy"] = source;
        }

        if (damage > 0 || (damage == 0 && survivalFactor)) {
            inflictRecoil(source, player);
            callieffects(source,player, "UponDamageInflicted");
            calleffects(source, player, "UponDamageInflicted");
        }
        if (!sub) {
            calleffects(player, source, "UponOffensiveDamageReceived");
            callpeffects(player, source, "UponOffensiveDamageReceived");
            callieffects(player, source, "UponOffensiveDamageReceived");
        }
    }

    if (!sub)
        turnMemory(player)["DamageTaken"] = damage;
}

void BattleSituation::changeDefMove(int player, int slot, int move)
{
    poke(player).move(slot).num() = move;
    poke(player).move(slot).load(gen());
    fpoke(player).moves[slot] = move;
    notify(this->player(player), ChangeTempPoke, player, quint8(DefMove), quint8(slot), quint16(move));
    changePP(player,slot,poke(player).move(slot).totalPP());
}

void BattleSituation::inflictSubDamage(int player, int damage, int source)
{
    int life = fpoke(player).substituteLife;

    if (life <= damage) {
        fpoke(player).remove(BasicPokeInfo::Substitute);
        inc(turnMemory(source)["DamageInflicted"], life);
        sendMoveMessage(128, 1, player);
        notifySub(player, false);
    } else {
        fpoke(player).substituteLife = life-damage;
        inc(turnMemory(source)["DamageInflicted"], damage);
        sendMoveMessage(128, 3, player);
    }
}

void BattleSituation::disposeItem(int player) {
    int item = poke(player).item();
    if (item != 0) {
        poke(player).itemUsed() = item;
        poke(player).itemUsedTurn() = turn();
    }
    loseItem(player);
}

void BattleSituation::eatBerry(int player, bool show) {
    int berry = poke(player).item();

    if (show && !turnMemory(player).value("BugBiter").toBool())
        sendItemMessage(8000,player,0, 0, berry);
    disposeItem(player);

    if (gen() >= 5) {
        QString harvest_key = QString("BerryUsed_%1").arg(team(this->player(player)).internalId(poke(player)));
        teamMemory(this->player(player))[harvest_key] = berry;
    }
}

void BattleSituation::devourBerry(int p, int berry, int s)
{
    int sitem = poke(s).item();
    poke(s).item() =0;

    /* Setting up the conditions so berries work properly */
    turnMemory(p)["BugBiter"] = true; // for testPinch of pinch berries to return true
    QVariant tempItemStorage = pokeMemory(p).take("ItemArg");
    acqItem(s, berry);

    /* Finding the function to call :P */
    QList<ItemInfo::Effect> l = ItemInfo::Effects(berry, gen());

    foreach(ItemInfo::Effect e, l) { /* Ripped from items.cpp (ItemEffect::activate, with some changes) */
        if (!ItemEffect::mechanics.contains(e.num)) {
            continue;
        }
        foreach (Mechanics::function f, ItemEffect::mechanics[e.num].functions) {
            //Some berries have 2 functions for pinch testing... so quitting after one used up the berry
            if (poke(s).item() == 0) {
                break;
            }

            f(p, s, *this);
        }
    }

    /* Restoring initial conditions */
    pokeMemory(p)["ItemArg"] = tempItemStorage;
    turnMemory(p).remove("BugBiter");
    poke(s).item() = sitem;
}

void BattleSituation::acqItem(int player, int item) {
    if (poke(player).item() != 0)
        loseItem(player, false);
    poke(player).item() = item;

    if (slotNum(player) < numberPerSide()) {
        ItemEffect::setup(poke(player).item(),player,*this);
        callieffects(player, player, "UponSetup");
    }
}

void BattleSituation::loseItem(int player, bool real)
{
    poke(player).item() = 0;
    if (real && slotNum(player) < numberPerSide() && hasWorkingAbility(player, Ability::Unburden)) {
        pokeMemory(player)["Unburdened"] = true;
    }
}

void BattleSituation::changeForme(int player, int poke, const Pokemon::uniqueId &newforme)
{
    PokeBattle &p  = this->poke(player,poke);
    p.num() = newforme;
    p.ability() = PokemonInfo::Abilities(newforme, gen()).ab(0);

    for (int i = 1; i < 6; i++)
        p.setNormalStat(i,PokemonInfo::FullStat(newforme,gen(),p.nature(),i,p.level(),p.dvs()[i], p.evs()[i]));

    if (isOut(player, poke)) {
        int slot = this->slot(player, poke);
        changeSprite(slot, newforme);

        fpoke(slot).id = newforme;

        if (gen() >= 3)
            acquireAbility(slot, p.ability());

        for (int i = 1; i < 6; i++)
            fpoke(slot).stats[i] = p.normalStat(i);

        fpoke(slot).type1 = PokemonInfo::Type1(newforme, gen());
        fpoke(slot).type2 = PokemonInfo::Type2(newforme, gen());
    }

    notify(All, ChangeTempPoke, player, quint8(DefiniteForme), quint8(poke), newforme);
}

void BattleSituation::changePokeForme(int slot, const Pokemon::uniqueId &newforme)
{
    PokeBattle &p  = this->poke(slot);

    fpoke(slot).id = newforme;
    fpoke(slot).type1 = PokemonInfo::Type1(newforme, gen());
    fpoke(slot).type2 = PokemonInfo::Type2(newforme, gen());

    for (int i = 1; i < 6; i++)
        fpoke(slot).stats[i] = PokemonInfo::FullStat(newforme,gen(),p.nature(),i,p.level(),p.dvs()[i], p.evs()[i]);

    notify(All, ChangeTempPoke, slot, quint8(AestheticForme), quint16(newforme.subnum));
}


void BattleSituation::changeAForme(int player, int newforme)
{
    fpoke(player).id.subnum = newforme;
    notify(All, ChangeTempPoke, player, quint8(AestheticForme), quint16(newforme));
}

void BattleSituation::changeHp(int player, int newHp)
{
    int hp = poke(player).lifePoints();

    BattleBase::changeHp(player, newHp);

    if (hp == poke(player).lifePoints()) {
        return;
    }

    callieffects(player, player, "AfterHPChange");
    callaeffects(player, player, "AfterHPChange");
}

void BattleSituation::koPoke(int player, int source, bool straightattack)
{
    if (poke(player).ko()) {
        return;
    }

    qint16 damage = poke(player).lifePoints();

    changeHp(player, 0);

    if (straightattack) {
        notify(this->player(player), StraightDamage,player, qint16(damage));
        notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
    }

    if (!attacking() || tmove(attacker()).power == 0 || gen() >= 5) {
        callaeffects(player, source, "BeforeBeingKoed");
        notifyKO(player);
    }

    //useful for third gen
    turnMem(player).add(TM::WasKoed);

    if (straightattack && player!=source) {
        callpeffects(player, source, "AfterKoedByStraightAttack");
    }

    /* For free fall */
    if (gen() >= 5)
        callpeffects(player, player, "AfterBeingKoed");
}

void BattleSituation::requestSwitchIns()
{
    testWin();
    selfKoer() = -1;

    QSet<int> koedPlayers;
    QSet<int> koedPokes;
    QSet<int> sentPokes;

    for (int i = 0; i < numberOfSlots(); i++) {
        if (!koedPlayers.contains(player(i)) && koed(i) && countBackUp(player(i)) > 0) {
            koedPlayers.insert(player(i));
            koedPokes.insert(i);

            if (gen() >= 5) {
                /* For Get Even. */
                teamMemory(player(i))["LastKoedTurn"] = turn();
            }
        }
    }

    while (koedPokes.size() > 0) {
        notifyInfos();

        foreach(int p, koedPokes) {
            requestChoice(p, false);
        }

        if (!allChoicesOkForPlayer(Player1)) {
            notify(All, StartChoices, Player1);
        }

        if (!allChoicesOkForPlayer(Player2)) {
            notify(All, StartChoices, Player2);
        }

        yield();

        /* To clear the cancellable moves list */
        for (int i = 0; i < numberOfSlots(); i++)
            couldMove[i] = false;

        foreach(int p, koedPokes) {
            analyzeChoice(p);

            if (!koed(p)) {
                sentPokes.insert(p);
            }
        }

        koedPokes.clear();
        koedPlayers.clear();

        testWin();

        for (int i = 0; i < numberOfSlots(); i++) {
            if (!koedPlayers.contains(player(i)) && koed(i) && countBackUp(player(i)) > 0) {
                koedPlayers.insert(player(i));
                koedPokes.insert(i);
            }
        }
    }

    std::vector<int> sorted = sortedBySpeed();

    /* Each wave calls the abilities in order , then next wave and so on. */
    foreach(int p, sorted) {
        if (sentPokes.contains(p)) {
            callEntryEffects(p);
        }
    }
}

void BattleSituation::requestSwitch(int slot, bool eeffects)
{
    testWin();

    int player = this->player(slot);

    if (countBackUp(player) == 0) {
        //No poke to switch in, so we won't request a choice & such;
        return;
    }

    notifyInfos();

    options[slot] = BattleChoices::SwitchOnly(slot);

    requestChoice(slot,true,true);
    analyzeChoice(slot);
    if (eeffects) {
        callEntryEffects(slot);
    }
}

bool BattleSituation::linked(int linked, QString relationShip)
{
    if (!pokeMemory(linked).contains(relationShip + "By"))
        return false;

    int linker = this->linker(linked, relationShip);

    return  !koed(linker) && slotMemory(linker)["SwitchCount"].toInt() == pokeMemory(linked)[relationShip + "Count"].toInt();
}

void BattleSituation::link(int linker, int linked, QString relationShip)
{
    pokeMemory(linked)[relationShip+"By"] = getInternalId(linker);
    pokeMemory(linked)[relationShip+"Count"] = slotMemory(linker)["SwitchCount"].toInt();
}

int BattleSituation::linker(int linked, QString relationShip)
{
    return fromInternalId(pokeMemory(linked)[relationShip + "By"].toInt());
}

void BattleSituation::changePP(int player, int move, int PP)
{
    if (isOut(player)) {
        fpoke(player).pps[move] = PP;

        if (fpoke(player).moves[move] == poke(player).move(move).num()) {
            poke(player).move(move).PP() = PP;
            notify(this->player(player), ChangePP, player, quint8(move), fpoke(player).pps[move]);
        }
        else {
            fpoke(player).pps[move] = std::min(fpoke(player).pps[move], quint8(5));
            notify(this->player(player), ChangeTempPoke, player, quint8(TempPP), quint8(move), fpoke(player).pps[move]);
        }
    } else {
        BattleBase::changePP(player, move, PP);
    }
}

void BattleSituation::gainPP(int player, int move, int gain)
{
    int PP = this->PP(player, move);

    PP = std::max(std::min(PP+gain, int(this->move(player, move) != poke(player).move(move).num() ? 5 : poke(player).move(move).totalPP())), PP);
    changePP(player, move, PP);
}

int BattleSituation::getBoostedStat(int player, int stat)
{
    if (stat == Attack && turnMemory(player).contains("CustomAttackStat")) {
        return turnMemory(player)["CustomAttackStat"].toInt();
    } else if (stat == Attack && turnMemory(player).contains("UnboostedAttackStat")) {
        return turnMemory(player)["UnboostedAttackStat"].toInt() * getStatBoost(player, Attack);
    } else{
        int givenStat = stat;
        /* Not sure on the order here... haha. */
        /* Attack and defense switched */
        if (pokeMemory(player).contains("PowerTricked") && (stat == 1 || stat == 2)) {
            givenStat = 3 - stat;
        }
        /* Wonder room: attack & sp attack switched, 5th gen */
        if (battleMemory().contains("WonderRoomCount") && (stat == 2 || stat == 4)) {
            stat = 6 - stat;
            givenStat = 6 - givenStat;
        }
        return fpoke(player).stats[givenStat] *getStatBoost(player, stat);
    }
}

void BattleSituation::notifySituation(int key)
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            notify(key, AbsStatusChange, i, qint8(j), qint8(poke(i, j).status()));
        }
        for (int k = 0; k < numberOfSlots()/ 2; k++) {
            int s = slot(i,k);
            if (!koed(s) && !rearrangeTime()) {
                notify(key, SendOut, s, true, quint8(k), opoke(s, i, k));
                /* Not clean. A pokemon with illusion could always have mimiced transform and then transformed... */
                if (!pokeMemory(s).contains("IllusionTarget"))
                    notify(key, ChangeTempPoke,s, quint8(TempSprite), pokenum(s));
                if (hasSubstitute(s))
                    notify(key, Substitute, s, hasSubstitute(s));
            }
        }
    }

    notifyInfos(key);
}

int BattleSituation::getStat(int player, int stat, int purityLevel)
{
    int baseStat = getBoostedStat(player, stat);

    QString q = "Stat"+QString::number(stat);
    turnMemory(player).remove(q+"AbilityModifier");
    turnMemory(player).remove(q+"PartnerAbilityModifier");
    turnMemory(player).remove(q+"ItemModifier");

    /* If the stat is a bit pure, we remove the item effect */
    if (purityLevel == 0)
        callieffects(player, player, "StatModifier");

    callaeffects(player, player, "StatModifier");

    if (multiples()) {
        for (int partner = 0; partner < numberOfSlots(); partner++) {
            if (partner == player || !arePartners(partner, player) || koed(partner))
                continue;
            callaeffects(partner, player, "PartnerStatModifier");
        }
    }
    int ret = baseStat*(20+turnMemory(player).value(q+"AbilityModifier").toInt())/20;

    if (multiples()) {
        ret = ret * (20+turnMemory(player).value(q+"PartnerAbilityModifier").toInt())/20;
    }

    ret = ret * (20+turnMemory(player).value(q+"ItemModifier").toInt())/20;

    if (stat == Speed) {
        if (teamMemory(this->player(player)).value("TailWindCount").toInt() > 0)
            ret *= 2;
        if (teamMemory(this->player(player)).value("SwampCount").toInt() > 0)
            ret /= 2;
        if (poke(player).status() == Pokemon::Paralysed && !hasWorkingAbility(player, Ability::QuickFeet))
            ret /= 4;
    }

    if (gen() >= 4 && stat == SpDefense && isWeatherWorking(SandStorm) && hasType(player,Pokemon::Rock))
        ret = ret * 3 / 2;

    if (ret == 0) {
        ret = 1;
    }

    if (gen() <= 2 && ret >= 1000) {
        ret = 999;
    }

    return ret;
}

ShallowBattlePoke BattleSituation::opoke(int slot, int player, int i) const
{
    if (pokeMemory(slot).contains("IllusionTarget")) {
        int illusioner = pokeMemory(slot).value("IllusionTarget").toInt();

        const PokeBattle &p2 = team(player).getByInternalId(illusioner);

        ShallowBattlePoke p = poke(player, i);
        p.num() = p2.num();
        p.nick() = p2.nick();
        p.gender() = p2.gender();
        p.shiny() = p2.shiny();

        return p;
    } else {
        return BattleBase::opoke(slot, player, i);
    }
}

void BattleSituation::fail(int player, int move, int part, int type, int trueSource)
{
    failSilently(player);
    sendMoveMessage(move, part, trueSource != -1? trueSource : player, type, player,turnMemory(player)["MoveChosen"].toInt());
}

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = fpoke(player).boosts[stat];

    if (gen() <= 4 && hasWorkingAbility(player,Ability::Simple)) {
        boost = std::max(std::min(boost*2, 6),-6);
    }

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */
    int attacker = this->attacker();
    int attacked = this->attacked();

    if (attacker != -1 && attacked != -1) {
        //Unaware / Sacred sword
        if (attacker != player && attacked == player) {
            if ((hasWorkingAbility(attacker, Ability::Unaware) || tmove(attacker).attack == Move::PaymentPlan || tmove(attacker).attack == Move::SacredSword)
                    && (stat == SpDefense || stat == Defense || stat == Evasion)) {
                boost = 0;
            }
        } else if (attacker == player && attacked != player && hasWorkingAbility(attacked, Ability::Unaware) &&
                   (stat == SpAttack || stat == Attack || stat == Accuracy)) {
            boost = 0;
        }
        //Critical hit
        if (turnMem(attacker).contains(TM::CriticalHit)) {
            if (gen() >= 3) {
                if ((stat == Attack || stat == SpAttack) && boost < 0) {
                    boost = 0;
                } else if ((stat == Defense || stat == SpDefense) && boost > 0) {
                    boost = 0;
                }
            } else if (gen().num == 1){
                boost = 0;
            } else if (gen().num == 2 && turnMemory(attacker).value("CritIgnoresAll").toBool()) {
                boost = 0;
            }
        }
    }

    if (stat <= 5) {
        return PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else if (stat == Accuracy) {
        /* Accuracy */
        return PokeFraction(std::max(3+boost, 3), std::max(3-boost, 3));
    } else {
        /* Evasion */
        if (pokeMemory(player).value("Sleuthed").toBool() && boost > 0) {
            boost = 0;
        }
        if (battleMemory().value("Gravity").toBool()) {
            boost = std::max(boost-2,-6);
        }
        return PokeFraction(std::max(3-boost, 3), std::max(3+boost, 3));
    }
}

BattleDynamicInfo BattleSituation::constructInfo(int slot)
{
    BattleDynamicInfo ret = BattleBase::constructInfo(slot);

    int player = this->player(slot);

    if (teamMemory(player).contains("Spikes")) {
        switch (teamMemory(player).value("Spikes").toInt()) {
        case 1: ret.flags |= BattleDynamicInfo::Spikes; break;
        case 2: ret.flags |= BattleDynamicInfo::SpikesLV2; break;
        case 3: ret.flags |= BattleDynamicInfo::SpikesLV3; break;
        }
    }
    if (teamMemory(player).contains("ToxicSpikes")) {
        switch (teamMemory(player).value("ToxicSpikes").toInt()) {
        case 1: ret.flags |= BattleDynamicInfo::ToxicSpikes; break;
        case 2: ret.flags |= BattleDynamicInfo::ToxicSpikesLV2; break;
        }
    }
    if (teamMemory(player).contains("StealthRock") && teamMemory(player).value("StealthRock").toBool()) {
        ret.flags |= BattleDynamicInfo::StealthRock;
    }

    return ret;
}

void BattleSituation::addUproarer(int player)
{
    if (!battleMemory().contains("Uproarer")) {
        QVariant v;
        v.setValue(QSharedPointer<QSet<int> > (new QSet<int>()));
        battleMemory()["Uproarer"] = v;
    }

    battleMemory()["Uproarer"].value< QSharedPointer<QSet<int> > >()->insert(player);
}

void BattleSituation::removeUproarer(int player)
{
    battleMemory()["Uproarer"].value<QSharedPointer<QSet<int> > >()->remove(player);
}

bool BattleSituation::isThereUproar()
{
    if (!battleMemory().contains("Uproarer")) {
        return false;
    }

    foreach(int player, *battleMemory()["Uproarer"].value<QSharedPointer< QSet<int> > >()) {
        if (!koed(player) && pokeMemory(player).value("UproarUntil").toInt() >= turn()) {
            return true;
        }
    }

    return false;
}

void BattleSituation::storeChoice(const BattleChoice &b)
{
    BattleBase::storeChoice(b);

    /* If the move is encored, a random target is picked. */
    if (counters(b.slot()).hasCounter(BattleCounterIndex::Encore))
        choice(b.slot()).choice.attack.attackTarget = b.slot();
}

void BattleSituation::setupMove(int i, int move)
{
    MoveEffect::setup(move,i,0,*this);
}

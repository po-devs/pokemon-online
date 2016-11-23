#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>
#include "battle.h"

#include <PokemonInfo/pokemoninfo.h>
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

BattleSituation::BattleSituation(const BattlePlayer &p1, const BattlePlayer &p2, const ChallengeInfo &c, int id, const TeamBattle &t1, const TeamBattle &t2, BattleServerPluginManager *p)
{
    init(p1, p2, c, id, t1, t2, p);

    for (int i = 0; i < numberOfSlots(); i++) {
        slotzone.push_back(context());
        contexts.push_back(PokeContext());
        indexes.push_back(i);
    }

    megas[0] = megas[1] = false;
    zmoves[0] = zmoves[1] = false;
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
    6.5 Poison Heal, Poison: "pokémon is hurt by poison"
    6.6 Burn
    6.7 Nightmare
    6.8 Flame Orb activation, Toxic Orb activation
    6.9 Curse (from a Ghost)
    6.10 Bind, Clamp, Fire Spin, Magma Storm, Sand Tomb, Whirlpool, Wrap
    6.11 Bad Dreams Damage
    6.12 End of Outrage, Petal Dance, Thrash, Uproar: "pokémon caused an uproar" & "pokémon calmed down"
    6.13 Disable ends: "pokémon is no longer disabled"
    6.14 Encore ends
    6.15 Taunt wears off
    6.16 Magnet Rise
    6.17 Heal Block: "the foe pokémon's heal block wore off"
    6.18 Embargo
    6.19 Yawn
    6.20 Sticky Barb

    7.0 Doom Desire, Future Sight

    8.0 Perish Song

    9.0 Trick Room

    10.0 Pokemon is switched in (if previous Pokemon fainted)
    10.1 Toxic Spikes
    10.2 Spikes
    10.3 Stealth Rock

    11.0 Healing Heart
    12.0 Slow start, Forecast
    */
    else if (gen() <= 4) {
        ownEndFunctions.push_back(QPair<int, VoidFunction>(3, &BattleSituation::endTurnWeather));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(10, &BattleSituation::requestEndOfTurnSwitchIns));

        addEndTurnEffect(AbilityEffect, 6, 2); /* Shed Skin, Speed Boost */
        addEndTurnEffect(ItemEffect, 6, 3); /* Black Sludge, Leftovers */

        addEndTurnEffect(OwnEffect, 6, 5, 0, "", NULL, &BattleSituation::endTurnPoison);
        addEndTurnEffect(OwnEffect, 6, 6, 0, "", NULL, &BattleSituation::endTurnBurn);

        addEndTurnEffect(ItemEffect, 6, 8); /* Orbs */
        addEndTurnEffect(AbilityEffect, 6, 11); /* Bad Dreams */
        addEndTurnEffect(ItemEffect, 6, 20); /* Sticky Barb */


        addEndTurnEffect(AbilityEffect, 12, 0); /* Slow Start, Forecast */
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

        9.0 (bad) poison damage, Poison Heal

        10.0 burn damage

        11.0 Nightmare

        12.0 Curse (from a Ghost-type)

        13.0 Bind, Wrap, Fire Spin, Clamp, Whirlpool, Sand Tomb, Magma Storm

        14.0 Taunt ends
        14.1 Throat Chop ends //Unconfirmed

        15.0 Encore ends

        16.0 Disable ends, Cursed Body ends

        17.0 Magnet Rise ends

        18.0 Telekinesis ends

        19.0 Heal Block ends

        20.0 Embargo ends

        21.0 Yawn

        22.0 Perish Song

        23.0 Reflect ends, Light Screen ends
        23.1 Aurora Veil ends
        23.2 Safeguard ends
        23.3 Mist ends
        23.4 Tailwind ends
        23.5 Lucky Chant ends
        23.6 Water Pledge + Fire Pledge ends, Fire Pledge + Grass Pledge ends, Grass Pledge + Water Pledge ends

        24.0 Gravity ends

        25.0 Trick Room ends

        26.0 Wonder Room ends

        27.0 Magic Room ends

        28.0 Uproar message
        28.1 Speed Boost, Bad Dreams, Harvest, Moody
        28.2 Toxic Orb activation, Flame Orb activation, Sticky Barb
        28.3 pickup

        29.0 Zen Mode

        30.0 Pokémon is switched in (if previous Pokémon fainted)
        30.1 Healing Wish, Lunar Dance
        30.2 Spikes, Toxic Spikes, Stealth Rock (hurt in the order they are first used)

        31.0 Slow Start, Forecast
        */
        ownEndFunctions.push_back(QPair<int, VoidFunction>(1, &BattleSituation::endTurnWeather));
        ownEndFunctions.push_back(QPair<int, VoidFunction>(30, &BattleSituation::requestEndOfTurnSwitchIns));

        addEndTurnEffect(AbilityEffect, 5, 1); /* Shed skin, Hydration, Healer */
        addEndTurnEffect(ItemEffect, 5, 2); /* Leftovers, Black sludge */

        addEndTurnEffect(OwnEffect, 9, 0, 0, "", NULL, &BattleSituation::endTurnPoison);
        addEndTurnEffect(OwnEffect, 10, 0, 0, "", NULL, &BattleSituation::endTurnBurn);

        addEndTurnEffect(AbilityEffect, 28, 1); /* Speed Boost, Bad Dreams, Harvest, Pickup Moody */
        addEndTurnEffect(ItemEffect, 28, 2); /* Orbs, sticky barb */

        addEndTurnEffect(AbilityEffect, 29, 0); /* Daruma Mode */

        addEndTurnEffect(AbilityEffect, 31, 0); /* Slow Start, Forecast */
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

//For Gen 5+, use the hex value
//For Gens 3 & 4, (value-1)*20 (ie. 1.2 = 4, 0.8 = -4)
void BattleSituation::chainBp(int , int mod)
{
    bpmodifiers.append(mod);
}
void BattleSituation::chainAtk(int , int mod)
{
    atkmodifiers.append(mod);
}
void BattleSituation::clearBp()
{
    bpmodifiers.clear();
}
void BattleSituation::clearAtk()
{
    atkmodifiers.clear();
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
    // Only gen 2 as it is supposed to get called at a different time
    foreach (int player, speedsVector) {
        if (poke(player).status() == Pokemon::Frozen && coinflip(26, 255)) {
            unthaw(player);
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
    endTurnPoison(player);
    endTurnBurn(player);
    callpeffects(player, player, "EndTurn6.4");//leech seed
    callpeffects(player, player, "EndTurn6.7");//nightmare
    callpeffects(player, player, "EndTurn6.9");//curse
    callpeffects(player, player, "EndTurn6.10");//bind

    testWin();
}

void BattleSituation::endTurnPoison(int player)
{
    if (koed(player)|| poke(player).status() != Pokemon::Poisoned)
        return;

    // PoisonHeal
    if (hasWorkingAbility(player, Ability::PoisonHeal)) {
        if (canHeal(player,HealByAbility,ability(player))) {
            sendAbMessage(45,0,player,0,Pokemon::Poison);
            healLife(player, poke(player).totalLifePoints()/8);
        }
    } else if (!hasWorkingAbility(player, Ability::MagicGuard)) {
        notify(All, StatusMessage, player, qint8(HurtPoison));

        if (poke(player).statusCount() == 0) {
            inflictDamage(player, poke(player).totalLifePoints() / 8, player);
        } else {
            inflictDamage(player, poke(player).totalLifePoints() * (16 - poke(player).statusCount()) / 16, player);
            //poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1); //Already being applied earlier.
        }
    }
    /* Toxic still increases under magic guard, poison heal */
    if (poke(player).statusCount() != 0)
        poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
}

void BattleSituation::endTurnBurn(int player)
{
    if (koed(player) || poke(player).status() != Pokemon::Burnt)
        return;

    if (hasWorkingAbility(player, Ability::MagicGuard))
        return;

    notify(All, StatusMessage, player, qint8(HurtBurn));
    //HeatProof cuts burn damage in half
    int denom = 8;
    if (hasWorkingAbility(player, Ability::Heatproof)) {
        denom *= 2;
    }
    //Gen 7 cut burn damage in half (1/8 to 1/16)
    if (gen() >= 7) {
        denom *= 2;
    }
    inflictDamage(player, poke(player).totalLifePoints() / denom, player);
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

    //Mega Evolution is not hindered by Embargo, etc.
    if (canMegaEvolve(slot)) {
        Pokemon::uniqueId forme = poke(slot).num() == Pokemon::Rayquaza ? Pokemon::Rayquaza_Mega : ItemInfo::MegaStoneForme(poke(slot).item());
        if (!bannedPokes[0].contains(PokemonInfo::Name(forme)) && !bannedPokes[1].contains(PokemonInfo::Name(forme))) {
            ret.mega = true;
        }
    }
    //Assuming same for ZMove, unconfirmed
    if (canUseZMove(slot)) {
        ret.zmove = true;
    }

    if (!hasWorkingItem(slot, Item::ShedShell) && (gen() < 6 || !hasType(slot, Type::Ghost))) {
        /* Shed Shell */
        if (linked(slot, "Blocked") || linked(slot, "Trapped")) {
            ret.switchAllowed = false;
        }

        if (pokeMemory(slot).contains("Rooted")) {
            ret.switchAllowed = false;
        }

        if (battleMemory().contains("FairyLockCount")) {
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
    bool possible = (BattleBase::isMovePossible(player, move) && turnMemory(player).value("Move" + QString::number(move) + "Blocked").toBool() == false);

    if (possible) {
        int attack = fpoke(player).moves[move];

        if (attack == Move::Belch && !(battleMemory().value(QString("BerryEaten%1%2").arg(this->player(player)).arg(currentInternalId(player))).toBool())) {
            return false;
        }
    }

    return possible;
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
    //UNTESTED: Gen 7 mega evolution changes turn order now.
    if (gen() >= 7) {
        foreach(int i, playersByOrder) {
            if (choice(i).attackingChoice() || choice(i).moveToCenterChoice()) {
                int slot = i;
                megaEvolve(slot);
            }
        }
        playersByOrder = sortedBySpeed();
    }

    foreach(int i, playersByOrder) {
        if (choice(i).itemChoice()) {
            items.push_back(i);
        } else if (choice(i).switchChoice()) {
            switches.push_back(i);
        } else if (choice(i).attackingChoice()){
            if (gen() >= 5) {
                calleffects(i, i, "PriorityChoice"); //Me First. Needs to go above aeffects
                callaeffects(i, i, "PriorityChoice");
            }
            priorities[tmove(i).priority].push_back(i);
        } else if (choice(i).moveToCenterChoice()){
            /* Shifting choice */
            priorities[0].push_back(i);
        }
    }

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;
    std::vector<int> &players = speedsVector;
    players.clear();

    /* Needs to be before switches, otherwise analytic + pursuit on empty speed vector crashes the game */
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

    foreach(int player, items) {
        analyzeChoice(player);
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

    foreach(int i, playersByOrder) {
        if (choice(i).attackingChoice() || choice(i).moveToCenterChoice()) {
            int slot = i;
            if (gen() < 7) {
                megaEvolve(slot);
            }
            useZMove(slot);
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

void BattleSituation::megaEvolve(int slot)
{
    //Split to allow Mega Evo to activate on Special Pursuit
    //Mega Evolution is not hindered by Embargo, etc.
    if (choice(slot).mega()) {
        if (canMegaEvolve(slot)) {
            Pokemon::uniqueId forme = poke(slot).num() == Pokemon::Rayquaza ? Pokemon::Rayquaza_Mega : ItemInfo::MegaStoneForme(poke(slot).item());
            if (!bannedPokes[0].contains(PokemonInfo::Name(forme)) && !bannedPokes[1].contains(PokemonInfo::Name(forme))) {
                //The Strong weather ability is lost before mega evolution occurs. Illusion however does NOT fade, so can't just call loseAbility haphazardly
                if (ability(player(slot)) == Ability::DesolateLand || ability(player(slot)) == Ability::PrimordialSea || ability(player(slot)) == Ability::DeltaStream) {
                    loseAbility(player(slot));
                }
                sendItemMessage(66, slot, 0, 0, 0, forme.toPokeRef());
                changeForme(player(slot), slotNum(slot), forme, false, false, true);
                megas[player(slot)] = true;
                pokeMemory(player(slot))["MegaEvolveTurn"] = turn();
            }
        }
    }
}

void BattleSituation::useZMove(int slot)
{
    if (choice(slot).zmove()) {
        if (canUseZMove(slot)) {
            zmoves[player(slot)] = true;
            pokeMemory(player(slot))["ZMoveTurn"] = turn();
        }
    }
}

void BattleSituation::sendPoke(int slot, int pok, bool silent)
{
    int player = this->player(slot);
    int snum = slotNum(slot);

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
            p.statusCount() = gen() <= 2 ? 0 : 15;
        else if (p.status() != Pokemon::Asleep)
            p.statusCount() = 0;
    }
    if (p.status() == Pokemon::Asleep && gen().num == 5) {
        p.statusCount() = p.oriStatusCount();
    }

    /* Increase the "switch count". Switch count is used to see if the pokemon has switched
       (like for an attack like attract), it is imo more effective that other means */
    inc(slotMemory(slot)["SwitchCount"], 1);
    slotMemory(slot)["SwitchTurn"] = turn();

    //we need to check for Multitype in case Arceus doesn't have its ability
    if (p.num() == Pokemon::Arceus && p.ability() == Ability::Multitype) {
        int type = Type::Normal;
        if (ItemInfo::isPlate(p.item())) {
            type = ItemInfo::PlateType(p.item());
        }
        //TODO: Code Z Crystal Types
        /*else if (ItemInfo::isZCrystal(p.item())) {
            type = ItemInfo::ZCrystalType(p.item());
        }*/
        if (type != Type::Normal) {
            changeAForme(slot, type);
        }
    }
    //we need to check for RKSSystem in case Silvally doesn't have its ability
    if (p.num() == Pokemon::Silvally && ItemInfo::isMemoryChip(p.item()) && p.ability() == Ability::RKSSystem) {
        int type = ItemInfo::MemoryChipType(p.item());

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

    /* If the switch in occured due to a ko
       we need to remove this so that hazard kos can happen */
    turnMem(player).remove(TM::WasKoed);

    calleffects(slot, slot, "UponSwitchIn");
    callseffects(slot, slot, "UponSwitchIn");
    callzeffects(player, slot, "UponSwitchIn");
    if(turn() != 0) {
        QList<int> opps = revs(slot);
        foreach(int opp, opps){
            callaeffects(opp, opp, "UponOpponentSwitchIn");
        }
    }
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

        if (gen() >= 3 && !turnMemory(player).contains("PrimalForme"))
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
    if (isOut(source) && hasWorkingItem(source, poke(source).item())) {
        ItemEffect::activate(name, poke(source).item(), source, target, *this);
    }
}

void BattleSituation::callaeffects(int source, int target, const QString &name)
{
    if (gen() > 2 && isOut(source) && hasWorkingAbility(source, ability(source))) {
        AbilityEffect::activate(name, ability(source), source, target, *this);
    }
}

void BattleSituation::sendBack(int player, bool silent)
{
    /* Just calling pursuit directly here, forgive me for this */
    if (!turnMemory(player).value("BatonPassed").toBool()) {
        QList<int> opps = revs(player);
        bool notified = false;
        foreach(int opp, opps) {
            //Pursuit does not deal additional effects to a teammate switching
            if (tmove(opp).attack == Move::Pursuit && !turnMem(opp).contains(TurnMemory::HasMoved) && !turnMemory(player).contains("RedCardUser") && !arePartners(opp, player)) {
                megaEvolve(opp);
                if (!notified) {
                    notified = true;
                    sendMoveMessage(171, 0, player);
                }
                turnMemory(player)["SendingBack"] = true; //To prevent PinchStat berries from activating right before switch
                if (gen().num > 2) {
                    tmove(opp).power = tmove(opp).power * 2;
                } else {
                    turnMemory(player)["PursuitedOnSwitch"] = true;
                }
                choice(opp).setTarget(player);
                analyzeChoice(opp);

                if (koed(player)) {
                    Mechanics::removeFunction(turnMemory(player),"UponSwitchIn","BatonPass");
                    //If a Pokemon is KOed with Pursuit when it is being sent back, we don't want to display the sending back message, so we override whatever was already defined.
                    silent = true;
                    break;
                }
            }
        }
    }
    if (pokeMemory(player).contains("PreTransformPoke")) {
        changeForme(this->player(player),slotNum(player),PokemonInfo::Number(pokeMemory(player).value("PreTransformPoke").toString()));
    }
    //If you primal evolve and die or are forced out on the same turn, the new pokemon's ability isn't loaded without unloading primal forme.
    if (turnMemory(player).contains("PrimalForme")) {
        turnMemory(player).remove("PrimalForme");
    }

    /*ADV: Sleep Turns spent SleepTalking/Snoring do not deduct from sleep counter
     * if you switch out while still asleep and the last move used was Sleep Talk or Snore. */
    if (gen().num == 3 && poke(player).status() == Pokemon::Asleep) {
        poke(player).statusCount() += poke(player).advSleepCount();
        //Variable cleared once used.
        poke(player).advSleepCount() = 0;
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
    callaeffects(player, target, "ActivateProtean");
    callpeffects(target, player, "TestEvasion"); /*dig bounce  ... */

    if (pokeMemory(player).contains("LockedOn") && pokeMemory(player).value("LockedOnEnd").toInt() >= turn()
            && pokeMemory(player).value("LockedOn") == target &&
            pokeMemory(player).value("LockedOnCount").toInt() == slotMemory(target)["SwitchCount"].toInt()) {
        return true;
    }

    //OHKO
    int move  = turnMemory(player)["MoveChosen"].toInt();

    //Micle Berry only guarantees next hit in Gen 4 (bar OHKO)
    if (pokeMemory(player).value("BerryLock").toBool()) {
        pokeMemory(player).remove("BerryLock");
        if (!MoveInfo::isOHKO(move, gen()))
            return true;
    }

    if (hasWorkingAbility(player, Ability::NoGuard) || hasWorkingAbility(target, Ability::NoGuard)) {
        return true;
    }

    //Toxic always hits if used by poison type in gen 6
    if (move == Move::Toxic && gen() > 5 && hasType(player, Type::Poison)) {
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
        bool ret = coinflip(unsigned(30 + poke(player).level() - poke(target).level()), 100);
        if (!ret && !silent) {
            notifyMiss(multiTar, player, target);
        }
        return ret;
    }

    turnMemory(player).remove("Stat6ItemModifier");
    turnMemory(player).remove("Stat6AbilityModifier");
    turnMemory(target).remove("Stat7ItemModifier");
    turnMemory(target).remove("Stat7AbilityModifier");
    pokeMemory(player).remove("Stat6BerryModifier");
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

    if (true || gen() < 5) {
        /* no *=: remember, we're working with fractions & int, changing the order might screw up by 1 % or so
                due to the ever rounding down to make an int */
        acc = acc * getStatBoost(player, Accuracy) * getStatBoost(target, Evasion)
                * (20+turnMemory(player).value("Stat6ItemModifier").toInt())/20
                * (20-turnMemory(target).value("Stat7ItemModifier").toInt())/20
                * (20+turnMemory(player).value("Stat6AbilityModifier").toInt())/20
                * (20+turnMemory(player).value("Stat6PartnerAbilityModifier").toInt())/20
                * (20-turnMemory(target).value("Stat7AbilityModifier").toInt())/20
                * (20+pokeMemory(player).value("Stat6BerryModifier").toInt())/20;
    } else {
        //Unconfirmed: The precise chaining order. Assumed Ability > Item. This might need further tweaking if the information presents itself
        //Unconfirmed: Bulbapedia claims only a total of 6 -ACC or +EVA are counted in gen 3+. This means a move with 100% accuracy can only go as low as 33% before applying additional mods
        int accChain = 0x1000;
            accChain = chainMod(accChain, turnMemory(player).value("Stat6AbilityModifier").toInt());
            accChain = chainMod(accChain, turnMemory(player).value("Stat6PartnerAbilityModifier").toInt());
            accChain = chainMod(accChain, turnMemory(player).value("Stat6ItemModifier").toInt());
            accChain = chainMod(accChain, pokeMemory(player).value("Stat6BerryModifier").toInt());

        int evaChain = 0x1000;
            evaChain = chainMod(evaChain, turnMemory(target).value("Stat7AbilityModifier").toInt());
            evaChain = chainMod(evaChain, turnMemory(target).value("Stat7ItemModifier").toInt());

        acc = applyMod(acc, accChain) * getStatBoost(player, Accuracy) * getStatBoost(target, Evasion) / applyMod(1, evaChain);
    }
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

    bool critical = false;
    /* Flail/Reversal don't inflict crits in gen 2 */
    if (gen().num == 2 && (tmove(player).attack == Move::Flail || tmove(player).attack == Move::Reversal)) {
        return;
    }

    int minch;
    int craise = tmove(player).critRaise;

    if (hasWorkingAbility(player, Ability::SuperLuck)) { /* Super Luck */
        craise += 1;
    }

    if (gen() < 6) {
        switch(craise) {
        case 0: minch = 3; break;
        case 1: minch = 6; break;
        case 2: minch = 12; break;
        case 3: minch = 16; break;
        case 4: case 5: minch = 24; break;
        case 6: default: minch = 48;
        }
    } else {
        switch(craise) {
        case 0: minch = 3; break;
        case 1: minch = 6; break;
        case 2: minch = 24; break;
        case 3: default: minch = 48;
        }
    }

    critical = coinflip(minch, 48);

    bool isCrit;
    if (pokeMemory(player).contains("LaserFocused") && pokeMemory(player).value("LaserFocusEnd").toInt() >= turn()) {
        isCrit = true;
    }
    if (hasWorkingAbility(player, Ability::Merciless) && poke(target).status() == Pokemon::Poisoned) {
        isCrit = true;
    }

    if (critical || isCrit) {
        turnMem(player).add(TM::CriticalHit);
        notify(All, CriticalHit, target); // An attack with multiple targets can have varying critical hits        
        pokeMemory(player).remove("LaserFocused");
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
        if (poke(player).statusCount() > 0) {
            //Early bird
            poke(player).statusCount() -= 1 + hasWorkingAbility(player, Ability::EarlyBird);
            notify(All, StatusMessage, player, qint8(FeelAsleep));

            if (!turnMemory(player).value("SleepingMove").toBool())
                return false;
        } else {
            healStatus(player, Pokemon::Asleep);
            notify(All, StatusMessage, player, qint8(FreeAsleep));
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

            int coin = gen() > 6 ? 3 : 2; //gen 7 confuse is 1/3
            if (coinflip(1, coin)) {
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

    /* Serene Grace, Rainbow (Pledges) */
    //We split them up because Serene Grace is cumulative with Rainbow (Pledges)
    if (tmove(player).attack != Move::SecretPower) {
        if (hasWorkingAbility(player,Ability::SereneGrace)) {
            rate *= 2;
        }
        if (teamMemory(this->player(target)).value("RainbowCount").toInt() > 0) {
            rate *=2;
        }
    }

    if (rate && coinflip(rate, 100)) {
        turnMem(target).add(TM::Flinched);
    }

    //Important to note: Stench does not stack with items
    if ((hasWorkingAbility(player, Ability::Stench) && gen().num >= 5) || hasWorkingItem(player, Item::KingsRock) || hasWorkingItem(player, Item::RazorFang)){
        if (gen().num >= 5){
            //As long as the move does damage and does not already have a chance to flinch, it will gain the effect
            if (tmove(player).flinchRate == 0 && tmove(player).category != Move::Other) {
                int rate2 = 10;
                if (hasWorkingAbility(player,Ability::SereneGrace)){
                    rate2 *=2;
                }
                if (teamMemory(this->player(target)).value("RainbowCount").toInt() > 0){
                    rate2 *=2;
                }
                if (rate2 && coinflip(rate2, 100)) {
                    turnMem(target).add(TM::Flinched);
                }
            }
        } else if (gen().num == 4 || gen().num == 3){
            if (tmove(player).kingRock) {
                if (coinflip(10, 100)) {
                    turnMem(target).add(TM::Flinched);
                }
            }
        } else if (gen().num == 2){
            if (tmove(player).kingRock) {
                if (coinflip(30, 256)) {
                    turnMem(target).add(TM::Flinched);
                }
            }
        }
    }
}

void BattleSituation::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    int oldAttacker = attacker();
    int oldAttacked = attacked();
    /* For Sleep Talk */
    bool special = specialOccurence;

    heatOfAttack() = true;

    attacker() = player;

    int attack;

    //Dead Pokemon shouldn't be attacking...
    if (koed(player)) {
        goto trueend;
    }

    /* Special Occurence could be through the use of Magic Mirror for example,
      that's why it's needed */
    if (!specialOccurence && !pokeMemory(player).contains("HasMovedOnce")) {
        pokeMemory(player)["HasMovedOnce"] = turn();
    }

    if (specialOccurence) {
        attack = move;
        turnMemory(player)["SpecialMoveUsed"] = move;
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
    if (gen() >= 5 && !battleMemory().value("CoatingAttackNow").toBool()) {
        counters(player).decreaseCounters();
    }

    calleffects(player,player,"EvenWhenCantMove");
    callaeffects(player,player,"EvenWhenCantMove");

    if (!testStatus(player)) {
        goto trueend;
    }

    //Just for truant
    callaeffects(player, player, "DetermineAttackPossible");
    /*Normalize, Aerilate, etc. Needs to be higher than "MovesPossible" to allow proper interaction with Ion Deluge*/
    callaeffects(player, player, "MoveSettings");

    if (!specialOccurence) {
        if (turnMemory(player).value("ImpossibleToMove").toBool() == true) {
            goto trueend;
        }

        callpeffects(player, player, "DetermineAttackPossible");
        if (turnMemory(player).value("ImpossibleToMove").toBool() == true) {
            goto trueend;
        }
    }

    turnMem(player).add(TM::HasPassedStatus);

    //Down here so it doesnt get overridden but still defines it before the announcement
    if (pokeMemory(player).value("ZMoveTurn") == turn()) {
        sendItemMessage(68, player);
        attack = ItemInfo::CrystalMove(poke(player).item());
        callieffects(player, player, "MoveSettings"); //Z Moves
    }

    turnMemory(player)["MoveChosen"] = attack;

    if (!specialOccurence) {
        callbeffects(player,player,"MovePossible");
        if (turnMemory(player)["ImpossibleToMove"].toBool()) {
            goto trueend;
        }

        callpeffects(player, player, "MovePossible");
        if (turnMemory(player).value("ImpossibleToMove").toBool()) {
            goto trueend;
        }
    }

    //Healing moves called with another move while under heal block are still blocked
    if (specialOccurence && pokeMemory(player).value("HealBlockCount").toInt() > 0) {
        callpeffects(player, player, "MovePossible");
        if (turnMemory(player).value("ImpossibleToMove").toBool()) {
            goto trueend;
        }
    }

    //Gen 3 Sleep Talk fails if the move selected has 0 pp
    if (specialOccurence && turnMemory(player).contains("SleepTalkedMove") && gen().num == 3) {
        for (int i = 0; i < 3; i++) {
            if (fpoke(player).moves[i] == turnMemory(player)["SleepTalkedMove"].toInt()) {
                if (PP(player, i) <= 0) {
                    notify(All, UseAttack, player, qint16(move));
                    notify(All, Failed, player);
                    goto trueend;
                }
            }
        }
    }

    if (!specialOccurence) {
        if (PP(player, move) <= 0) {
            notify(All, UseAttack, player, qint16(attack), !(tellPlayers && !turnMemory(player).contains("TellPlayers")));
            sendMoveMessage(123,1,player);
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
        if (pokeMemory(player).contains("OutrageMove")) {
            /* Have to set last move for disable to work on 2nd or 3rd turn*/
            pokeMemory(player)["LastMoveUsed"] = attack;
            pokeMemory(player)["LastMoveUsedTurn"] = turn();
        }
    }

    //For metronome calling fly / sky attack / ...
    if (attack != 0) {
        fpoke(player).lastMoveUsed = attack;
    }

    calleffects(player, player, "MoveSettings");

    //Sleep Talked moves should be tracked on tooltip. We use a new bool so PP isn't deducted from the tooltip.
    if (turnMemory(player).contains("SleepTalkedMove")) {
        special = false;
    }
    notify(All, UseAttack, player, qint16(attack), !(tellPlayers && !turnMemory(player).contains("TellPlayers")), special);

    calleffects(player, player, "AfterTellingPlayers");

    if (!specialOccurence) {
        if (turnMemory(player).value("PowderExploded").toBool()) {
            goto ppfunction;
        }
        if (turnMemory(player).value("ImpossibleToMove").toBool()) {
            goto trueend;
        }
    }

    //Follow Me takes priority over abilities
    callbeffects(player,player, "GeneralTargetChange");

    /* Lightning Rod & Storm Drain */
    foreach(int poke, sortedBySpeed()) {
        if (poke != player) {
            callaeffects(poke, player, "GeneralTargetChange");
        }
    }

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

ppfunction:
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
        if (turnMemory(player).value("PowderExploded").toBool()) {
            goto trueend;
        }
    }

    /* Choice items act before target selection if no target in gen 5 */
    callieffects(player, player, "BeforeTargetList");

    if (targetList.size() == 0) {
        notify(All, NoOpponent, player);
        goto end;
    }

    calleffects(player, player, "BeforeTargetList");
    //To prevent a Snatched move from changing type
    if (!(turnMemory(player).value("SkipProtean").toBool())) {
        callaeffects(player, player, "BeforeTargetList");
    }

    /* Choice item memory, copycat in gen 4 and less */
    if (!specialOccurence && attack != Move::Struggle) {
        battleMemory()["LastMoveUsed"] = attack;
    }

    foreach(int target, targetList) {
        //heatOfAttack() = true;
        attacked() = target;
        if (!specialOccurence && (tmove(player).flags & Move::MemorableFlag) ) {
            pokeMemory(target)["MirrorMoveMemory"] = attack;
        }

        turnMem(player).remove(TM::Failed);
        turnMem(player).add(TM::FailingMessage);

        if (tmove(player).type == Type::Water && tmove(player).power > 0 && isWeatherWorking(StrongSun)) {
            sendAbMessage(126, 6, player, player, TypeInfo::TypeForWeather(StrongSun),1);
            continue;
        }
        if (tmove(player).type == Type::Fire && tmove(player).power > 0 && isWeatherWorking(StrongRain)) {
            sendAbMessage(126, 7, player, player, TypeInfo::TypeForWeather(StrongRain),1);
            continue;
        }
        if (target != player && !testAccuracy(player, target)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }

        if (tmove(player).power > 0)
        {
            calculateTypeModStab();

            calleffects(player, target, "BeforeCalculatingDamage");
            /* For Focus Punch*/
            if (turnMemory(player).contains("LostFocus")) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            int typemod = turnMem(player).typeMod;
            if (typemod < -50) {
                /* If it's ineffective we just say it */
                notify(All, Effective, target, quint8(0));
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            if (target != player) {
                callaeffects(target,player,"OpponentBlock");
                callieffects(target,player,"OpponentBlock"); //Safety Goggles
                if (!isFlying(target) && terrainCount > 0 && terrain == Type::Psychic && tmove(player).priority > 0) {
                    sendMoveMessage(222,2,target,Type::Psychic);
                    calleffects(player,target,"AttackSomehowFailed");
                    continue;
                }
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

            //Moved after failure check to allow Sucker punch to work correctly.
            /* In gen 6, this check is after the "no effect" check. Since king's shield
             * on aegislash on a physical normal/fighting/poison attack doesn't reduce the opponent's
             * attack by two stages. */
            //fixme: try to get protect to work on a calleffects(target, player), and wide guard/priority guard on callteffects(this.player(target), player)
            /* Protect, ... */
            callbeffects(player, target, "DetermineGeneralAttackFailure", true);
            if (testFail(player)) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }
            int num = repeatNum(player);
            bool hit = num > 1;

            int hitcount = 0;
            bool hitting = false;
            for (repeatCount() = 0; repeatCount() < num && !koed(target) && (repeatCount()==0 || !koed(player)); repeatCount()+=1) {
                clearAtk();
                clearBp();
                //heatOfAttack() = true;
                fpoke(target).remove(BasicPokeInfo::HadSubstitute);
                bool sub = hasSubstitute(target);
                if (sub) {
                    fpoke(target).add(BasicPokeInfo::HadSubstitute);
                }

                /*Gyro Ball needs a special exclusion here to display tooltips correctly because BP is still registered as "1" through the next 2 checks.
                 * The actual BP isn't calculated until after a crit is determined otherwise the incorrect speed stat and modifiers are used.*/
                if ((tmove(player).power > 1  || tmove(player).attack == Move::GyroBall) && repeatCount() == 0) {
                    notify(All, Effective, target, quint8(typemod > 0 ? 8 : (typemod < 0 ? 2 : 4)));
                }

                if (tmove(player).power > 1 || tmove(player).attack == Move::GyroBall) {
                    calleffects(player, target, "BeforeHitting");
                    callaeffects(player, target, "ActivateProtean");
                    if (turnMemory(player).contains("HitCancelled")) {
                        turnMemory(player).remove("HitCancelled");
                        continue;
                    }
                    testCritical(player, target);
                    int damage = calculateDamage(player, target);
                    inflictDamage(target, damage, player, true);
                    hitcount += 1;
                    hitting = true;
                } else {
                    callaeffects(player, target, "ActivateProtean");
                    turnMemory(player).remove("CustomDamage");
                    calleffects(player, target, "CustomAttackingDamage");

                    if (turnMemory(player).contains("CustomDamage")) {
                        int damage = turnMemory(player).value("CustomDamage").toInt();
                        inflictDamage(target, damage, player, true);
                        hitcount += 1;
                        hitting = true;
                    }
                }

                calleffects(player, target, "UponAttackSuccessful");
                if (!hasSubstitute(target))
                    calleffects(player, target, "OnFoeOnAttack");

                healDamage(player, target);

                //heatOfAttack() = false;
                if (hitting) {
                    if (!sub) {
                        callaeffects(target, player, "UponBeingHit");
                        callaeffects(player, target, "OnHitting");
                    }
                    callaeffects(target, player, "UponOffensiveDamageReceived");
                    callieffects(target, player, "UponBeingHit");
                    /*This allows Knock off to work*/
                    calleffects(player, target, "KnockOff");
                    callieffects(target, player, "AfterKnockOff");
                }

                if (koed(target))
                    callaeffects(player, target, "AfterKoing");

                /* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
                /* In Gen 2, KOing a pokemon won't provide any beneficial boosts*/
                if (gen() > 2 || !koed(target)) {
                    applyMoveStatMods(player, target);
                }

                /* For berries that activate after taking damage */
                callaeffects(target, target, "TestPinch");
                callieffects(target, target, "TestPinch");

                if (!sub && !koed(target)) testFlinch(player, target);

                attackCount() += 1;

                if (poke(player).status() == Pokemon::Asleep && !turnMemory(player).value("SleepingMove").toBool()) {
                    break;
                }
            }

            // Triple Kick has an accuracy check for every attack, so it is possible that it only hits once.
            // Make sure that the hit message is still shown.
            if (hit || turnMemory(player).contains("RepeatCount")) {
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
            //fixme: try to get protect to work on a calleffects(target, player), and wide guard/priority guard on callteffects(this.player(target), player)
            /* Protect, ... */
            callbeffects(player, target, "DetermineGeneralAttackFailure", true);
            if (testFail(player)) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            /* Magic Coat, Magic Bounce */
            callbeffects(player, target, "DetermineGeneralAttackFailure2", true);
            if (testFail(player)) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }

            /* Needs to be called before opponentblock because lightning rod / twave */
            int type = tmove(player).type; /* move type */
            if (target != player) {
                bool fail = false;
                //Poison can't be poisoned regardless of Sleuthed status
                if (Move::StatusInducingMove && tmove(player).status == Pokemon::Poisoned && hasType(target, Type::Poison)) {
                    if (!hasWorkingAbility(player, Ability::Corrosion))
                        fail = true;
                } else if (!pokeMemory(target).value(QString("%1Sleuthed").arg(type)).toBool()) {
                    //RawTypeEffect is useless here because Inverted will never create an "ineffective" scenario.
                    if (Move::StatusInducingMove && tmove(player).status == Pokemon::Poisoned && hasType(target, Type::Steel)) {
                        if (!hasWorkingAbility(player, Ability::Corrosion))
                            fail = true;
                    } else if (attack == Move::ThunderWave) {
                        //Thunderwave is affected by immunities in all forms of battle
                        if (ineffective(rawTypeEff(type, target))) {
                            fail = true;
                        } else if (gen() >= 6 && hasType(target, Type::Electric) &&
                                   !(hasWorkingTeamAbility(target, Ability::Lightningrod) || hasWorkingAbility(target, Ability::VoltAbsorb) || hasWorkingAbility(target, Ability::MotorDrive))) {
                            fail = true;
                        }
                    }
                }

                if (fail) {
                    sendMoveMessage(31,0,target); //It doesn't affect X...
                    //notify(All, Failed, player);
                    continue;
                }
            }

            /* Needs to be called before DetermineAttackFailure because
              of SapSipper/Leech Seed */
            if (target != player) {
                callaeffects(target,player,"OpponentBlock");
                callieffects(target,player,"OpponentBlock"); //Safety Goggles
            }
            if (turnMemory(target).contains(QString("Block%1").arg(attackCount()))) {
                calleffects(player,target,"AttackSomehowFailed");
                continue;
            }
            if ( target != player && (tmove(player).flags & Move::PowderFlag) && hasType(target, Type::Grass) && !pokeMemory(target).value(QString("%1Sleuthed").arg(Type::Grass)).toBool()) {
                notify(All, Failed, player);
                continue;
            }
            callpeffects(player, target, "DetermineAttackFailure");
            if (testFail(player)) continue;

            //Type changing moves activate protean. We force it to check here so the actual move can fail if needed.
            if (attack == Move::Camouflage || attack == Move::Conversion || attack == Move::Conversion2) {
                callaeffects(player, target, "ActivateProtean");
            }
            calleffects(player, target, "DetermineAttackFailure");
            if (testFail(player)) continue;

            if (target != player && hasSubstitute(target) && !canBypassSub(player)) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
                continue;
            }

            calleffects(player, target, "BeforeHitting");
            callaeffects(player, target, "ActivateProtean");

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
        //Will-O-Wisp shouldn't thaw target. Scald thaws target in Gen 6. Hidden Power doesn't thaw before Gen 4
        if (poke(target).status() == Pokemon::Frozen) {
            if ((tmove(player).type == Type::Fire && tmove(player).power > 0 && !(gen().num < 4 && attack == Move::HiddenPower)) || (attack == Move::Scald && gen() >= 6)) {
                unthaw(target);
            }
        }
        pokeMemory(target)["LastAttackToHit"] = attack;
    }
end:
    /* In gen 4, choice items are there - they lock even if the move had no target possible.  */
    callieffects(player, player, "AfterTargetList");
trueend:
    heatOfAttack() = false;

    if (gen() <= 4 && koed(player) && tmove(player).power > 0) {
        notifyKO(player);
    }

    attacker() = oldAttacker;
    attacked() = oldAttacked;

    /* For U-TURN, so that none of the variables of the switchin are afflicted, it's put at the utmost end */
    calleffects(player, player, "AfterAttackFinished");
    foreach(int target, targetList) {
        callaeffects(target, target, "AfterAttackFinished"); //Immunity & such
        turnMemory(target)["HadSubstitute"] = false;
    }
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
    QVector<int> attackTypes = QVector<int>() << type;
    if (tmove(player).attack == Move::FlyingPress) {
        attackTypes.push_back(Type::Flying);
    }
    QVector<int> typeAdv = getTypes(target);
    QVector<int> typePok = getTypes(player);

    int typemod = 0;

    foreach(int type, attackTypes) {
        if (type == Type::Ground && hasFlyingEffect(target) && tmove(player).attack != Move::ThousandArrows) {
            typemod = -100;
            goto end;
        }
        foreach(int def, typeAdv) {
            if (tmove(player).attack == Move::FreezeDry && def == Type::Water) {
                typemod += 1;
                continue;
            }
            int typeeff = convertTypeEff(TypeInfo::Eff(type, def, gen()));

            if(clauses() & ChallengeInfo::Inverted){
                if(typeeff<-50){
                    typeeff = 1;
                }
                else {
                    typeeff *= -1;
                }
            }
            if (typeeff > 0) {
                // Delta Stream reduces SE effectiveness to Neutral on flying types
                if (def == Type::Flying && isWeatherWorking(StrongWinds)) {
                    typeeff -= 1;
                }
            }
            if (typeeff < -50) {
                /* Check for grounded flying types */
                if (type == Type::Ground) {
                    if (hasGroundingEffect(target)) {
                        continue;
                    } else if (tmove(player).attack == Move::ThousandArrows && def == Type::Flying) {
                        typemod = 0;
                        continue;
                    }
                }
                if (pokeMemory(target).value(QString::number(def)+"Sleuthed").toBool() || hasWorkingItem(target, Item::RingTarget)) {
                    continue;
                }
                /* Scrappy */
                if (hasType(target, Pokemon::Ghost) && hasWorkingAbility(player,Ability::Scrappy)) {
                    continue;
                }
                typemod = -100;
                goto end;
            }
            typemod += typeeff;
        }
    }
end:

    int stab = 2;

    foreach(int type, attackTypes) {
        foreach(int pok, typePok) {
            if (type == pok) {
                stab++;
            }
        }
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

    if (ability(player) != ab)
        return false;

    if (attacking()) {
        if (heatOfAttack() && player == attacked() && player != attacker() && AbilityInfo::moldBreakable(ability(player))) {
            int ab = ability(attacker());
            if (hasWorkingAbility(attacker(), ab)) {
                if (ab == Ability::MoldBreaker || ab == Ability::TeraVolt || ab == Ability::TurboBlaze) {
                   return false;
               }
            }
            int move = tmove(attacker()).attack;
            if (move == Move::MoongeistBeam || move == Move::SunsteelStrike) {
                sendMoveMessage(239,0,attacker(),0,player); //UNTESTED. Might look really bad
                return false;
            }
            if (move == Move::CoreEnforcer && hasMoved(attacker())) {
                sendMoveMessage(239,0,attacker(),0,player); //UNTESTED. Might look really bad
                return false;
            }
        }
    }
    return !pokeMemory(player).value("AbilityNullified").toBool();
}

bool BattleSituation::hasWorkingTeamAbility(int play, int ability, int excludedSlot)
{
    int p = player(play);
    for (int i = 0; i < numberPerSide(); i++) {
        int s = slot(p, i);
        if (s == excludedSlot) { //Unnerve. Checks if any team mate has the ability still
            continue;
        }
        if (!koed(s) && hasWorkingAbility(s, ability)) {
            return true;
        }
    }
    return false;
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
    callaeffects(slot, slot, "OnLoss");
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
    if (hasWorkingItem(player, Item::FloatStone)) {
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
            && !(ItemInfo::isBerry(poke(player).item()) && opponentsHaveWorkingAbility(player, Ability::Unnerve));
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

    if (recoil < 0) {
        if(repeatCount() >= repeatNum(source) - 1 || koed(target)) {
            notify(All, Recoil, source, true);
        }
    } else {
        notify(All, Recoil, target, false);
    }

    // "33" means one-third
    //if (recoil == -33) recoil = -100 / 3.; -- commented out until ingame confirmation

    int damage = recoil < 0 ? std::abs(int(recoil * turnMem(target).damageTaken / 100)):std::abs(int(recoil * turnMemory(source).value("LastDamageInflicted").toInt() / 100));

    if (recoil < 0) {
        if(repeatCount() >= repeatNum(source) - 1 || koed(target)) {
            inflictDamage(source, damage, source, false);

            /* Self KO Clause! */
            if (koed(source)) {
                /* In VGC 2011 (gen 5), the user of the recoil move wins instead of losing with the Self KO Clause */
                if (gen() <= 4)
                    selfKoer() = source;
            }
        }
    } else  {
        if (hasWorkingItem(source, Item::BigRoot)) /* Big root */ {
            damage = damage * 13 / 10;
        }

        if (hasWorkingAbility(target, Ability::LiquidOoze)) {
            if (gen().num < 5 && tmove(source).attack == Move::DreamEater) {
                if (canHeal(source,HealByMove,Move::DreamEater))
                    healLife(source, damage);
            } else if (!hasWorkingAbility(source, Ability::MagicGuard)){
                sendMoveMessage(1,2,source,Pokemon::Poison,target);
                inflictDamage(source,damage,source,false);
            }
        } else {
            if (canHeal(source,HealByMove,tmove(source).attack)) {
                healLife(source, damage);
            }
        }
    }
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    applyingMoveStatMods = true;
    bool sub = hasSubstitute(target) && !canBypassSub(player);

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
    //bool negativeStatChange = false; //Done Elsewhere

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

        signed char increase = char (fm.boostOfStat >> (i*8));

        int rate = char (fm.rateOfStat >> (i*8));

        if (increase < 0 && target != player && sub) {
            if (rate == 0 && cl != Move::OffensiveStatusInducingMove) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
            }
            applyingMoveStatMods = false;
            return;
        }

        /* Serene Grace, Rainbow */
        //Serene Grace does not affect Secret Power
        if ((hasWorkingAbility(player,Ability::SereneGrace) && tmove(player).attack != Move::SecretPower) || teamMemory(this->player(target)).value("RainbowCount").toInt() > 0) {
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
            //negativeStatChange = true; //Done Elsewhere
        }

        statChange = true;
    }

    if (statChange == true) {
        callieffects(target, player, "AfterStatChange");
        callaeffects(target, player, "AfterStatChange");
        //Done Elsewhere
        /*if (target != player && negativeStatChange && gen() >= 5) {
            callaeffects(target, player, "AfterNegativeStatChange");
        }*/
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
    //Serene Grace does not affect Secret Power
    if ((hasWorkingAbility(player,Ability::SereneGrace) && tmove(player).attack != Move::SecretPower) || teamMemory(this->player(player)).value("RainbowCount").toInt() > 0) {
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

bool BattleSituation::canGetStatus(int target, int status, int inflicter) {
    if (hasWorkingAbility(target, Ability::LeafGuard) && isWeatherWorking(Sunny) && !(tmove(target).attack == Move::Rest && gen().num == 4))
        //Gen 4 allows the use of Rest with working Leaf Guard.
        return false;
    if (!isFlying(target) && terrainCount > 0 && terrain == Type::Fairy) {
        //Rest, 2nd part of Yawn, Status Orbs, Effect Spore, Flame Body, Poison Point, Psycho Shift
        return false;
    }
    if (hasWorkingAbility(target, Ability::Comatose) && status != Pokemon::Asleep) {
        sendAbMessage(148,0,target);
        return false;
    }

    //Veils
    for (int i_ = 0; i_ < numberPerSide(); i_++) {
        int i = slot(player(target), i_);

        if (!koed(i) && hasWorkingAbility(i, poke(i).ability())) {
            if (pokeMemory(i)["AbilityArg"].toString().startsWith("Veil_")) {
                if (hasType(target,pokeMemory(i)["AbilityArg"].toString().mid(5).toInt())) {
                    sendAbMessage(104,1,target,i,pokeMemory(i)["AbilityArg"].toString().mid(5).toInt(),ability(i));
                    return false;
                }
            }
        }
    }
    switch (status) {
    case Pokemon::Asleep: {
        if (hasWorkingAbility(target, Ability::Insomnia) || hasWorkingAbility(target, Ability::VitalSpirit)) {
            return false;
        }
        if (hasWorkingTeamAbility(target, Ability::SweetVeil)){
            sendAbMessage(112,0,target);
            return false;
        }
        if (isThereUproar()) {
            //sendMoveMessage(141,4,target); //Needs to remove "But it failed" from Rest first
            return false;
        }
        if (!isFlying(target) && terrainCount > 0 && terrain == Type::Electric) {
            sendMoveMessage(201,3,target);
            return false;
        }
        return true;
    }
    case Pokemon::Burnt: {
        if (!hasType(target, Pokemon::Fire) || hasWorkingAbility(target, Ability::WaterVeil) || hasWorkingAbility(target, Ability::WaterBubble)) {
            return false;
        }
        return true;
    }
    case Pokemon::Frozen: {
        if (hasType(target, Pokemon::Ice) || hasWorkingAbility(target, Ability::MagmaArmor)) {
            return false;
        }
        if (isWeatherWorking(Sunny) || isWeatherWorking(StrongSun)) {
            return false;
        }
        return true;
    }
    case Pokemon::Paralysed: {
        if (gen() >= 6 && hasType(target, Type::Electric)) {
            return false;
        }
        if (hasWorkingAbility(target, Ability::Limber)) {
            return false;
        }
        return true;
    }
    case Pokemon::Poisoned: {
        if (hasWorkingAbility(target, Ability::Immunity)) {
            return false;
        }
        //Unconfirmed: As far as we know, Corrosion only allows poisoning Steels and Poison types. Should it bypass other abilities too? (Comatose, Immunity, etc.)
        //If so, add "&& status == Pokemon::Poisoned" to the conditional and move it to the correct placing (aka, the top if it bypasses everything)
        if (inflicter != target && hasWorkingAbility(inflicter, Ability::Corrosion)) {
            return true;
        }
        if ((gen() > 2 && hasType(target, Pokemon::Steel)) || hasType(target, Pokemon::Poison)) {
            return false;
        }
        return true;
    }
    default:
        return false;
    }
}

bool BattleSituation::inflictStatMod(int player, int stat, int mod, int attacker, bool tell, bool *negative)
{
    bool pos = (mod > 0) ^ hasWorkingAbility(player, Ability::Contrary);
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
        callbeffects(player, attacker, "PreventStatChange");
        if (turnMemory(player).contains(q)) {
            return false;
        }

        if(teamMemory(this->player(player)).value("MistCount").toInt() > 0 && (!hasWorkingAbility(attacker, Ability::Infiltrator) || this->player(player) == this->player(attacker))) {
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
            callaeffects(player, attacker, "AfterStatChange");
        }
        if (player != attacker) {
            callaeffects(player, attacker, "AfterNegativeStatChange");
        }
    } else {
        notify(All, CappedStat, player, qint8(stat), false);
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
            if (!hasWorkingAbility(attacker, Ability::Infiltrator) || this->player(player) == this->player(attacker)) {
                sendMoveMessage(109, 2, player,Pokemon::Psychic, player, tmove(player).attack);
                return;
            }
        }
        if(std::abs(terrain) == Type::Fairy && terrainCount > 0 && !isFlying(player)) {
            sendMoveMessage(208, 2, player,Pokemon::Fairy, player, tmove(player).attack);
            return;
        }
    }

    if (!canGetStatus(player, status, attacker))
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
    if (status == Pokemon::Frozen && poke(player).num() == Pokemon::Shaymin_Sky) {
        changeForme(this->player(player), slotNum(player), Pokemon::Shaymin);
    }
    if (attacker != player && status != Pokemon::Asleep && status != Pokemon::Frozen && poke(attacker).status() == Pokemon::Fine && canGetStatus(attacker,status,player)
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
                callieffects(i,i,"WeatherSpecial");
                if (!turnMemory(i).contains("WeatherSpecialed") && (weather == Hail || weather == SandStorm) && getTypes(i).toList().toSet().intersect(immuneTypes).isEmpty()
                        && !hasWorkingAbility(i, Ability::MagicGuard)) {
                    notify(All, WeatherMessage, i, qint8(HurtWeather),qint8(weather));

                    //In GSC, the damage is 1/8, otherwise 1/16
                    inflictDamage(i, poke(i).totalLifePoints()*(gen() > 2 ? 1 : 2)/16, i, false);
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

int BattleSituation::getType(int player, int slot) const
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

QVector<int> BattleSituation::getTypes(int player, bool transform) const
{
    QVector<int> ret;

    foreach(int type, fpoke(player).types) {
        if (type == Pokemon::Flying && pokeMemory(player).value("Roosted").toBool() && !transform) {
            continue;
        }
        if (type == Pokemon::Curse) {
            continue;
        }
        ret.push_back(type);
    }

    if (ret.isEmpty()) {
        //If a pokemon was pure fire and lost its type via burn up, it takes neutral damage from all moves
        if (pokeMemory(player).value("BurnedUp").toBool()) {
            ret.push_back(Type::Curse);
        } else {
            ret.push_back(Type::Normal);
        }
    }

    return ret;
}

void BattleSituation::setType(int player, int type)
{
    fpoke(player).types.clear();
    fpoke(player).types.push_back(type);
}

void BattleSituation::addType(int player, int type)
{
    foreach (int t, fpoke(player).types) {
        if (t == type) {
            return;
        }
    }
    fpoke(player).types.push_back(type);
}

void BattleSituation::removeType(int player, int type)
{
    int t = fpoke(player).types.indexOf(type);
    if (t != -1) {
        fpoke(player).types.remove(t);
    }
}

int BattleSituation::rawTypeEff(int atttype, int player)
{
    int typemod = 0;

    foreach(int deftype, fpoke(player).types) {
        int eff = TypeInfo::Eff(atttype, deftype);
        if(clauses() & ChallengeInfo::Inverted){
            switch(eff){
                case Type::Ineffective:
                    eff=Type::SuperEffective;
                    break;
                case Type::NotEffective:
                    eff=Type::SuperEffective;
                    break;
                case Type::SuperEffective:
                    eff=Type::NotEffective;
                    break;
                default:
                    eff=Type::Effective;
                    break;
            }
        }
        typemod += convertTypeEff(eff);
    }

    return typemod;
}

PokeFraction BattleSituation::effFraction(int typeeff)
{
    if (typeeff < 0) {
        return PokeFraction(1,intpow2(-typeeff));
    }
    return PokeFraction(intpow2(typeeff), 1);
}

bool BattleSituation::isFlying(int player, bool levi)
{
    return hasFlyingEffect(player, levi) ||
            (!hasGroundingEffect(player) && (!attacking() || !hasWorkingItem(player, Item::RingTarget)) && hasType(player, Pokemon::Flying));
}

bool BattleSituation::hasFlyingEffect(int player, bool levi)
{
    return !hasGroundingEffect(player)  &&
            ( (hasWorkingAbility(player, Ability::Levitate) && levi)
             || hasWorkingItem(player, Item::AirBalloon)
             || pokeMemory(player).value("MagnetRiseCount").toInt() > 0
             || pokeMemory(player).value("LevitatedCount").toInt() > 0);
}

bool BattleSituation::hasGroundingEffect(int player)
{
    return battleMemory().value("Gravity").toBool() || hasWorkingItem(player, Item::IronBall)
            || (gen() >= 3 && pokeMemory(player).value("Rooted").toBool()) || pokeMemory(player).value("SmackedDown").toBool();
}

bool BattleSituation::isProtected(int slot, int target)
{
    /* check to see if protected. order is important: wide guard > endure > protect */
    if ((teamMemory(player(slot)).value("WideGuardUsed", -1).toInt() == turn()
         && (tmove(target).targets == Move::Opponents
             || tmove(target).targets == Move::All
             || tmove(target).targets == Move::AllButSelf))) {
        return true;
    }

    /*Endure shares code with Protect, but it is not considered "protected" as far as abilities go*/
    if (turnMemory(slot).value("CannotBeKoed").toBool()) {
        return false;
    }

    if (pokeMemory(slot).value("ProtectiveMoveTurn", -1).toInt() == turn()) {
        return true;
    }

    return false;
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
        if (status == Pokemon::Asleep) {
            poke(player).oriStatusCount() = poke(player).statusCount() + poke(player).advSleepCount();
        }
    }
    else if (status == Pokemon::Asleep) {
        if (gen() <= 2) {
            poke(player).statusCount() = 1 + (randint(6));
        } else if (gen() <= 4) {
            poke(player).statusCount() = 1 + (randint(4));
            //Variable cleared when put to sleep again
            poke(player).advSleepCount() = 0;
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

void BattleSituation::debug(const QString &message)
{
    battleChat(conf.ids[0], message);
}

bool BattleSituation::canSendPreventMessage(int defender, int attacker) {
    //Message needs to show with Intimidate, et al. No attacking() check possible.
    return attacker != defender && (!turnMemory(defender).contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
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

// This is how you chain two modifiers/multipliers together
int BattleSituation::chainMod(int mod1, int mod2) {
    if (!mod1) return mod2;
    if (!mod2) return mod1;
    return (mod1 * mod2 + 0x800) >> 12;
}
// This is for when you apply the modifier/multiplier to stats, move power, and damage
int BattleSituation::applyMod(int num, int mod) {
    if (mod == 0) {
        mod = 0x1000; //In case something is undefined we don't want to ruin everything. 0x1000 is inert.
    }
    float temp = num * mod / (float) 0x1000;
    // round the result half down
    // e.x. 2.3 round to 2, 2.5 rounds to 2, 2.6 rounds to 3
    if (temp - (int) temp <= 0.5) {
        return (int) temp;
    }
    return 1 + (int) temp;
}

//For gen 3 and 4
int BattleSituation::floorMod(int num) {
    for (int i = 0; i < bpmodifiers.size(); i++) {
        num = num * (20 + bpmodifiers[i]) / 20;
    }
    clearBp();
    return num;
}

int BattleSituation::calculateDamage(int p, int t)
{
    PokeBattle &poke = this->poke(p);
    int level = fpoke(p).level;
    int attack, def;
    bool crit = turnMem(p).contains(TM::CriticalHit);
    int attackused = tmove(p).attack;
    int cat = tmove(p).category;
    int type = tmove(p).type;
    int randnum = randint(16) + 85;
    if (attackused == Move::SpitUp && gen() < 4) {
        randnum = 100;
    }
    if (gen().num == 2) {
        calleffects(p, t, "DamageFormulaStart");

        QString qA, qD;
        if (cat == Move::Physical) {
            attack = getStat(p, Attack, 1);
            qA = "Stat"+QString::number(Attack);
            def = getStat(t, Defense, 1);
            qD = "Stat"+QString::number(Defense);
            if ((poke.status() == Pokemon::Burnt || turnMemory(p).contains("WasBurned"))
                && !(crit && turnMemory(p).value("CritIgnoresAll").toBool())) {
                attack = std::max(1, attack / 2);
            }
        } else {
            attack = getStat(p, SpAttack, 1);
            qA = "Stat"+QString::number(SpAttack);
            def = getStat(t, SpDefense, 1);
            qD = "Stat"+QString::number(SpDefense);
        }

        if (attackused == Move::Present && gen() != Pokemon::gen(Gen::Stadium2)) {
            // In GSC, a glitch causes the level, Attack, and Defense variables to be replaced.
            // Attack will be replaced with 10, level will be based on the index number of the defending Pokémon's type.
            //Defense will be based on the index number of the attacking Pokémon's type.
            // If a Pokémon has two types, its secondary type will be used.

            // Index numbers:
            // 0 = Normal
            // 1 = Fighting
            // 2 = Flying
            // 3 = Poison
            // 4 = Ground
            // 5 = Rock
            // 7 = Bug
            // 8 = Ghost
            // 9 = Steel
            // 20 = Fire
            // 21 = Water
            // 22 = Grass
            // 23 = Electric
            // 24 = Psychic
            // 25 = Ice
            // 26 = Dragon
            // 27 = Dark
            
            attack = 10;
            int typeConversions[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 20, 21, 22, 23, 24, 25, 26, 27};
            def = typeConversions[getType(p, 2)];
            level = typeConversions[getType(t, 2)];

        }

        if (!(crit && turnMemory(p).value("CritIgnoresAll").toBool())
            && teamMemory(this->player(t)).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            def *= 2;
        }

        callieffects(p, p, "StatModifier");
        callieffects(t, t, "StatModifier");

        // Thick Club and Light Ball
        attack = attack * (20 + turnMemory(p).value(qA+"ItemModifier").toInt()) / 20;

        if (gen() != Pokemon::gen(Gen::Stadium2)) {
            if (attack > 255 || def > 255) { // Stat Scaling 1
                attack = (attack / 4) % 256;
                def = (def / 4) % 256;
                if (def == 0) {
                    def = 1;
                }
            }
        }

        // Metal Powder
        if (turnMemory(t).value(qD+"ItemModifier").toInt() > 0) {
            def = def * (20 + turnMemory(t).value(qD+"ItemModifier").toInt()) / 20;
            if (gen() != Pokemon::gen(Gen::Stadium2)) {
                if (def > 255) { // Stat Scaling 2
                    attack = (attack / 2) % 256;
                    def = (def / 2) % 256;
                    if (def == 0) {
                        def = 1;
                    }
                }
            }
        }

        turnMemory(p).remove(qA+"ItemModifier");
        turnMemory(t).remove(qD+"ItemModifier");

        if (attackused == Move::Explosion || attackused == Move::Selfdestruct) {
            /* Explosion / Selfdestruct */
            def /= 2;
            if (def == 0) { // prevent division by zero
                def = 1;
            }
        }

        calleffects(p, t, "BasePowerModifier");
        callieffects(p, t, "BasePowerModifier");

        int power = tmove(p).power;
        int type = tmove(p).type;

        /* This is for Dig and Fly */
        if (pokeMemory(t).contains("VulnerableMoves") && pokeMemory(t).value("Invulnerable").toBool()) {
            QList<int> vuln_moves = pokeMemory(t)["VulnerableMoves"].value<QList<int> >();
            QList<int> vuln_mults = pokeMemory(t)["VulnerableMults"].value<QList<int> >();

            for (int i = 0; i < vuln_moves.size(); i++) {
                if (vuln_moves[i] == attackused) {
                    power = power * vuln_mults[i];
                }
            }
        }

        /* "The math buffer can store up to 32-bit numbers even though the final result must fit in
         *  16 bits. The multiplicand can be as high as a 24-bit number anytime and the multiplier
         *  as high as 255, so you can calc up to 1677215 * 255 for example." ~Crystal_
         *  42 * 255 * 255 (max value for damage before dividing) < 1677215 * 255
         *  2731050 < 427689825
         *  So we don't ever need to worry about capping out.
         *  Note: We used 255 as attack and defense were scaled to an 8-bit int if they weren't already.
         *        Power is always less than 255 so that's fine too.
         */
        int damage = (int)((2 * level / 5 + 2) * (long)power * attack / def / 50);

        if (crit) {
            damage *= 2;
        }

        callieffects(p, t, "Mod2Modifier");
        damage = damage * (turnMemory(p).value("ItemMod2Modifier").toInt() + 10) / 10; // item boosts for damage
        turnMemory(p).remove("ItemMod2Modifier");

        damage = std::min(997, damage) + 2;

        // weather goes here
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
        if (attackused == Move::SolarBeam && turnMemory(p).value("SolarbeamDamageReduction").toBool()) { // Solarbeam affects damage, not base power
            damage /= 2;
        }

        damage = damage * turnMem(p).stab / 2;

        // Type effectiveness
        int typemod = turnMem(p).typeMod;
        while (typemod > 0) {
            damage *= 2;
            typemod--;
        }

        if (attackused == Move::Present && gen() != Pokemon::gen(Gen::Stadium2) && typemod < 0) // Present only inflicts a quarter of the normal damage against Rock and Steel-type Pokémon.
            typemod--;

        while (typemod < 0) {
            damage /= 2;
            typemod++;
        }

        int randnum = randint(39) + 217; // remember that randint is 0 to n-1
        if (attackused == Move::Flail || attackused == Move::Reversal) {
            // these moves are proven to ignore randomization
            randnum = 255;
        }
        damage = damage * randnum / 255;

        if (attackused == Move::Pursuit && turnMemory(t).value("PursuitedOnSwitch").toBool()) { // pursuit affects damage, not base power
            damage *= 2;
        }

        turnMemory(t)["FinalModifier"] = 0;
        return damage;
    }

    /*This stuff is the same between gens 3, 4, and 5+ */
    callaeffects(p,t,"DamageFormulaStart");
    callaeffects(t,p,"FoeDamageFormulaStart");
    calleffects(p,t,"DamageFormulaStart");

    context &move = turnMemory(p);
    if (cat == Move::Physical) {
        attack = getStat(p, Attack);
        def = getStat(t, Defense);
    } else if (attackused == Move::Psyshock || attackused == Move::Psystrike || attackused == Move::SecretSword) {
        attack = getStat(p, SpAttack);
        def = getStat(t, Defense);
    } else {
        attack = getStat(p, SpAttack);
        def = getStat(t, SpDefense);
    }
    if (gen() < 5) {
        // *** WARNING: ORDER MATTERS ***/
        if (gen() < 4) {
            //Thick Fat is attack mod in gen 3, BP in gen 4
            for (int i = 0; i < atkmodifiers.size(); i++) {
                attack = attack * (20 + atkmodifiers[i]) / 20;
            }
        }
        /* Explosion / Selfdestruct Defense Halving */
        if (attackused == Move::Explosion || attackused == Move::Selfdestruct) {
            def = std::max(def/2, 1);
        }
        /*** Power ***/
        int power = tmove(p).power;
        /* Earthquake + Dig, Surf + Dive */
        /* Calculate the multiplier for two turn attacks */
        if (pokeMemory(t).contains("VulnerableMoves") && pokeMemory(t).value("Invulnerable").toBool()) {
            QList<int> vuln_moves = pokeMemory(t)["VulnerableMoves"].value<QList<int> >();
            QList<int> vuln_mults = pokeMemory(t)["VulnerableMults"].value<QList<int> >();
            for (int i = 0; i < vuln_moves.size(); i++) {
                if (vuln_moves[i] == attackused) {
                    power *= vuln_mults[i];
                }
            }
        }

        /* Helping Hand, Gen 4 is Power. Gen 3 is Damage */
        if (gen() > 3 && move.contains("HelpingHanded")) {
            power = power * 3 / 2;
        }

        /* Move *///Moves: Facade, Brine
        calleffects(p,t,"BasePowerModifier");
        power = floorMod(power);

        /* Item *///Items: Muscle Band, Wise Glasses, Type boosting items, Adamant/Lustrous/Griseous Orb
        callieffects(p,t,"BasePowerModifier");
        power = floorMod(power);

        /* Charge */// Can't be called via ChainBP else it will be out of order
        callpeffects(p, t, "BasePowerModifier");
        if (move.contains("Charged")) {
            power *= 2;
        }

        /* Sports */
        QString sport = "Sported" + QString::number(type);
        if (battleMemory().contains(sport) && pokeMemory(battleMemory()[sport].toInt()).value(sport).toBool()) {
            power /= 2;
        }

        /* User Ability *///Abilities: Rivalry, Reckless, Iron Fist, Blaze/et al., Technician
        callaeffects(p,t,"BasePowerModifier");
        power = floorMod(power);

        /* Foe Ability *///Foe Abilities: Thick Fat, Heatproof, Dry Skin
        callaeffects(t,p,"BasePowerFoeModifier");
        power = floorMod(power);

        int damage;
        if (gen().num == 4) {
            damage = ((level * 2 / 5 + 2) * power * attack / 50 / def);
        } else {
            damage = ((level * 2 / 5 + 2) * power * attack / def / 50);
        }

        /*** MOD 1 ***/
        /*Apply burn mods */
        if ((poke.status() == Pokemon::Burnt || turnMemory(p).contains("WasBurned"))
            && cat == Move::Physical && !hasWorkingAbility(p,Ability::Guts)) {
            damage /= 2;
        }

        /* Reflect, Light Screen */
        if (!crit && teamMemory(this->player(t)).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            if (!multiples()) {
                damage /= 2;
            } else {
                damage /= 3;
            }
        }
        /* Multiples */
        if (multiples() && targetList.size() > 1) {
            damage = damage * 3 / 4;
        }
        /* Weather */
        if (isWeatherWorking(Sunny)) {
            if (type == Type::Fire) {
               damage = damage * 3 / 2;
            } else if (type == Type::Water) {
                damage /= 2;
            }
        } else if (isWeatherWorking(Rain)) {
            if (type == Type::Water) {
                damage = damage * 3 / 2;
            } else if (type == Type::Fire) {
                damage /= 2;
            }
        }

        /* Flash Fire */
        if (type == Type::Fire && pokeMemory(p).contains("FlashFired") && hasWorkingAbility(p, Ability::FlashFire)) {
            damage = damage * 3 / 2;
        }
        /*** MOD 1 End ***/

        /* Add 2 */
        damage = std::max(1, damage) + 2;

        /* Apply Crit */
        if (crit) {
            damage *= 2 + hasWorkingAbility(p, Ability::Sniper);
        }

        /* Helping Hand, Gen 4 is Power. Gen 3 is Damage */
        if (gen() < 4 && move.contains("HelpingHanded")) {
            damage = damage * 3 / 2;
        }

        /*** MOD 2 ***/ //Aka: Gen 4
        /* Life Orb, Metronome */
        move.remove("ItemMod2Modifier");
        callieffects(p,t,"Mod2Modifier");
        int itemmod = move["ItemMod2Modifier"].toInt();
        if (itemmod != 0) {
            damage = damage * (20 + itemmod) / 20;
        }
        /* Me First */
        if (move.contains("MeFirstAttack")) {
            damage = damage * 3 / 2;
        }
        /*** MOD 2 End ***/

        /* Apply Random Factor */
        damage = damage * randnum / 100;

        /* Apply STAB mod, round down */
        int stab = turnMem(p).stab;
        damage = damage * stab / 2;

        /* Apply type mods */
        int typemod = turnMem(p).typeMod;
        while (typemod > 0) {
            damage *= 2;
            typemod--;
        }
        while (typemod < 0) {
            damage /= 2;
            typemod++;
        }

        /*** MOD 3 ***/ //Aka: Gen 4
        /* Solid Rock & Filter */
        if (turnMem(p).typeMod > 0 && (hasWorkingAbility(t,Ability::Filter) || hasWorkingAbility(t,Ability::SolidRock) || hasWorkingAbility(t,Ability::PrismArmor))) {
            damage = damage * 3 / 4;
        }
        /* Expert Belt */
        if (turnMem(p).typeMod > 0 && hasWorkingItem(p, Item::ExpertBelt)) {
            damage = damage * 6 / 5;
        }
        /* Tinted Lens */
        if (turnMem(p).typeMod < 0 && hasWorkingAbility(p, Ability::TintedLens)) {
            damage *= 2;
        }
        /* Damage reducing Berries */
        move.remove("Mod3Berry");
        callieffects(t, p, "Mod3Items");
        int berrymod = turnMemory(p).value("Mod3Berry").toInt();
        if (berrymod != 0) {
            damage = damage * (20 + berrymod) / 20;
        }
        /*** MOD 3 End ***/
        return std::max(damage, 1); //no 0 damage in gen 3 & 4
    }
    else {
        /* Used by Pledges to use a special attack, the sum of both */
        if (move.contains("AttackStat")) {
            attack = move.value("AttackStat").toInt();
            move.remove("AttackStat");
        }
        int chainedMods = 0x1000;
        for (int i = 0; i < atkmodifiers.size(); i++) {
            chainedMods = chainMod(chainedMods, atkmodifiers[i]);
        }
        /* Flash Fire */
        if (type == Type::Fire && pokeMemory(p).contains("FlashFired") && hasWorkingAbility(p, Ability::FlashFire)) {
            chainedMods = chainMod(chainedMods, 0x1800);
        }
        attack = applyMod(attack, chainedMods);
        attack = std::min(attack, 65535);


        /*** WARNING ***/
        /* The peculiar order here is caused by the fact that helping hand applies before item boosts,
          but item boosts are decided (not applied) before acrobat, and acrobat needs to modify
          move power (not just power variable) because of technician which relies on it */
        calleffects(p,t,"BasePowerModifier");
        callieffects(p,t,"BasePowerModifier");
        /* Gems */
        if (turnMemory(p).value("GemActivated").toBool()) {
            gen() < 6 ? chainBp(p, 0x1800) : chainBp(p, 0x14CD);
        }
        /* The Acrobat thing is here because it's supposed to activate after gem Consumption */
        if (attackused == Move::Acrobatics && poke.item() == Item::NoItem) {
            tmove(p).power *= 2;
        }
        int power = tmove(p).power;
        callaeffects(p,t,"BasePowerModifier");
        callaeffects(t,p,"BasePowerFoeModifier");

        /* Helping Hand */
        if (move.contains("HelpingHanded")) {
            power = applyMod(power, 0x1800);
        }
        /*Sports */
        if (gen() < 6) {
            QString sport = "Sported" + QString::number(type);
            if (battleMemory().contains(sport) && pokeMemory(battleMemory()[sport].toInt()).value(sport).toBool()) {
                power = applyMod(power, 0x548);
            }
        } else {
            QString sport = "SportedEnd" + QString::number(type);
            if (battleMemory().contains(sport) && battleMemory().value(sport).toInt() > turn()) {
                power = applyMod(power, 0x548);
            }
        }
        /* The following are included here
         * Ability Self: Technician, Iron Fist, Reckless, Sand Force, Sheer Force, Analytic, Toxic Boost, Flare Boost
         * Ability Target: Dry Skin, Heatproof, Rivalry
         * Item: Adamant Orb, Grseous Orb, Lustrous Orb, Type boosting items
         * Move: Knock Off, Brine, Me First, Charge, Solarbeam, SmellingSalts/Venoshock, Retaliate, Facade
         */
        callpeffects(p, t, "BasePowerModifier"); //for charge
        chainedMods = 0x1000;
        for (int i = 0; i < bpmodifiers.size(); i++) {
            chainedMods = chainMod(chainedMods, bpmodifiers[i]);
        }
        power = applyMod(power, chainedMods);
        int oppPlayer = this->player(t);
        int damage = std::min((level * 2 / 5 + 2) * std::min(power, 65535), 65535) * attack / def / 50 + 2;

        /* Apply multi-target mod */
        if (multiples() && targetList.size() > 1) {
            damage = applyMod(damage, 0xC00);
        }

        /* Apply weather mod*/
        if (isWeatherWorking(Sunny) || isWeatherWorking(StrongSun)) {
            if (type == Type::Fire) {
               damage = applyMod(damage, 0x1800);
            } else if (type == Type::Water) {
                damage = applyMod(damage, 0x800);
            }
        } else if (isWeatherWorking(Rain) || isWeatherWorking(StrongRain)) {
            if (type == Type::Water) {
                damage = applyMod(damage, 0x1800);
            } else if (type == Type::Fire) {
                damage = applyMod(damage, 0x800);
            }
        }

        /* Apply Terrain mods, no idea if in the right spot */
        if (std::abs(terrain) == Type::Fairy && type == Type::Dragon && !isFlying(oppPlayer)) {
            damage = applyMod(damage, 0x800);
        }
        if (std::abs(terrain) == Type::Grass && (attackused == Move::Bulldoze || attackused == Move::Earthquake || attackused == Move::Magnitude)) {
            damage = applyMod(damage, 0x800);
        }
        //Terrains boost moves of same type
        if (terrainCount > 0 && terrain > 0 && terrain < Type::Curse && terrain == type && !isFlying(p)) {
            damage = applyMod(damage, 0x1800);
        }

        /* Apply CH Mod, round down */
        if (crit) {
            damage = damage * 3 / 2;
        }

        /* Apply random factor, round down */
        damage = damage * randnum / 100;

        /* Apply STAB mod, round down */
        int stab = turnMem(p).stab;
        damage = damage * stab / 2;

        /* Apply type mods */
        int typemod = turnMem(p).typeMod;
        while (typemod > 0) {
            damage *= 2;
            typemod--;
        }
        while (typemod < 0) {
            damage /= 2;
            typemod++;
        }

        /*Apply burn mods */
        damage /= (((poke.status() == Pokemon::Burnt || turnMemory(p).contains("WasBurned")) && cat == Move::Physical && !hasWorkingAbility(p,Ability::Guts)
                     && !(gen() >= 6 && attackused == Move::Facade)) ? 2 : 1);

        // in Gen 5 the rounding is done before finalmods are applied, so it is possible to deal 0 damage
        if (gen() == 5 && damage < 1)
            damage = 1;

        /* Final Mods section*/
        /* Correct Order:
         * 1. Moves:     Reflect, Light Screen
         * 2. Abilities: Multiscale, Shadow Shield, Tinted Lens, Friend Guard, Sniper, Solid Rock, Filter
         * 3. Items:     Metronome, Expert Belt, Life Orb, Damage Reducing berry
         * 4. Combos:    Stomp+Minimize, Earthquake+Dig, Surf+Dive, Steamroller+Minimize
         */
        int finalmod = 0x1000;
        //*** 1 ***//
        /* Reflect, Light Screen */
        if (!crit && !hasWorkingAbility(p, Ability::Infiltrator) && teamMemory(this->player(t)).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
            if (!multiples()) {
                finalmod = chainMod(finalmod, 0x800);
            } else {
                finalmod = chainMod(finalmod, 0xA8F);
            }
        }
        //*** 2 ***//
        /* Multiscale, Shadow Shield */
        if (this->poke(t).isFull() && (hasWorkingAbility(t, Ability::MultiScale) || hasWorkingAbility(t, Ability::ShadowShield))) {
            finalmod = chainMod(finalmod, 0x800);
        }
        /* Tinted Lens, Stakeout */
        if ((turnMem(p).typeMod < 0 && hasWorkingAbility(p, Ability::TintedLens))
            || (hasWorkingAbility(p, Ability::Stakeout) && slotMemory(slot(t)).value("SwitchTurn").toInt() == turn())) {
            finalmod = chainMod(finalmod, 0x2000);
        }
        /* Friend Guard */
        for (int i = 0; i < numberPerSide(); i++) {
            int sl = slot(oppPlayer, i);
            if (!koed(sl) && sl != t && hasWorkingAbility(sl, Ability::FriendGuard)) {
                finalmod = chainMod(finalmod, 0xC00);
            }
        }
        /* Sniper */
        if (crit && hasWorkingAbility(p, Ability::Sniper)) {
            finalmod = chainMod(finalmod, 0x1800);
        }
        /* Solid Rock, Filter */
        if (turnMem(p).typeMod > 0 && (hasWorkingAbility(t,Ability::Filter) || hasWorkingAbility(t,Ability::SolidRock))) {
            finalmod = chainMod(finalmod, 0xC00);
        }
        //*** 3 ***//
        /* Expert Belt */
        if (turnMem(p).typeMod > 0 && hasWorkingItem(p, Item::ExpertBelt)) {
            finalmod = chainMod(finalmod, 0x1333);
        }
        /* Metronome, Life Orb */
        move.remove("ItemMod2Modifier");
        callieffects(p,t,"Mod2Modifier");
        int itemmod = move["ItemMod2Modifier"].toInt();
        if (itemmod != 0) {
            finalmod = chainMod(finalmod, itemmod);
        }
        /* Damage reducing Berries */
        move.remove("Mod3Berry");
        callieffects(t, p, "Mod3Items");
        int berrymod = turnMemory(p).value("Mod3Berry").toInt();
        if (berrymod != 0) {
            finalmod = chainMod(finalmod, berrymod);
        }
        //*** 4 ***//
        /* Stomp + Minimize, Steamroller + Minimize. Phantom/Shadow Force get this boost too */
        if (pokeMemory(t).value("Minimize").toBool() && (attackused == Move::Steamroller || attackused == Move::Stomp || attackused == Move::PhantomForce || attackused == Move::ShadowForce)) {
            finalmod = chainMod(finalmod, 0x2000);
        }
        /* Earthquake + Dig, Surf + Dive */
        /* Calculate the multiplier for two turn attacks */
        if (pokeMemory(t).contains("VulnerableMoves") && pokeMemory(t).value("Invulnerable").toBool()) {
            QList<int> vuln_moves = pokeMemory(t)["VulnerableMoves"].value<QList<int> >();
            QList<int> vuln_mults = pokeMemory(t)["VulnerableMults"].value<QList<int> >();
            for (int i = 0; i < vuln_moves.size(); i++) {
                if (vuln_moves[i] == attackused) {
                    finalmod = chainMod(finalmod, vuln_mults[i]);
                }
            }
        }
        turnMemory(t)["FinalModifier"] = finalmod;
        damage = applyMod(damage, finalmod);

        //If a pokemon protects against a Z move, damage is reduced to 1/4
        if (turnMemory(p).value("ZMoveProtected").toBool()) {
            damage /= 4;
        }
        if (gen() == 5) {
            return std::round(damage); // it is possible to deal 0 damage in Gen 5, but not in Gen 6
        } else {
            return std::max(1, damage);
        }
    }
}

int BattleSituation::repeatNum(int player)
{
    if (turnMemory(player).contains("RepeatCount")) {
        return turnMemory(player)["RepeatCount"].toInt();
    }

    if (tmove(player).repeatMin == 0) {
        if (targetList.size() == 1 && hasWorkingAbility(player, Ability::ParentalBond)) {
            turnMemory(player)["ParentalBond"] = true;
            return 2;
        } else {
            return 1;
        }
    }

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

    if (straightattack && player != source) {
        //Sturdy in gen 5
        callaeffects(player, source, "BeforeTakingDamage");
        callieffects(player, source, "BeforeTakingDamage");
    }

    //Damage can only be 0 if there is a final modifier in play. So like gen 5+
    int finalmod = turnMemory(player).value("FinalModifier").toInt();
    if (damage == 0 && (finalmod >= 0x1000 || finalmod == 0)) {
        damage = 1;
    }

    bool sub = hasSubstitute(player) && !canBypassSub(source);

    // Used for Sturdy, Endure(?), Focus Sash, and Focus Band
    bool survivalFactor = false;

    if (sub && (player != source || goForSub) && straightattack) {
        inflictSubDamage(player, damage, source);
        if(tmove(source).recoil > 0) {
            inflictRecoil(source, player);
        }
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

        if (hp > 0) {
            /* Endure & Focus Sash */
            if (survivalFactor) {
                //Sturdy
                if (turnMemory(player).contains("CannotBeKoedAt") && turnMemory(player)["CannotBeKoedAt"].toInt() == attackCount())
                    callaeffects(player, source, "UponSelfSurvival");

                if (turnMemory(player).contains("SurviveReason"))
                    goto end;

                //False Swipe/Hold Back
                if (turnMemory(player).contains("CannotBeKoedBy") && turnMemory(player)["CannotBeKoedBy"].toInt() == source)
                    calleffects(player, source, "UponSelfSurvival");

                if (turnMemory(player).contains("SurviveReason"))
                    goto end;

                //Endure
                if (turnMemory(player).value("CannotBeKoed").toBool() && source != player)
                    calleffects(player, source, "UponSelfSurvival");

                if (turnMemory(player).contains("SurviveReason"))
                    goto end;

                //Focus Items
                callieffects(player, source, "UponSelfSurvival");

end:
                turnMemory(player).remove("SurviveReason");
            }
        }

        if(hp <= 0) {
            changeHp(player, 0);
            if (straightattack) {
                notify(this->player(player), StraightDamage,player, qint16(damage));
                notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
            }

        } else {
            if (straightattack) {
                notify(this->player(player), StraightDamage,player, qint16(damage));
                notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
            }
            changeHp(player, hp);
        }

        if(tmove(source).recoil > 0) {
            if (!sub && straightattack && player != source) {
                turnMemory(source)["LastDamageInflicted"] = damage;
                inflictRecoil(source, player);
            }
        }

        if (straightattack) {
            if (player != source && !sub) {
                callpeffects(player, source, "UponOffensiveDamageReceived");
                callieffects(player, source, "UponOffensiveDamageReceived");
            }

            if (makesContact(source) && player != source) {
                if (!sub) {
                    callieffects(player, source, "UponPhysicalAssault");
                    callaeffects(player,source,"UponPhysicalAssault");
                    calleffects(player,source,"UponPhysicalAssault");
                }
                callaeffects(source,player,"OnPhysicalAssault");
            }
        }

        if (hp <= 0) {
            koPoke(player, source, straightattack);
        }
    }


    if (straightattack && player != source) {
        if (!sub) {
            /* If there's a sub its already taken care of */
            inc(turnMemory(source)["DamageInflicted"], damage);
            /* Needed for Parental Bond improperly compounding amount of damage to recoil off of*/
            turnMemory(source)["LastDamageInflicted"] = damage;
            pokeMemory(player)["DamageTakenByAttack"] = damage;
            turnMem(player).damageTaken += damage;
            turnMemory(player)["DamageTakenBy"] = source;
        }

        if (damage > 0 || (damage == 0 && survivalFactor)) {
            if(tmove(source).recoil < 0) {
                inflictRecoil(source, player);
            }
            callieffects(source,player, "UponDamageInflicted");
            calleffects(source, player, "UponDamageInflicted");
        }
        if (!sub) {
            calleffects(player, source, "UponOffensiveDamageReceived");
        }
    }

    if (!sub)
        pokeMemory(player)["DamageTaken"] = damage;
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
        turnMemory(player)["HadSubstitute"] = true;
        inc(turnMemory(source)["DamageInflicted"], life);
        /* Needed for Parental Bond improperly compounding amount of damage to recoil off of*/
        turnMemory(source)["LastDamageInflicted"] = life;
        turnMem(player).damageTaken += life;
        sendMoveMessage(128, 1, player);
        notifySub(player, false);
    } else {
        fpoke(player).substituteLife = life-damage;
        inc(turnMemory(source)["DamageInflicted"], damage);
        /* Needed for Parental Bond improperly compounding amount of damage to recoil off of*/
        turnMemory(source)["LastDamageInflicted"] = damage;
        turnMem(player).damageTaken += life;
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
    symbiosisPass(player);
}

void BattleSituation::eatBerry(int player, bool show) {
    int berry = poke(player).item();

    if (show && !turnMemory(player).value("BugBiter").toBool()) {
        sendItemMessage(8000,player,0, 0, berry);

        if (hasWorkingAbility(player, Ability::CheekPouch)) {
            sendAbMessage(125,0,player);
            healLife(player, poke(player).totalLifePoints()/3);
        }
        battleMemory()[QString("BerryEaten%1%2").arg(this->player(player)).arg((currentInternalId(player)))] = true;
    }

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

    if (hasWorkingAbility(s, Ability::CheekPouch)) {
        healLife(s, poke(s).totalLifePoints()/3);
    }

    battleMemory()[QString("BerryEaten%1%2").arg(this->player(p)).arg((currentInternalId(p)))] = true;

    /* Restoring initial conditions */
    pokeMemory(p)["ItemArg"] = tempItemStorage;
    turnMemory(p).remove("BugBiter");
    poke(s).item() = sitem;
}

void BattleSituation::acqItem(int player, int item) {
    if (poke(player).item() != 0)
        loseItem(player, false);
    poke(player).item() = item;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempItem), quint8(slotNum(player)), item);

    //Symbiosis + Eject Button. Item is not activated, but still transfered
    if (turnMemory(player).value("SendingBack").toBool())
        return;

    if (slotNum(player) < numberPerSide()) {
        ItemEffect::setup(poke(player).item(),player,*this);
        callieffects(player, player, "UponSetup");
    }
}

bool BattleSituation::canLoseItem(int player, int attacker)
{
    const auto& poke = this->poke(player);
    int item = poke.item();
    if (item == 0) {
        return false;
    }
    if (attacker != player && hasSubstitute(player) && !canBypassSub(attacker)) {
        return false;
    }
    if (attacker != player && hasWorkingAbility(player, Ability::StickyHold)) {
        return false;
    }
    if (ability(player) == Ability::Multitype) {
        if (gen() <= 4) {
            return false;
        }
        if (ItemInfo::isPlate(item)) {
            return false;
        }
    }
    if (ability(player) == Ability::RKSSystem) {
        if (ItemInfo::isMemoryChip(item)) {
            return false;
        }
    }
    if (item == Item::GriseousOrb) {
        if (gen() <= 4) {
            return false;
        }
        if (poke.num().original() == Pokemon::Giratina) {
            return false;
        }
    }
    if (ItemInfo::isMail(item)) {
        return false;
    }
    if (ItemInfo::isDrive(item) && poke.num().original() == Pokemon::Genesect) {
        return false;
    }
    //primalstones using MegaStoneForme function because lazy
    if ((ItemInfo::isMegaStone(item) || ItemInfo::isPrimalStone(item)) && ItemInfo::MegaStoneForme(item).original() == poke.num().original()) {
        return false;
    }
    /* Knock off */
    if (gen() <= 4 && (battleMemory().value(QString("KnockedOff%1%2").arg(this->player(player)).arg(currentInternalId(player))).toBool())) {
        return false;
    }
    return true;
}

void BattleSituation::loseItem(int player, bool real)
{
    poke(player).item() = 0;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempItem), quint8(slotNum(player)), 0);
    if (real && slotNum(player) < numberPerSide() && hasWorkingAbility(player, Ability::Unburden)) {
        pokeMemory(player)["Unburdened"] = true;
    }
}

void BattleSituation::changeForme(int player, int poke, const Pokemon::uniqueId &newforme, bool temp, bool transform, bool mega)
{
    PokeBattle &p  = this->poke(player,poke);
    if (p.num() == newforme) {
        //Prevents crashes from pokemon accidentally turning into the same forme
        return;
    }
    int slot = this->slot(player, poke);
    if (temp && !pokeMemory(slot).contains("PreTransformPoke")) {
        pokeMemory(slot)["PreTransformPoke"] = PokemonInfo::Name(p.num());
        pokeMemory(slot)["PreTransformAbility"] = AbilityInfo::Name(p.ability());
    }

    /* Note: &o must be defined before p.num() is replaced by newforme */
    PokeBattle &o  = this->poke(player,poke);
    p.num() = newforme;

    if (!transform) {
        int abnum = 0;
        if (!mega) {
            for (int i = 0; i < 3; i++) {
                if (o.ability() == PokemonInfo::Abilities(o.num(), gen()).ab(i)) {
                    abnum = i;
                    break;
                }
            }
        }

        if (!pokeMemory(slot).contains("PreTransformAbility")) {
            p.ability() = PokemonInfo::Abilities(newforme, gen()).ab(abnum);
        } else {
            p.ability() = AbilityInfo::Number(pokeMemory(slot).value("PreTransformAbility").toString());
        }

        for (int i = 1; i < 6; i++)
            p.setNormalStat(i,PokemonInfo::FullStat(newforme,gen(),p.nature(),i,p.level(),p.dvs()[i], p.evs()[i]));
    }

    if (isOut(player, poke)) {
        changeSprite(slot, newforme);

        fpoke(slot).id = newforme;

        if (!transform) {
            //Only change ability if it actually needs to be changed.
            //Imposter is overriden because there's nothing to cancel a transform in battle but without an override it causes a double transform in sendBack.
            if (gen() >= 3 && p.ability() != Ability::ZenMode && p.ability() != Ability::Forecast && p.ability() != Ability::Imposter) {
                acquireAbility(slot, p.ability());
                notify(player, ChangeTempPoke, player, quint8(TempAbility), quint8(poke), p.ability());
            }

            for (int i = 1; i < 6; i++)
                fpoke(slot).stats[i] = p.normalStat(i);

            fpoke(slot).type1 = PokemonInfo::Type1(newforme, gen());
            fpoke(slot).type2 = PokemonInfo::Type2(newforme, gen());
            fpoke(slot).types = QVector<int>() << fpoke(slot).type1 << fpoke(slot).type2;
        }
    }

    notify(All, ChangeTempPoke, player, quint8(DefiniteForme), quint8(poke), newforme);
}

void BattleSituation::changePokeForme(int slot, const Pokemon::uniqueId &newforme)
{
    PokeBattle &p  = this->poke(slot);

    fpoke(slot).id = newforme;
    fpoke(slot).type1 = PokemonInfo::Type1(newforme, gen());
    fpoke(slot).type2 = PokemonInfo::Type2(newforme, gen());
    fpoke(slot).types = QVector<int>() << fpoke(slot).type1 << fpoke(slot).type2;

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
    if (turnMem(player).flags & TM::WasKoed) {
        return;
    }

    changeHp(player, 0);
    if (pokeMemory(slot(player)).contains("PreTransformPoke")) {
        changeForme(this->player(player),slotNum(player),PokemonInfo::Number(pokeMemory(slot(player)).value("PreTransformPoke").toString()));
    }
    //If you primal evolve and die or are forced out on the same turn, the new pokemon's ability isn't loaded without unloading primal forme.
    if (turnMemory(player).contains("PrimalForme")) {
        turnMemory(player).remove("PrimalForme");
    }

    if (!attacking() || tmove(attacker()).power == 0 || gen() >= 5) {
        notifyKO(player);
    }

    for (int i : sortedBySpeed()) {
        if (koed(i) || !arePartners(player, i)) {
            continue;
        }
        callaeffects(i, player, "OnPartnerKO"); //receiver, soul heart
    }
    //useful for third gen
    turnMem(player).add(TM::WasKoed);

    if (straightattack && player!=source) {
        callpeffects(player, source, "AfterKoedByStraightAttack");
        callaeffects(player, source, "AfterBeingKoed");
    }

    /* For free fall */
    if (gen() >= 5)
        callpeffects(player, player, "AfterBeingKoed");
    callaeffects(player, player, "UponKoed");
    //for Strong Weather
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

            if (gen() >= 4) {
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

void BattleSituation::requestSwitch(int s, bool eeffects)
{
    testWin();

    if (pokeMemory(s).contains("PreTransformPoke")) {
        changeForme(player(s), slotNum(s), PokemonInfo::Number(pokeMemory(slot(s)).value("PreTransformPoke").toString()));
    }
    //If you primal evolve and die or are forced out on the same turn, the new pokemon's ability isn't loaded without unloading primal forme.
    if (turnMemory(player(s)).contains("PrimalForme")) {
        turnMemory(player(s)).remove("PrimalForme");
    }

    int player = this->player(s);

    if (countBackUp(player) == 0) {
        //No poke to switch in, so we won't request a choice & such;
        return;
    }

    notifyInfos();

    options[s] = BattleChoices::SwitchOnly(s);

    requestChoice(s,true,true);
    analyzeChoice(s);
    if (eeffects) {
        callEntryEffects(s);
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
        notify(All, UsePP, player, qint16(this->move(player, move)), quint8(PP));
        if (fpoke(player).moves[move] == poke(player).move(move).num()) {
            poke(player).move(move).PP() = PP;
            notify(this->player(player), ChangePP, player, quint8(move), fpoke(player).pps[move]);
        } else {
            notify(this->player(player), ChangeTempPoke, player, quint8(TempPP), quint8(move), quint8(PP));
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
        if (gen() > 2) {
            return turnMemory(player)["UnboostedAttackStat"].toInt() * getStatBoost(player, Attack);
        } else {
            //Gen 2 returns same calculated stat as Gen 1
            return turnMemory(player)["UnboostedAttackStat"].toInt() * (floor(100*getStatBoost(player, Attack))/100);
        }
    } else{
        int givenStat = stat;
        /* Not sure on the order here... haha. */
        /* Attack and defense switched */
        if (pokeMemory(player).contains("PowerTricked") && (stat == 1 || stat == 2)) {
            givenStat = 3 - stat;
        }
        /* Wonder room: defense & sp defense switched*/
        if (battleMemory().contains("WonderRoomCount") && (stat == 2 || stat == 4)) {
            //Wonder room doesn't swap boosts
            //stat = 6 - stat;
            givenStat = 6 - givenStat;
        }
        if (gen() > 2) {
            return fpoke(player).stats[givenStat] * getStatBoost(player, stat);
        } else {
            //Gen 2 returns same calculated stat as Gen 1
            return fpoke(player).stats[givenStat] * (floor(100*getStatBoost(player, stat))/100);
        }

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
    int ret = baseStat;
    if (gen() < 5) {
        ret = ret * (20+turnMemory(player).value(q+"AbilityModifier").toInt())/20;
        if (multiples()) {
            ret = ret * (20+turnMemory(player).value(q+"PartnerAbilityModifier").toInt())/20;
        }
        ret = ret * (20+turnMemory(player).value(q+"ItemModifier").toInt())/20;
    } else {
        int chain = 0x1000;
        if (hasWorkingAbility(player, Ability::Hustle)) {
            //Hustle applies the mod directly instead of chaining it
            ret = applyMod(ret, turnMemory(player).value(q+"AbilityModifier").toInt());
        } else {
            chain = chainMod(chain, turnMemory(player).value(q+"AbilityModifier").toInt());
        }
        if (multiples()) {
            chain = chainMod(chain, turnMemory(player).value(q+"PartnerAbilityModifier").toInt());
        }
        chain = chainMod(chain, turnMemory(player).value(q+"ItemModifier").toInt());
        ret = applyMod(ret, chain);
    }

    if (stat == Speed) {
        if (teamMemory(this->player(player)).value("TailWindCount").toInt() > 0)
            ret *= 2;
        if (teamMemory(this->player(player)).value("SwampCount").toInt() > 0)
            ret /= 2;
        if (poke(player).status() == Pokemon::Paralysed && !hasWorkingAbility(player, Ability::QuickFeet)) {
            if (gen() > 6) {
                ret /= 2;
            } else {
                ret /= 4;
            }
        }
    }

    if (gen() >= 4 && isWeatherWorking(SandStorm) && hasType(player,Pokemon::Rock)) {
        //Wonder Room changes Sandstorm from SpDef to Def
        if (battleMemory().value("WonderRoomCount").toInt() > 0) {
            if (stat == Defense) {
                ret = ret * 3 / 2;
            }
        } else if (stat == SpDefense) {
            ret = ret * 3 / 2;
        }
    }

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
        //Unaware / Sacred sword / Keeneye ignores evasion in gen 6
        if (attacker != player && attacked == player) {
            if (((hasWorkingAbility(attacker, Ability::Unaware) || tmove(attacker).attack == Move::ChipAway || tmove(attacker).attack == Move::DarkestLariat || tmove(attacker).attack == Move::SacredSword)
                    && (stat == SpDefense || stat == Defense || stat == Evasion)) || (gen() > 5 && stat == Evasion && hasWorkingAbility(attacker, Ability::KeenEye))) {
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
    if (teamMemory(player).contains("StickyWeb") && teamMemory(player).value("StickyWeb").toBool()) {
        ret.flags |= BattleDynamicInfo::StickyWeb;
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
    if (counters(b.slot()).hasCounter(BattleCounterIndex::Encore) && gen() <= 4)
        choice(b.slot()).choice.attack.attackTarget = b.slot();
}

void BattleSituation::setupMove(int i, int move)
{
    MoveEffect::setup(move,i,0,*this);
}

bool BattleSituation::canHeal(int s, int part, int focus)
{
    if (koed(s) || poke(s).isFull())
        return false;

    //Gen 4 items are not hindered by heal block.
    if (pokeMemory(s).value("HealBlockCount").toInt() > 0 && !(gen() <= 4 && part == HealByItem)) {
        sendMoveMessage(59,part,s,Type::Psychic,s,focus);
        return false;
    }

    return true;
}

bool BattleSituation::canBypassSub(int t)
{
    if (tmove(t).flags & Move::MischievousFlag)
        return true;

    if (gen().num >= 6) {
        if (hasWorkingAbility(t, Ability::Infiltrator))
            return true;
        if (tmove(t).flags & Move::SoundFlag)
            return true;
    }
    return false;
}

void BattleSituation::symbiosisPass(int s)
{
    foreach (int p, sortedBySpeed()) {
        if (player(p) == player(s)) {
            callaeffects(p, s, "AllyItemUse");
        }
    }
}

bool BattleSituation::canPassMStone (int target, int item) {
    if (ItemInfo::isMegaStone(item) && ItemInfo::MegaStoneForme(item).original() == pokenum(target).original()) {
        return false;
    }
    return true;
}

bool BattleSituation::preTransPoke(int s, Pokemon::uniqueId check)
{
    if (pokeMemory(slot(s)).contains("PreTransformPoke")) {
        return PokemonInfo::Number(pokeMemory(slot(s)).value("PreTransformPoke").toString()) != check;
    }
    return false;
}

bool BattleSituation::canMegaEvolve (int slot)
{
    if (megas[player(slot)]) {
        return false;
    }
    int item = poke(slot).item();
    if (ItemInfo::isZCrystal(item)) {
        //Rayquaza can't mega evolve if it holds a Z Crystal
        return false;
    }
    if (ItemInfo::isMegaStone(item)) {
        //Pokemon can't mega into themselves
        if (ItemInfo::MegaStoneForme(item) == poke(slot).num()) {
            return false;
        }
        //But they can mega between forms! (Ex: Char X -> Char Y)
        if (ItemInfo::MegaStoneForme(item).original() == poke(slot).num()) {
            return true;
        }
        if (ItemInfo::MegaStoneForme(item).original() == Pokemon::uniqueId(poke(slot).num().pokenum,0) && !pokeMemory(slot).contains("PreTransformPoke")) {
            return true;
        }
    }
    if ((poke(slot).num() == Pokemon::Rayquaza && hasMove(slot, Move::DragonAscent))) {
        return true;
    }
    return false;
}

bool BattleSituation::canUseZMove (int slot)
{
    if (zmoves[player(slot)]) {
        return false;
    }
    int item = poke(slot).item();
    if (ItemInfo::isZCrystal(item)) {
        //A Pokemon must have a move of equal type to the Z Crystal in order to use.
        //Special cases may apply
        int zmove = ItemInfo::CrystalMove(item);
        int ztype = MoveInfo::Type(zmove, gen());
        for (int i = 0; i < 4; i++) {
            if (MoveInfo::Type(move(slot, i), gen()) == ztype) {
                return true;
            }
        }
    }
    return false;
}

int BattleSituation::intendedMoveSlot (int s, int slot, int mv)
{
    if (move(s, slot) == mv) {
        return slot;
    }
    for(int i = 0; i < 4; i++) {
        if (move(s,i) == mv) {
            return i;
        }
    }
    return slot;
}

bool BattleSituation::makesContact(int s)
{
    if (hasWorkingAbility(s, Ability::LongReach)) {
        return false;
    }
    if (hasWorkingItem(s, Item::ProtectivePads)) {
        sendItemMessage(72, s, 0);
        return false;
    }
    if (tmove(s).flags & Move::ContactFlag) {
        return true;
    }
    return false;
}

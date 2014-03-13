#include "auxpokedataproxy.h"
#include <PokemonInfo/pokemoninfo.h>
#include "battledataaccessor.h"
#include "zoneproxy.h"

AuxPokeDataProxy::AuxPokeDataProxy()
{
    poke = NULL;
    showing = true;
    onTheField = false;
    substitute = false;
    playerPoke = false;

    resetStatBoosts();
    resetStats();
}

AuxPokeDataProxy::~AuxPokeDataProxy()
{
    if (playerPoke) {
        //delete poke; /* Memory leak if you don't give the pokemon a parent (a team parent) */
    }
}

void AuxPokeDataProxy::setPlayerPoke(bool p)
{
    playerPoke = p;

    if (p) {
        poke = new PokeProxy(new PokeBattle());
        poke->setOwnerShip(true);
    }
}

int AuxPokeDataProxy::statBoost(int stat)
{
    return statboosts[stat];
}

int AuxPokeDataProxy::stat(int stat)
{
    if (pokemon() && pokemon()->gen().num <= 2 && basestats[stat] == 0) {
        int boost = statBoost(stat);

        return PokemonInfo::Stat(pokemon()->num(), pokemon()->gen(), stat, pokemon()->level(), 15, 255)
                * PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    }

    if (!playerPoke) {
        return 0;
    }

    if (basestats[stat] == 0) {
        return 0;
    }

    return boostedstats[stat];
}

int AuxPokeDataProxy::minStat(int stat)
{
    int st = this->stat(stat);

    if (st) return st;

    if (!pokemon()) return 0;

    int boost = statBoost(stat);

    if (pokemon()->gen().num <=2) {
        return (PokemonInfo::Stat(pokemon()->num(), pokemon()->gen(), stat, pokemon()->level(), 0, 0))
                * PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else {
        return (PokemonInfo::Stat(pokemon()->num(), pokemon()->gen(), stat, pokemon()->level(), 0, 0) * 9/10)
                * PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    }
}

int AuxPokeDataProxy::maxStat(int stat)
{
    int st = this->stat(stat);

    if (st) return st;

    if (!pokemon()) return 0;

    int boost = statBoost(stat);

    if (pokemon()->gen().num <=2) {
        return (PokemonInfo::Stat(pokemon()->num(), pokemon()->gen(), stat, pokemon()->level(), 15, 255))
                * PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else {
        return (PokemonInfo::Stat(pokemon()->num(), pokemon()->gen(), stat, pokemon()->level(), 31, 255) * 11/10)
                * PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    }
}

int AuxPokeDataProxy::type1()
{
    if (pokemon()) {
        return PokemonInfo::Type1(pokemon()->num(), pokemon()->gen());
    } else {
        return Type::Curse;
    }
}

int AuxPokeDataProxy::type2()
{
    if (pokemon()) {
        return PokemonInfo::Type2(pokemon()->num(), pokemon()->gen());
    } else {
        return Type::Curse;
    }
}

void AuxPokeDataProxy::resetStatBoosts()
{
    for (int i = 0; i < 8; i++) {
        statboosts[i] = 0;
    }
}

void AuxPokeDataProxy::resetStats()
{
    for (int i = 0; i < 8; i++) {
        boostedstats[i] = basestats[i] = 0;
    }
}

void AuxPokeDataProxy::setBoost(int stat, int level)
{
    statboosts[stat] = level;

    updateBoostedStat(stat);
}

void AuxPokeDataProxy::setStat(int stat, int value)
{
    boostedstats[stat] = value;
}

void AuxPokeDataProxy::updateBoostedStat(int stat)
{
    if (basestats[stat] != 0) {
        boostedstats[stat] = PokemonInfo::BoostedStat(basestats[stat], statBoost(stat));

        if (poke) {
            if (stat == Speed && poke->status() == Pokemon::Paralysed) {
                boostedstats[stat] = std::max(boostedstats[stat]/4, 1);
            }

            //Todo: add other abilities, such as Huge Power, in stat calculation
        }
    }
}

void AuxPokeDataProxy::changeForme(int subnum)
{
    /* TODO: Still need to update the stats */
    if (pokemon()) {
        setAlternateSprite(Pokemon::uniqueId(pokemon()->num().pokenum, subnum));
    }
}

void AuxPokeDataProxy::boostStat(int stat, int level)
{
    statboosts[stat] += level;

    if (statboosts[stat] > 6) {
        statboosts[stat] = 6;
    } else if (statboosts[stat] < -6) {
        statboosts[stat] = -6;
    }

    updateBoostedStat(stat);

    if (level > 0) {
        emit statUp(stat, level);
    } else if (level < 0) {
        emit statDown(stat, level);
    }
}

void AuxPokeDataProxy::setPoke(PokeProxy *poke)
{
    if (playerPoke) {
        this->poke->adaptTo(poke->exposedData());

        for (int i = 1; i < 6; i++) {
            basestats[i] = poke->basestat(i);
            updateBoostedStat(i);
        }
    } else {
        this->poke = poke;
        emit pokemonChanged();
    }
}

void AuxPokeDataProxy::onSendOut(ShallowBattlePoke *shallow)
{
    if (playerPoke) {
        this->poke->adaptTo(shallow);
    }

    setShowing(true);
    setSubstitute(false);
    setAlternateSprite(Pokemon::NoPoke);
    setOnTheField(true);
}

void AuxPokeDataProxy::onSendBack()
{
    setOnTheField(false);
    resetStatBoosts();
    if (playerPoke) {
        resetStats();
    }
}

void FieldProxy::setWeather(int weather)
{
    if (mWeather == weather) {
        return;
    }
    mWeather = weather;
    emit weatherChanged();
}

FieldProxy::FieldProxy(int numOfSlots) : mWeather(NormalWeather) {
    for (int i = 0; i < numOfSlots; i++) {
        AuxPokeDataProxy *ptr = new AuxPokeDataProxy();
        ptr->setParent(this);
        auxdata.push_back(ptr);
    }
    for (int i = 0; i < 2; i++) {
        zonedata.push_back(new ZoneProxy());
    }
}

FieldProxy::~FieldProxy() {
    for (int i = 0; i < 2; i++) {
        delete zonedata[i];
    }
}

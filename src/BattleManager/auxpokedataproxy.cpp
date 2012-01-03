#include "auxpokedataproxy.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "battledataaccessor.h"

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
        delete poke;
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
    if (!playerPoke) {
        return 0;
    }

    if (basestats[stat] == 0) {
        return 0;
    }

    return boostedstats[stat];
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
        auxdata.push_back(ptr);
    }
    for (int i = 0; i < 2; i++) {
        zonedata.push_back(new ZoneProxy());
    }
}

FieldProxy::~FieldProxy() {
    for (unsigned i = 0; i < auxdata.size(); i++) {
        delete auxdata[i];
    }
    for (int i = 0; i < 2; i++) {
        delete zonedata[i];
    }
}

ZoneProxy::ZoneProxy() : mSpikes(0), mTSpikes(0), mRocks(0) {

}

ZoneProxy::~ZoneProxy() {

}

void ZoneProxy::setHazards(quint8 hazards) {
    setStealthRocks(hazards & StealthRock);

    if (hazards & Spikes) {
        setSpikesLevel(1);
    } else if (hazards & SpikesLV2) {
        setSpikesLevel(2);
    } else if (hazards & SpikesLV3) {
        setSpikesLevel(3);
    } else  {
        setSpikesLevel(0);
    }

    if (hazards & ToxicSpikes) {
        setToxicSpikesLevel(1);
    } else if (hazards & ToxicSpikesLV2) {
        setToxicSpikesLevel(2);
    } else {
        setToxicSpikesLevel(0);
    }
}

void ZoneProxy::setSpikesLevel(int level)
{
    if (level == spikesLevel())
        return;
    mSpikes = level;
    emit spikesChanged();
}

void ZoneProxy::setToxicSpikesLevel(int level)
{
    if (level == tspikesLevel())
        return;
    mTSpikes = level;
    emit tspikesChanged();
}

void ZoneProxy::setStealthRocks(bool rocks)
{
    if (rocks == stealthRocks())
        return;
    mRocks = rocks;
    emit rocksChanged();
}

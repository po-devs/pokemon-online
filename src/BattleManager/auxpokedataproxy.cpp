#include "auxpokedataproxy.h"

AuxPokeDataProxy::AuxPokeDataProxy()
{
    showing = true;
    onTheField = false;
    substitute = false;

    resetStatBoosts();
}

int AuxPokeDataProxy::statBoost(int stat)
{
    return statboosts[stat];
}

void AuxPokeDataProxy::resetStatBoosts()
{
    for (int i = 0; i < 8; i++) {
        statboosts[i] = 0;
    }
}

void AuxPokeDataProxy::setBoost(int stat, int level)
{
    statboosts[stat] = level;
}

void AuxPokeDataProxy::boostStat(int stat, int level)
{
    statboosts[stat] += level;

    if (statboosts[stat] > 6) {
        statboosts[stat] = 6;
    } else if (statboosts[stat] < -6) {
        statboosts[stat] = -6;
    }

    if (level > 0) {
        emit statUp(stat, level);
    } else if (level < 0) {
        emit statDown(stat, level);
    }
}

void AuxPokeDataProxy::onSendOut()
{
    setShowing(true);
    setSubstitute(false);
    setAlternateSprite(Pokemon::NoPoke);
    setOnTheField(true);
}

void AuxPokeDataProxy::onSendBack()
{
    setOnTheField(false);
    resetStatBoosts();
}

void FieldProxy::setWeather(int weather)
{
    if (mWeather == weather) {
        return;
    }
    mWeather = weather;
    emit weatherChanged();
}

FieldProxy::FieldProxy() : mWeather(NormalWeather) {
    /* Resizes for triple. Later, when loaded with battle configuration, will get
              more accurate loading */
    for (int i = 0; i < 6; i++) {
        AuxPokeDataProxy *ptr = new AuxPokeDataProxy();
        auxdata.push_back(ptr);
    }
    for (int i = 0; i < 2; i++) {
        zonedata.push_back(new ZoneProxy());
    }
}

FieldProxy::~FieldProxy() {
    for (int i = 0; i < 6; i++) {
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

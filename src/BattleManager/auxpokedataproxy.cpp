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
}

FieldProxy::~FieldProxy() {
    for (int i = 0; i < 6; i++) {
        delete auxdata[i];
    }
}

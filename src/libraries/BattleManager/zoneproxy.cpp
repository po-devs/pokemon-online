#include "zoneproxy.h"

ZoneProxy::ZoneProxy() : mSpikes(0), mTSpikes(0), mRocks(0), mStickyWeb(0) {

}

ZoneProxy::~ZoneProxy() {

}

void ZoneProxy::setHazards(quint8 hazards) {
    setStealthRocks(hazards & StealthRock);
    setStickyWeb(hazards&StickyWeb);

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

void ZoneProxy::setStickyWeb(bool web)
{
    if (web == stickyWeb())
        return;
    mStickyWeb = web;
    emit stickyWebChanged();
}

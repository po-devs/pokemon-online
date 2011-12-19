#ifndef ADVANCEDBATTLEDATA_H
#define ADVANCEDBATTLEDATA_H

#include "battledata.h"
#include "proxydatacontainer.h"

class AdvancedBattleData : public BattleDataInherit<ProxyDataContainer, AdvancedBattleData>
{
public:
    typedef BattleDataInherit<ProxyDataContainer, AdvancedBattleData> baseClass;

    AdvancedBattleData(const BattleConfiguration *conf) : baseClass(conf) {

    }

    void onStatBoost(int spot, int stat, int boost, bool silent) {
        (void) silent;
        fieldPoke(spot).boostStat(stat, boost);
    }

    void onDynamicInfo(int spot, BattleDynamicInfo info) {
        for (int i = 0; i < 7; i++) {
            fieldPoke(spot).setBoost(i+1, info.boosts[i]);
        }
    }

    void onStartWeather(int spot, int weather, bool ability) {
        (void) ability;
        (void) spot;
        d()->field()->setWeather(weather);
    }

    void onContinueWeather(int weather) {
        d()->field()->setWeather(weather);
    }

    void onEndWeather(int weather) {
        (void) weather;
        d()->field()->setWeather(Weather::NormalWeather);
    }

    bool areAdjacent (int poke1, int poke2) const {
        return abs(slotNum(poke1)-slotNum(poke2)) <= 1;
    }
};

#endif // ADVANCEDBATTLEDATA_H

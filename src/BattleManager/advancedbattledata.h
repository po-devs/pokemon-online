#ifndef ADVANCEDBATTLEDATA_H
#define ADVANCEDBATTLEDATA_H

#include "battledata.h"
#include "proxydatacontainer.h"

class AdvancedBattleData : public BattleDataInherit<ProxyDataContainer, AdvancedBattleData>
{
public:
    typedef BattleDataInherit<ProxyDataContainer, AdvancedBattleData> baseClass;

    AdvancedBattleData(BattleConfiguration *conf) : baseClass(conf) {

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
};

#endif // ADVANCEDBATTLEDATA_H

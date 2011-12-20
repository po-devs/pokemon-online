#ifndef BATTLEDATATYPES_H
#define BATTLEDATATYPES_H

class DataContainer;
template<class T> class BattleData;
class ProxyDataContainer;
class AdvancedBattleData;

typedef BattleData<DataContainer> battledata_basic;
typedef BattleData<ProxyDataContainer> battledata_proxy;
typedef AdvancedBattleData advbattledata_proxy;

#endif // BATTLEDATATYPES_H

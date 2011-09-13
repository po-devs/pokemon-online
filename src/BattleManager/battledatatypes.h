#ifndef BATTLEDATATYPES_H
#define BATTLEDATATYPES_H

#include "battledata.h"
#include "proxydatacontainer.h"

typedef BattleData<DataContainer> battledata_basic;
typedef BattleData<ProxyDataContainer> battledata_proxy;

#endif // BATTLEDATATYPES_H

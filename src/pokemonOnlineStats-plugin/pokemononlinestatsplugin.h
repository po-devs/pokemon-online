#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "pokemonOnlineStats-plugin_global.h"
#include "../Server/plugininterface.h"

extern "C" {
POKEMONONLINESTATSPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(void);
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsPlugin
    : public ServerPlugin
{
public:
    PokemonOnlineStatsPlugin();

    QString pluginName();
};

#endif // POKEMONONLINESTATSPLUGIN_H

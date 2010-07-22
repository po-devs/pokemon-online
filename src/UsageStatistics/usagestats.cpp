#include "usagestats.h"

ServerPlugin * createPluginClass() {
    return new PokemonOnlineStatsPlugin();
}

PokemonOnlineStatsPlugin::PokemonOnlineStatsPlugin()
{
}

QString PokemonOnlineStatsPlugin::pluginName()
{
    return "Usage Statistics";
}

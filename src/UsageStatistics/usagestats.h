#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "usagestats_global.h"
#include "../Server/plugininterface.h"

#include <QtCore>

extern "C" {
POKEMONONLINESTATSPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(void);
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsPlugin
    : public ServerPlugin
{
public:
    PokemonOnlineStatsPlugin();
    virtual ~PokemonOnlineStatsPlugin() {}

    QString pluginName();

    void battleStarting(Player *p1, Player *p2);

private:
    QSet<QString> existingDirs;
};

#endif // POKEMONONLINESTATSPLUGIN_H

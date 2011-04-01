#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "usagestats_global.h"
#include "../Server/plugininterface.h"
#include "../Server/battleinterface.h"

#include <QtCore>

extern "C" {
POKEMONONLINESTATSPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(void);
};

class PokeBattle;

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsPlugin
    : public ServerPlugin
{
public:
    PokemonOnlineStatsPlugin();
    virtual ~PokemonOnlineStatsPlugin() {}

    QString pluginName() const;

    BattlePlugin *getBattlePlugin(BattleInterface*);
    bool hasConfigurationWidget() const;

/* Private */
    QHash<QString, QString> existingDirs;
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsBattlePlugin
    : public BattlePlugin
{
public:
    PokemonOnlineStatsBattlePlugin();

    QHash<QString, Hook> getHooks();

    int battleStarting(BattleInterface &b);
    void savePokemon(const PokeBattle &p, bool lead, const QString &d);
private:
    PokemonOnlineStatsPlugin *master;
    static const int bufsize = 6*sizeof(qint32)+4*sizeof(quint16);
    /* Returns a simplified version of the pokemon on bufsize bytes */
    QByteArray data(const PokeBattle &p) const;
};


#endif // POKEMONONLINESTATSPLUGIN_H

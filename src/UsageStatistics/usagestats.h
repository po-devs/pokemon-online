#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "usagestats_global.h"
#include "../Server/plugininterface.h"

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

    void battleStarting(PlayerInterface *p1, PlayerInterface *p2, int mode, unsigned int &c, bool rated);
    bool hasConfigurationWidget() const;

private:
    QHash<QString, QString> existingDirs;
    QCryptographicHash md5;

    /* Returns a simplified version of the pokemon on 28 bytes */
    QByteArray data(const PokeBattle &p) const;
    void savePokemon(const PokeBattle &p, bool lead, const QString &d);

    static const int bufsize = 6*sizeof(qint32)+4*sizeof(quint16);
};

#endif // POKEMONONLINESTATSPLUGIN_H

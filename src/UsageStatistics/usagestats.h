#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "usagestats_global.h"
#include "../Server/plugininterface.h"
#include "../Server/battleinterface.h"
#include "../Server/serverinterface.h"

#include <QtCore>

extern "C" {
POKEMONONLINESTATSPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(ServerInterface*);
};

class PokeBattle;

struct TierRank {
    explicit TierRank(QString tier="");

    int timer;
    QString tier;

    QHash<Pokemon::uniqueId, int> positions;
    QList<QPair<Pokemon::uniqueId,qint32> > uses;

    void addUsage(const Pokemon::uniqueId &pokemon);
    void writeContents();
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsPlugin
    : public ServerPlugin, public QObject
{
public:
    PokemonOnlineStatsPlugin(ServerInterface *server);
    ~PokemonOnlineStatsPlugin();

    QString pluginName() const;

    BattlePlugin *getBattlePlugin(BattleInterface*);
    bool hasConfigurationWidget() const;

    void addUsage(QString tier, const Pokemon::uniqueId &pokemon);

/* Private */
    QHash<QString, TierRank> tierRanks;
    ServerInterface *server;

    QMutex m;
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsBattlePlugin
    : public BattlePlugin
{
public:
    PokemonOnlineStatsBattlePlugin(PokemonOnlineStatsPlugin *master);

    QHash<QString, Hook> getHooks();

    int battleStarting(BattleInterface &b);
    void savePokemon(const PokeBattle &p, bool lead, const QString &d);
private:
    static const int bufsize = 6*sizeof(qint32)+4*sizeof(quint16);
    /* Returns a simplified version of the pokemon on bufsize bytes */
    QByteArray data(const PokeBattle &p) const;

    QPointer<PokemonOnlineStatsPlugin> master;
};


#endif // POKEMONONLINESTATSPLUGIN_H

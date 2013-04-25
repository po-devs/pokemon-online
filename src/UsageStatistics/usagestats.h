#ifndef POKEMONONLINESTATSPLUGIN_H
#define POKEMONONLINESTATSPLUGIN_H

#include "usagestats_global.h"
#include "../Server/plugininterface.h"
#include "../Server/battleinterface.h"
#include "../Server/serverinterface.h"

#include <QtCore>

extern "C" {
POKEMONONLINESTATSPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(ServerInterface*);
}

class PokeBattle;

struct TierRank {
    explicit TierRank(QString tier="");
    ~TierRank();

    int timer;
    QString tier;

    QHash<Pokemon::uniqueId, int> positions;
    QList<QPair<Pokemon::uniqueId,qint32> > uses;

    void addUsage(const Pokemon::uniqueId &pokemon);
    void writeContents();

    QMutex m;
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

    bool isReadyForDeletion() const {
#ifdef QT5
        return refCounter.load() == 0;
#else
        return refCounter == 0;
#endif
    }

/* Private */
    QHash<QString, TierRank*> tierRanks;
    ServerInterface *server;

    QAtomicInt refCounter;
};

class POKEMONONLINESTATSPLUGINSHARED_EXPORT PokemonOnlineStatsBattlePlugin
    : public BattlePlugin
{
public:
    PokemonOnlineStatsBattlePlugin(PokemonOnlineStatsPlugin *master, TierRank *t);
    ~PokemonOnlineStatsBattlePlugin();

    QHash<QString, Hook> getHooks();

    int battleStarting(BattleInterface &b);
    void savePokemon(const PokeBattle &p, bool lead, const QString &d);
private:
    static const int bufsize = 6*sizeof(qint32)+4*sizeof(quint16);
    /* Returns a simplified version of the pokemon on bufsize bytes */
    QByteArray data(const PokeBattle &p) const;
    PokemonOnlineStatsPlugin *master;
    TierRank* ranked_ptr;
};


#endif // POKEMONONLINESTATSPLUGIN_H

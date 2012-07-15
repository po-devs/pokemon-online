#ifndef SQLUSAGEPLUGIN_H
#define SQLUSAGEPLUGIN_H

#include "sqlusagestats_global.h"
#include "../Server/plugininterface.h"
#include "../Server/battleinterface.h"
#include "../Server/serverinterface.h"

#include <QtCore>
#include <QtSql>

extern "C" {
SQLUSAGEPLUGINSHARED_EXPORT ServerPlugin *createPluginClass(ServerInterface*);
}

class PokeBattle;

class SQLUSAGEPLUGINSHARED_EXPORT SqlStatsPlugin: public ServerPlugin, public QObject
{
    ServerInterface *server;
    QSqlDatabase db;
    int battles;
public:
    SqlStatsPlugin(ServerInterface *server);
    ~SqlStatsPlugin();

    QString pluginName() const;

    BattlePlugin *getBattlePlugin(BattleInterface*);
    bool hasConfigurationWidget() const;

    bool isReadyForDeletion() const {
        return battles == 0;
    }

    bool createTable(QString);

friend class SqlStatsBattlePlugin;

};

#endif // SQLUSAGEPLUGIN_H

namespace Pokemon {
    class uniqueId;
}
//unsigned int qHash (const Pokemon::uniqueId &key);

#include "sqlusagestats.h"
#include "../Server/playerinterface.h"
#include "../PokemonInfo/battlestructs.h"
#include "sqlstatsbattleplugin.h"

ServerPlugin *createPluginClass(ServerInterface *server) {
    return new SqlStatsPlugin(server);
}

SqlStatsPlugin::SqlStatsPlugin(ServerInterface *s): server(s) {
    QString connectionName("POSqlUsageStatsPlugin");
    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    /*db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    db.setHostName("");
    db.setUserName("");
    db.setPassword("");*/
    db.setDatabaseName("po-stats");
    if (!db.open())
        qDebug() << "SQL connection failed: " << db.lastError().text();
    qDebug() << "Created sql connection: " << db;

    db = QSqlDatabase::database( connectionName );

    /*
     *for tier in tiers:
     *  createTable(tier)
     **/
}

SqlStatsPlugin::~SqlStatsPlugin() {}

QString SqlStatsPlugin::pluginName() const {
    return "SQL Usage Statistics";
}

BattlePlugin *SqlStatsPlugin::getBattlePlugin(BattleInterface*b) {
    return new SqlStatsBattlePlugin(this);
}

bool SqlStatsPlugin::hasConfigurationWidget() const {
    return false;
}

bool SqlStatsPlugin::createTable(QString name) {
    QString sqliteQuery(
            "CREATE TABLE IF NOT EXISTS `"+name+"` ("

            "`id` integer PRIMARY KEY NOT NULL,"

            "`pokemon` integer NOT NULL,"
            "`form` integer NOT NULL,"

            "`level` integer NOT NULL,"
            "`nature` integer NOT NULL,"
            "`ability` integer NOT NULL,"
            "`gender` integer NOT NULL,"

            "`move1` integer NOT NULL,"
            "`move2` integer NOT NULL,"
            "`move3` integer NOT NULL,"
            "`move4` integer NOT NULL,"

            "`ev_hp` integer NOT NULL,"
            "`ev_atk` integer NOT NULL,"
            "`ev_def` integer NOT NULL,"
            "`ev_satk` integer NOT NULL,"
            "`ev_sdef` integer NOT NULL,"
            "`ev_spd` integer NOT NULL,"

            "`iv_hp` integer NOT NULL,"
            "`iv_atk` integer NOT NULL,"
            "`iv_def` integer NOT NULL,"
            "`iv_satk` integer NOT NULL,"
            "`iv_sdef` integer NOT NULL,"
            "`iv_spd` integer NOT NULL,"

            "`usage` integer NOT NULL,"
            "`lead_usage` integer NOT NULL,"
            //"PRIMARY KEY (id)"
            "UNIQUE (`pokemon`, `form`, `level`, `nature`, `ability`, `gender`, `move1`, `move2`, `move3`, `move4`, `ev_hp`, `ev_atk`, `ev_def`, `ev_satk`, "
                "`ev_sdef`, `ev_spd`, `iv_hp`, `iv_atk`, `iv_def`, `iv_satk`, `iv_sdef`, `iv_spd`)"
            ")"
                   );
    QSqlQuery q(db);
    if (!q.exec(sqliteQuery)) {
        qDebug() << "Failed to create Table" << name << "Error: " << db.lastError().text();
        return false;
    }
    else
        return true;
}

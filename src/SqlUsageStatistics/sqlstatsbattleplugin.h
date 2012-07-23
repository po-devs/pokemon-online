#ifndef SQLSTATSBATTLEPLUGIN_H
#define SQLSTATSBATTLEPLUGIN_H

#include "sqlusagestats.h"

class SQLUSAGEPLUGINSHARED_EXPORT SqlStatsBattlePlugin: public BattlePlugin {
    SqlStatsPlugin *master;
    void bindValues(QSqlQuery &query, PokeBattle &pokemon);

public:
    SqlStatsBattlePlugin(SqlStatsPlugin *master);
    ~SqlStatsBattlePlugin();

    QHash<QString, Hook> getHooks();

    int battleStarting(BattleInterface &b);
    void savePokemon(const PokeBattle &p, bool lead, const QString &d);
};

#endif // SQLSTATSBATTLEPLUGIN_H


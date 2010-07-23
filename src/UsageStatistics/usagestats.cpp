#include "usagestats.h"
#include "../Server/player.h"
#include "../PokemonInfo/pokemonstructs.h"

ServerPlugin * createPluginClass() {
    return new PokemonOnlineStatsPlugin();
}

PokemonOnlineStatsPlugin::PokemonOnlineStatsPlugin()
{
    QDir d;
    d.mkdir("usage_stats");
    d.mkdir("usage_stats/raw");
    d.mkdir("usage_stats/formatted");
}

QString PokemonOnlineStatsPlugin::pluginName()
{
    return "Usage Statistics";
}

void PokemonOnlineStatsPlugin::battleStarting(Player *p1, Player *p2)
{
    if (p1->tier() == p2->tier()) {
        QString tier = p1->tier();
        if (!existingDirs.contains(tier)) {
            QDir d;
            d.mkdir(QString("usage_stats/raw/%1").arg(tier));
            existingDirs[tier] = QString("usage_stats/raw/%1").arg(tier);
        }
    }
}

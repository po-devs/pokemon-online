#include "battleserverlog.h"

BattleServerLog::BattleServerLog(BattleData<DataContainer> *data, BattleDefaultTheme *theme)
                :BattleClientLog(data, theme)
{
    for (int i = 0; i < 2; i++) {
        QStringList roster;
        for (int j=0; j<6; j++) {
            Pokemon::uniqueId num = data->team(i).poke(j)->num();

            if (num != Pokemon::NoPoke) {
                roster+=PokemonInfo::Name(num);
            }
        }
        printHtml("Teams",toBoldColor(tr("%1's team:"), Qt::blue).arg(data->name(i)) + roster.join(" / "));
    }
}

void BattleServerLog::onVariation(int player, int bonus, int malus)
{
    printHtml("Variation", tr("%1+%2, %3").arg(toBoldColor(tr("%1's variation: ").arg(data()->name(player)), Qt::blue)).arg(bonus).arg(malus));
}

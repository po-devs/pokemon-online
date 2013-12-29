#ifndef BATTLESERVERLOG_H
#define BATTLESERVERLOG_H

#include <BattleManager/battleclientlog.h>

class BattleServerLog : public BattleClientLog
{
public:
    BattleServerLog(BattleData<DataContainer> *data, BattleDefaultTheme *theme);

    //do nothing, teams are logged elsewhere
    void onRearrangeTeam(int, const ShallowShownTeam &){}
    //variations are now named
    void onVariation(int player, int bonus, int malus);
};

#endif // BATTLESERVERLOG_H

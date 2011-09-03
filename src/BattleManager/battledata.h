#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"
#include "teamdata.h"

class BattleData : public BattleCommandManager<BattleData>
{
public:
    void onKo(int spot);

    TeamData &team(int player);
private:
    TeamData teams[2];
};

#endif // BATTLEDATA_H

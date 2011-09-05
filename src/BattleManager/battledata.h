#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"
#include "teamdata.h"

class BattleData : public BattleCommandManager<BattleData>
{
public:
    void onKo(int spot);

    TeamData &team(int player);
    ShallowBattlePoke &poke(int player);
    int player(int spot);
    QString name(int player);
    int slotNum(int player);

    enum {
        Player1,
        Player2
    };
private:
    TeamData teams[2];
};

#endif // BATTLEDATA_H

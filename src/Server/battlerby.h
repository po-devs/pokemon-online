#ifndef BATTLERBY_H
#define BATTLERBY_H

#include "battlebase.h"

class BattleRBY : public BattleBase
{
public:
    BattleRBY(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id, int nteam1, int nteam2, PluginManager *p);
    ~BattleRBY();
};

#endif // BATTLERBY_H

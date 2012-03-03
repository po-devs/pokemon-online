#include "battlerby.h"

BattleRBY::BattleRBY(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id, int nteam1, int nteam2, PluginManager *p)
{
    init(p1, p2, additionnalData, id, nteam1, nteam2, p);
}

BattleRBY::~BattleRBY()
{

}

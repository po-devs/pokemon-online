#ifndef BATTLERBY_H
#define BATTLERBY_H

#include "battlebase.h"

class BattleRBY : public BattleBase
{
public:
    BattleRBY(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id, int nteam1, int nteam2, PluginManager *p);
    ~BattleRBY();
protected:
    void beginTurn();
    void endTurn();
    void initializeEndTurnFunctions();
    void changeStatus(int player, int status, bool tell=false, int turns=0);
    int getStat(int poke, int stat);
    void sendPoke(int player, int poke, bool silent);
    BattleChoice &choice (int p);
    BattleChoices createChoice(int slot);
private:
    BattleChoice choices[2];

    BasicPokeInfo pokes[2];

    struct SlotMemory {
        SlotMemory() {
            switchCount = 0;
        }

        quint16 switchCount;
    };

    SlotMemory slotzones[2];

    TurnMemory turnzones[2];

    BasicMoveInfo moves[2];

    BasicPokeInfo &fpoke(int i) {return pokes[i];}
    const BasicPokeInfo &fpoke(int i) const {return pokes[i];}
    SlotMemory &slotMemory(int i) {return slotzones[i];}
    TurnMemory &turnMem(int i) {return turnzones[i];}
    BasicMoveInfo &tmove(int slot) { return moves[slot];}
    const BasicMoveInfo &tmove(int slot) const {return moves[slot];}
};

#endif // BATTLERBY_H

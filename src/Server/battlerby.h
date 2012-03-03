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

    struct TurnMemory {
        TurnMemory() {
            reset();
        }

        void reset() {
            flags = 0;
        }

        quint32 flags;

        enum {
            Incapacitated = 1
        };
    };

    TurnMemory turnzones[2];

    BasicPokeInfo &fpoke(int i) {return pokes[i];}
    SlotMemory &slotMemory(int i) {return slotzones[i];}
    TurnMemory &turnMemory(int i) {return turnzones[i];}
};

#endif // BATTLERBY_H

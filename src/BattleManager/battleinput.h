#ifndef BATTLEINPUT_H
#define BATTLEINPUT_H

#include "battlecommandmanager.h"
#include <vector>

struct BattleConfiguration;
class DataStream;

class BattleInput : public BattleCommandManager<BattleInput>
{
public:
    BattleInput(const BattleConfiguration *conf=0);

    void receiveData(QByteArray data);
    void dealWithCommandInfo(DataStream&, uchar command,int spot);

    void pause(int ticks=1);
    void unpause(int ticks=1);
    bool paused();

    enum StatusFeeling
    {
        FeelConfusion,
        HurtConfusion,
        FreeConfusion,
        PrevParalysed,
        PrevFrozen,
        FreeFrozen,
        FeelAsleep,
        FreeAsleep,
        HurtBurn,
        HurtPoison
    };

    template <enumClass val, typename... Params>
    bool shouldStore(Params...) {
        if (delayCount > 0) {
            //To tell we have a raw command there
            delayedCommands.push_back(QByteArray());
            return true;
        }
        return false;
    }

protected:
    int delayCount;
    std::vector<QByteArray> delayedCommands;
    unsigned mCount; /* Used to know the index in delayedCommands */
    const BattleConfiguration *conf;
};

#endif // BATTLEINPUT_H

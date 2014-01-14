#ifndef BATTLETOJSONFLOW_H
#define BATTLETOJSONFLOW_H

#include <BattleManager/commandflow.h>
#include <BattleManager/param.h>

template <class T, class Underling>
class BattleToJsonFlow
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    template <enumClass val, typename ...Params>
    void receiveCommand(Params... params) {
        wc()->map.clear();
        wc()->updated = true;
        wc()->template invoke<val, Params...>(params...);
        wc()->template output<val, Params...>(params...);
        if (wc()->map.count() > 0) {
            wc()->sendCommand();
        }
    }

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};

#endif // BATTLETOJSONFLOW_H

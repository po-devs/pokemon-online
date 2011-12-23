#ifndef BATTLESCENEFLOW_H
#define BATTLESCENEFLOW_H

#include "commandflow.h"
#include "param.h"

template <class T, class Underling>
class BattleSceneFlow
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    template <enumClass val, typename ...Params>
    void receiveCommand(Params... params) {
        if (wc()->isPeeking()) {
            if (!wc()->template shouldContinuePeeking(param<val>(), params...)) {
                wc()->stopPeeking();
                wc()->replayCommands();

                if (wc()->isPaused()) {
                    wc()->template store(wc()->template createCommand<val, Params...>(params...));
                } else {
                    goto continueLife;
                }
            } else {
                wc()->template store(wc()->template createCommand<val, Params...>(params...));
            }
        } else {
continueLife:
            if (!wc()->replaying() && wc()->template shouldStartPeeking(param<val>(), params...)) {
                wc()->startPeeking();
                wc()->template store(wc()->template createCommand<val, Params...>(params...));
            } else {
                wc()->template invoke<val, Params...>(params...);
                wc()->template output<val, Params...>(params...);
            }
        }
    }

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};


#endif // BATTLESCENEFLOW_H

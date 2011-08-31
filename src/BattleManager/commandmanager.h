#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <functional>
#include <utility>

#include "commandextracter.h"
#include "command.h"
#include "commandflow.h"
#include "commandinvoke.h"

template <class T>
class AbstractCommandManager
{
public:
    typedef T enumClass;

    virtual void entryPoint(enumClass commandId, ...) = 0;
};

/* Example class inherited from AbstractCommandManager, with some cool functionalities */
template<class T=int, class Current=AbstractCommandManager<T>, class Extracter=CommandExtracter<T, Current>,
         class FlowWorker = CommandFlow<T, Current>, class Invoker=CommandInvoker<T, Current> >
class CommandManager : public AbstractCommandManager<T>, public Extracter, public FlowWorker, public Invoker
{
public:
    typedef T enumClass;
    typedef Current type;
    typedef Extracter extracterType;
    typedef FlowWorker flowType;
    typedef Invoker invokeType;

    template <enumClass val, typename ...Params>
    void receiveCommand(Params&&... params) {
        flowType::template receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    bool shouldInvoke(Params...){return true;}

    template <enumClass val, typename ...Params>
    bool shouldSaveCommand(Params...) {return false;}

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        invokeType::template invoke<val, Params...>(std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    void output(Params...) {}

    template <enumClass val, typename ...Params>
    AbstractCommand* createCommand(Params&&... params) {
        return new Command<type, enumClass, val, Params...>(this, std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    AbstractCommand* replayCommand(Params&&... params) {
        receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    /* Not by default */
    void store(AbstractCommand *c) {
        delete c;
        return;
    }

private:
    enum {
        /* If triggered, means Current is incorrect type */
        ErrorCurrentShouldBeCastableToBase = static_cast<AbstractCommandManager<enumClass> *>((Current*) (NULL))
    };
};


#endif // COMMANDMANAGER_H

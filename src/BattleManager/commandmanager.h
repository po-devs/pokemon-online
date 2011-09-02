#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <functional>

#include "command.h"
#include "commandflow.h"
#include "commandinvoke.h"
#include "commandextracter.h"

template <class T>
class AbstractCommandManager
{
public:
    typedef T enumClass;

    virtual void entryPoint(enumClass commandId, va_list) = 0;
    void entryPoint(enumClass commandId, ...) {
        va_list args;
        va_start(args, commandId);
        entryPoint(commandId, args);
        va_end(args);
    }
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
    bool shouldStore(Params...) {return false;}

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        invokeType::template invoke<val, Params...>(std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    void mInvoke(Params&&...) {

    }

    void entryPoint(enumClass commandId, va_list args) {
        extracterType::entryPoint(commandId, args);
    }

    template <enumClass val, typename ...Params>
    void entryPoint(Params&&...params) {
        receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    void unknownEntryPoint(enumClass, va_list) {
        /* If your class never introduces delays, i.e. always forward
          and keeps nothing in storage, it's safe to use
          va_copy to pass it to the outputs.

          Same if all the Extracter<enumClass> never introduce non-POD
          types, or pointer that may go dangling with delays */
    }

    template <enumClass val, typename ...Params>
    void output(Params...) {}

    template <enumClass val, typename ...Params>
    AbstractCommand* createCommand(Params&&... params) {
        return new Command<type, enumClass, val, Params...>(wc(), std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    void replayCommand(Params&&... params) {
        receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    /* Not by default */
    void store(AbstractCommand *c) {
        delete c;
        return;
    }

    inline type* wc() {
        return (type*)(this);
    }

private:
    /* TODO: Fix this */
//    enum {
//        /* If triggered, means Current is incorrect type */
//        ErrorCurrentShouldBeCastableToBase = sizeof(dynamic_cast<AbstractCommandManager<enumClass> *>((Current*) (NULL)))
//    };
};


#endif // COMMANDMANAGER_H

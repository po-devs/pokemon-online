#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <functional>
#include <list>
#include <vector>

#include "command.h"
#include "commandflow.h"
#include "commandinvoke.h"
#include "commandextracter.h"

template <class T>
class AbstractCommandManager
{
public:
    typedef T enumClass;

    virtual ~AbstractCommandManager() {

    }

    virtual void entryPoint_v(enumClass commandId, va_list&) = 0;
    void entryPoint(enumClass commandId, ...) {
        va_list args;
        va_start(args, commandId);
        entryPoint_v(commandId, args);
        va_end(args);
    }
};

template <class T>
class FlowCommandManager : public AbstractCommandManager<T>
{
public:
    typedef T enumClass;
    typedef FlowCommandManager<T> baseClass;

    FlowCommandManager() {
        m_input = NULL;
        deletable = true;
    }

    /* Used to clean up a whole battle flow tree's memory */
    void deleteTree() {
        for(unsigned i = 0; i < m_outputs.size(); i++) {
            m_outputs[i]->deleteTree();
            if (m_outputs[i]->deletable) {
                delete m_outputs[i];
            }
        }
        m_outputs.clear();
    }

    template <enumClass val, typename ...Params>
    void output(Params...params) {
        /* Todo: convert this to new iterating function when gcc 4.6 is widely broadcast */
        for (unsigned i = 0; i < m_outputs.size(); i++) {
            m_outputs[i]->entryPoint(val, params...);
        }
    }

    void addOutput(baseClass* source) {
        m_outputs.push_back(source);
        source->m_input = this;
    }

    /* Reimplement this for the base input class,
      and everything in the chain can be stopped.

      More fine grain control can be achieved with the Command structure */
    virtual void pause(int ticks=1) {
        if (m_input) {
            m_input->pause(ticks);
        }
    }

    virtual void unpause(int ticks=1) {
        if (m_input) {
            m_input->unpause(ticks);
        }
    }

    virtual bool paused() {
        if (m_input) {
            return m_input->paused();
        } else {
            return false;
        }
    }

    bool deletable;
protected:
    baseClass *m_input;
    std::vector<baseClass*> m_outputs;
};

/* Example class inherited from AbstractCommandManager, with some cool functionalities */
template<class T=int, class Current=AbstractCommandManager<T>, class Extracter=CommandExtracter<T, Current>,
         class FlowWorker = CommandFlow<T, Current>, class Invoker=CommandInvoker<T, Current> >
class CommandManager : public FlowCommandManager<T>, public Extracter, public FlowWorker, public Invoker
{
public:
    typedef T enumClass;
    typedef FlowCommandManager<T> baseClass;
    typedef Current type;
    typedef Extracter extracterType;
    typedef FlowWorker flowType;
    typedef Invoker invokeType;

    CommandManager() : misReplayingCommands(false) {

    }

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

    void entryPoint_v(enumClass commandId, va_list &args) {
        extracterType::entryPoint_v(commandId, args);
    }

    template <enumClass val, typename ...Params>
    void entryPoint_tpl(Params&&...params) {
        receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    void unknownEntryPoint(enumClass, va_list&) {
        /* If your class never introduces delays, i.e. always forward
          and keeps nothing in storage, it's safe to use
          va_copy to pass it to the outputs.

          Same if all the Extracter<enumClass> never introduce non-POD
          types, or pointer that may go dangling with delays */
    }

    template <enumClass val, typename ...Params>
    AbstractCommand* createCommand(Params&&... params) {
        return new Command<type, enumClass, val, Params...>(wc(), std::forward<Params>(params)...);
    }

    template <enumClass val, typename ...Params>
    void replayCommand(Params&&... params) {
        receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    /* Not by default */
    void store(AbstractCommand *command) {
        commands.push_back(command);
        return;
    }

    /* Replays the commands stored and delete them. */
    void replayCommands() {
        misReplayingCommands = true;

        int size = commands.size();
        for(int i = 0; i < size; i++) {
            AbstractCommand *command = *commands.begin();
            commands.pop_front();
            command->apply();
            delete command;
        }

        misReplayingCommands = false;
    }

    inline type* wc() {
        return static_cast<type*>(this);
    }

    bool replaying() const {
        return misReplayingCommands;
    }

protected:
    std::list<AbstractCommand *> commands;
    bool misReplayingCommands;
    /* TODO: Fix this */
//    enum {
//        /* If triggered, means Current is incorrect type */
//        ErrorCurrentShouldBeCastableToBase = sizeof(dynamic_cast<AbstractCommandManager<enumClass> *>((Current*) (NULL)))
//    };
};

#endif // COMMANDMANAGER_H

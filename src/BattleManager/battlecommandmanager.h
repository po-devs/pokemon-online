#ifndef BATTLEMANAGER_H
#define BATTLEMANAGER_H

#include "battleenum.h"
#include "commandmanager.h"

template<class T=int, class Current=AbstractCommandManager<T>, class Extracter=CommandExtracter<T, Current>,
         class FlowWorker = CommandFlow<T, Current>, class Invoker=CommandInvoker<T, Current> >

template <class Current, class Invoker=CommandInvoker<battle::BattleEnum, Current>, class FlowWorker = CommandFlow<battle::BattleEnum, Current> >
class BattleCommandManager : public CommandManager<battle::BattleEnum, Current, BattleExtracter<Current>, FlowWorker, Invoker> {
};

#endif // BATTLEMANAGER_H

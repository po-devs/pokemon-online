#ifndef BATTLEMANAGER_H
#define BATTLEMANAGER_H

#include "battleextracter.h"
#include "battleenum.h"
#include "commandmanager.h"
#include "battlecommandinvoker.h"

template <class Current, class FlowWorker = CommandFlow<BattleEnum, Current>, class Invoker=BattleCommandInvoker<Current> >
class BattleCommandManager : public CommandManager<BattleEnum, Current, BattleExtracter<Current>, FlowWorker, Invoker> {
public:
    typedef BattleEnum enumClass;
};

#endif // BATTLEMANAGER_H

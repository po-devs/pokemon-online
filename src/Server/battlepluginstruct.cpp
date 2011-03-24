#include "battleinterface.h"
#include "battlepluginstruct.h"
#include "plugininterface.h"

BattlePStorage::BattlePStorage(BattlePlugin *p)
{
    for (int i = 0; i < lastEnum; i++) {
        calls[i] = NULL;
    }

    QHash<QString, Hook> functions = p->getHooks();

    if (functions.contains("battleStarting(BattleInterface&)")) {
        calls[battleStarting] = functions.value("battleStarting(BattleInterface&)");

        functions.remove("battleStarting(BattleInterface&)");
    }

    if (!functions.empty()) {
        /* To Do: Some way to propagate an error about unreckognized stuff, maybe cancel the plugin entirely */
    }

    plugin = p;
}

BattlePStorage::~BattlePStorage()
{
    delete plugin;
}

void BattlePStorage::call(int f, BattleInterface *b)
{
    if (calls[f]) {
        /* Calls the plugin member function, from the correct class, with the appropriate parameters */
        (*plugin.*(reinterpret_cast<void (BattlePlugin::*)(BattleInterface &)>(calls[f])))(*b);
    }
}

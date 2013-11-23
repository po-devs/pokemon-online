#ifndef BATTLEPLUGININTERFACE_H
#define BATTLEPLUGININTERFACE_H

#include "../Utilities/plugininterface.h"

/* An interface Server Plugins must follow.
 *
 * Remember that plugins are still very experimental and that this file is going to be subject to
 * a lot of changes... */

class BattlePlugin;
class BattleInterface;

class BattleServerPlugin : public Plugin
{
public:
    static const int battleServerPluginVersion = 0;

    virtual BattlePlugin * getBattlePlugin (BattleInterface *) {
        return NULL;
    }

    virtual int version() const {
        return battleServerPluginVersion;
    }
};

class BattlePlugin
{
public:
    typedef int (BattlePlugin::*Hook) ();
    virtual ~BattlePlugin(){}

    virtual QHash<QString, Hook> getHooks(){
        return QHash<QString, Hook>();
    }
};

/* Each plugin will have to have a function like that named
   createPluginClass, that creates a ServerEngine (or a derived
    class) through new and returns it. */
typedef BattleServerPlugin *(*BattleServerPluginInstanceFunction) ();

/* Will be used like that:

class MyPlugin : public BattleServerPlugin
{
...
}

extern "C" {
BattleServerPlugin *createPluginClass();
};

....

BattleServerPlugin *createBattleServerPlugin() {
    return new MyPlugin();
}

*/

#endif // BATTLEPLUGININTERFACE_H

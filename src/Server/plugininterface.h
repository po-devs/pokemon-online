#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QString>
#include <QHash>

/* An interface Server Plugins must follow.
   Remember that plugins are still very experimental and that this file is going to be subject to
   a lot of changes... */

class QWidget;
class PlayerInterface;
class BattleInterface;
class ServerInterface;
class ChallengeInfo;

class BattlePlugin
{
public:
    typedef int (BattlePlugin::*Hook) ();
    virtual ~BattlePlugin(){}

    virtual QHash<QString, Hook> getHooks(){
        return QHash<QString, Hook>();
    }
};

class ServerPlugin
{
public:
    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    virtual QString pluginName() const = 0;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    virtual QWidget * getConfigurationWidget() {
        return NULL;
    }

    virtual bool hasConfigurationWidget() const {
        return false;
    }

    virtual bool isReadyForDeletion() const {
        return true;
    }

    virtual BattlePlugin * getBattlePlugin (BattleInterface *) {
        return NULL;
    }
};

/* Each plugin will have to have a function like that named
   createPluginClass, that creates a ServerEngine (or a derived
    class) through new and returns it. */
typedef ServerPlugin *(*PluginInstanceFunction) (ServerInterface *);

/* Will be used like that:

class MyPlugin : public ServerPlugin
{
...
}

extern "C" {
ServerPlugin *createPluginClass(ServerInterface*);
};

....

ServerPlugin *createPluginClass() {
    return new MyPlugin();
}

*/

#endif // PLUGININTERFACE_H

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QString>
#include <QHash>

/* An interface Client Plugins must follow.
   Remember that plugins are still very experimental and that this file is going to be subject to
   a lot of changes... */

class QWidget;
class MainEngineInterface;
class ClientInterface;
class TeambuilderInterface;

class OnlineClientPlugin : public QObject
{
public:
    virtual ~OnlineClientPlugin(){}

    virtual void clientStartUp() {}
    virtual void clientShutDown() {}

    typedef int (OnlineClientPlugin::*Hook) ();

    virtual QHash<QString, Hook> getHooks(){
        return QHash<QString, Hook>();
    }
};

class TeambuilderPlugin : public QObject
{
public:
    virtual ~TeambuilderPlugin(){}

    virtual void teambuilderStartUp() {}
    virtual void teambuilderShutDown() {}

    typedef int (TeambuilderPlugin::*Hook) ();

    virtual QHash<QString, Hook> getHooks(){
        return QHash<QString, Hook>();
    }
};

class ClientPlugin
{
public:
    virtual ~ClientPlugin() {}

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

    virtual OnlineClientPlugin *getOnlinePlugin(ClientInterface*) {
        return NULL;
    }

    virtual TeambuilderPlugin *getTeambuilderPlugin(TeambuilderInterface*) {
        return NULL;
    }
};

/* Each plugin will have to have a function like that named
   createPluginClass, that creates a ClientPlugin (or a derived
    class) through new and returns it. */
typedef ClientPlugin *(*PluginInstanceFunction) (MainEngineInterface*);

/* Will be used like that:

class MyPlugin : public ClientPlugin
{
...
}

extern "C" {
ClientPlugin *createPluginClass(void);
};

....

ClientPlugin *createPluginClass() {
    return new MyPlugin();
}

*/

#endif // PLUGININTERFACE_H

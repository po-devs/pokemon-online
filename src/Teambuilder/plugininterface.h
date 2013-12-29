#ifndef CLIENTPLUGININTERFACE_H
#define CLIENTPLUGININTERFACE_H

#include <QObject>
#include <QString>
#include <QHash>

#include <Utilities/plugininterface.h>

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

class ClientPlugin : public Plugin
{
public:
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
typedef ClientPlugin *(*ClientPluginInstanceFunction) (MainEngineInterface*);

/* Will be used like that:

class MyPlugin : public ClientPlugin
{
...
}

extern "C" {
ClientPlugin *createPluginClass(void);
};

....

ClientPlugin *createClientPlugin() {
    return new MyPlugin();
}

*/

#endif // CLIENTPLUGININTERFACE_H

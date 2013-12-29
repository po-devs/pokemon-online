#ifndef CLIENTPLUGINMANAGER_H
#define CLIENTPLUGINMANAGER_H

#include <QtCore>
#include <QtGui>
#ifdef QT5
#include <QtWidgets>
#endif

#include <Utilities/pluginmanager.h>
#include "plugininterface.h"

class ClientPlugin;
class MainEngine;
class ClientInterface;
class OnlineClientPlugin;
class TeambuilderInterface;
class TeambuilderPlugin;

class ClientPluginManager : public PluginManager
{
    friend class PluginManagerWidget;
public:
    ClientPluginManager(MainEngine *t);
    ~ClientPluginManager();

    ClientPlugin* plugin(int index) const;
    ClientPlugin* plugin(const QString&) const;

    void addPlugin(const QString &path);
    void freePlugin(int index);

    void launchClient(ClientInterface *c);
    void quitClient(ClientInterface *c);

    void launchTeambuilder(TeambuilderInterface *c);
    void quitTeambuilder(TeambuilderInterface *c);
protected:
    /* What settings file to use? */
    virtual QSettings& settings();
    /* How to instanciate the plugin? */
    virtual ClientPlugin* instanciatePlugin(void *function);
    /* What is the name of the function in the library to create the plugin? */
    virtual const char* instantiatingFunctionName() const {return "createClientPlugin";}
private:
    QHash<ClientInterface*, QHash<ClientPlugin *, OnlineClientPlugin*> > clientPlugins;
    QHash<TeambuilderInterface*, QHash<ClientPlugin *, TeambuilderPlugin*> > teambuilderPlugins;

    MainEngine *engine;
    QSet<ClientInterface *> clients;
    QSet<TeambuilderInterface *> teambuilders;
    QSettings m_settings;
};

#endif // CLIENTPLUGINMANAGER_H

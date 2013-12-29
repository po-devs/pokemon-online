#include "pluginmanager.h"
#include "plugininterface.h"
#include "mainwindow.h"
#include "clientinterface.h"
#include "teambuilderinterface.h"
#include <Utilities/CrossDynamicLib.h>

ClientPluginManager::ClientPluginManager(MainEngine *t) : engine(t)
{
    loadPlugins();
}

ClientPluginManager::~ClientPluginManager()
{
}

QSettings &ClientPluginManager::settings()
{
    return m_settings;
}

ClientPlugin* ClientPluginManager::plugin(int index) const
{
    return dynamic_cast<ClientPlugin*>(PluginManager::plugin(index));
}

ClientPlugin* ClientPluginManager::plugin(const QString &name) const
{
    return dynamic_cast<ClientPlugin*>(PluginManager::plugin(name));
}

void ClientPluginManager::addPlugin(const QString &path)
{
    PluginManager::addPlugin(path);

    ClientPlugin *s = plugin(count()-1);

    foreach (ClientInterface *ci, clients) {
        OnlineClientPlugin *ocp = s->getOnlinePlugin(ci);

        if (ocp) {
            clientPlugins[ci].insert(s, ocp);
            ci->addPlugin(ocp);
        }
    }
    foreach (TeambuilderInterface *ci, teambuilders) {
        TeambuilderPlugin *ocp = s->getTeambuilderPlugin(ci);

        if (ocp) {
            teambuilderPlugins[ci].insert(s, ocp);
            ci->addPlugin(ocp);
        }
    }
}

void ClientPluginManager::freePlugin(int index)
{
    if (index < count() && index >= 0) {
        ClientPlugin *p = plugin(index);
        foreach(ClientInterface *ci, clients) {
            if (clientPlugins.value(ci).contains(p)) {
                ci->removePlugin(clientPlugins[ci][p]);
                delete clientPlugins[ci][p];
                clientPlugins[ci].remove(p);
            }
        }
        foreach(TeambuilderInterface *ci, teambuilders) {
            if (teambuilderPlugins.value(ci).contains(p)) {
                ci->removePlugin(teambuilderPlugins[ci][p]);
                delete teambuilderPlugins[ci][p];
                teambuilderPlugins[ci].remove(p);
            }
        }

        PluginManager::freePlugin(index);
    }
}

void ClientPluginManager::launchClient(ClientInterface *c)
{
    clients.insert(c);

    for(int i = 0; i < count(); i++) {
        ClientPlugin *pl = plugin(i);
        OnlineClientPlugin *o = pl->getOnlinePlugin(c);

        if (o) {
            c->addPlugin(o);
            clientPlugins[c].insert(pl, o);
        }
    }
}

void ClientPluginManager::quitClient(ClientInterface *c)
{
    foreach(OnlineClientPlugin *o, clientPlugins.value(c)) {
        delete o;
    }

    clientPlugins.remove(c);
    clients.remove(c);
}

void ClientPluginManager::launchTeambuilder(TeambuilderInterface *c)
{
    teambuilders.insert(c);

    for(int i = 0; i < count(); i++) {
        ClientPlugin *pl = plugin(i);
        TeambuilderPlugin *o = pl->getTeambuilderPlugin(c);

        if (o) {
            c->addPlugin(o);
            teambuilderPlugins[c].insert(pl, o);
        }
    }
}

void ClientPluginManager::quitTeambuilder(TeambuilderInterface *c)
{
    foreach(TeambuilderPlugin *o, teambuilderPlugins.value(c)) {
        delete o;
    }

    teambuilderPlugins.remove(c);
    teambuilders.remove(c);
}

ClientPlugin *ClientPluginManager::instanciatePlugin(void *function)
{
    return dynamic_cast<ClientPlugin*>(((ClientPluginInstanceFunction)function)(engine));
}

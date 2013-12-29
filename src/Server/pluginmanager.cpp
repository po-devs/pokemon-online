#include "pluginmanager.h"
#include "plugininterface.h"
#include "server.h"
#include <Utilities/CrossDynamicLib.h>

ServerPluginManager::ServerPluginManager(Server *ser) : server(ser), m_settings("config", QSettings::IniFormat)
{
    loadPlugins();
}

QSettings &ServerPluginManager::settings()
{
    return m_settings;
}

ServerPlugin *ServerPluginManager::instanciatePlugin(void *function)
{
    return dynamic_cast<ServerPlugin*>(((ServerPluginInstanceFunction)function)(server));
}

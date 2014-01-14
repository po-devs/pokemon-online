#ifndef SERVERPLUGINMANAGER_H
#define SERVERPLUGINMANAGER_H

#include <Utilities/pluginmanager.h>
#include "plugininterface.h"

class Server;
class ServerPlugin;

class ServerPluginManager : public PluginManager
{
public:
    ServerPluginManager(Server *s);

    server_plugin_version()
protected:
    QSettings &settings();
    ServerPlugin* instanciatePlugin(void *function);
    const char* instantiatingFunctionName() const {return "createServerPlugin";}
    QString directory() const;
private:
    Server *server;
    QSettings m_settings;
};


#endif // SERVERPLUGINMANAGER_H

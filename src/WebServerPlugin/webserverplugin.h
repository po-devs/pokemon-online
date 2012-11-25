#ifndef WEBSERVERPLUGIN_H
#define WEBSERVERPLUGIN_H

#include <QtCore>

#include "WebServerPlugin_global.h"
#include "../Server/plugininterface.h"

class ServerInterface;

extern "C" {
WEBSERVERPLUGINSHARED_EXPORT ServerPlugin * createPluginClass(ServerInterface*);
}

class WEBSERVERPLUGINSHARED_EXPORT WebServerPlugin
    : public QObject, public ServerPlugin
{
    Q_OBJECT
public:
    WebServerPlugin(ServerInterface *server);
    ~WebServerPlugin();

    QString pluginName() const;

public slots:
    void onChatMessage(const QString& message);
private:
    ServerInterface *server;
};

#endif // WEBSERVERPLUGIN_H

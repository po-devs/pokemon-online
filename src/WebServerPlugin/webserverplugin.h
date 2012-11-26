#ifndef WEBSERVERPLUGIN_H
#define WEBSERVERPLUGIN_H

#include <QtCore>

#include "WebServerPlugin_global.h"
#include "../Server/plugininterface.h"
#include "../QtWebsocket/QWsServer.h"

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
    void onServerMessage(const QString& message);
    void dealWithNewConnection();
    void dealWithFrame(const QString& );
    void removeSocket();

signals:
    void sendMessage(const QString &msg);
    void scriptsChanged(const QString&);

private:
    ServerInterface *server;
    QWsServer *webserver;

    QSet<QWsSocket*> clients;

    void broadcast(const QString &);
};

#endif // WEBSERVERPLUGIN_H

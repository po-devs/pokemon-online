#include "../Server/serverinterface.h"
#include "webserverplugin.h"

ServerPlugin * createPluginClass(ServerInterface* server) {
    return new WebServerPlugin(server);
}

WebServerPlugin::WebServerPlugin(ServerInterface* server) : server(server)
{
    connect(dynamic_cast<QObject*>(server), SIGNAL(chatMessage(QString)), SLOT(onChatMessage(QString)));
}

WebServerPlugin::~WebServerPlugin()
{

}

QString WebServerPlugin::pluginName() const
{
    return "Web Server";
}

void WebServerPlugin::onChatMessage(const QString &message)
{
    qDebug() << "Chat message: " << message;
}

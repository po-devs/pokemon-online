#include "../Server/serverinterface.h"
#include "webserverplugin.h"

ServerPlugin * createPluginClass(ServerInterface* server) {
    return new WebServerPlugin(server);
}

WebServerPlugin::WebServerPlugin(ServerInterface* server) : server(server)
{
    qDebug() << "Launching web server...";
    webserver = new QWsServer(this);

    qDebug() << "[WebServer] Starting to listen on port 8508...";
    if (!webserver->listen(QHostAddress::Any, 8508)) {
        qCritical() << "[WebServer] Failure to listen on port 8508...";;
        return;
    }

    QObject *srv = dynamic_cast<QObject*>(server);
    connect(webserver, SIGNAL(newConnection()), SLOT(dealWithNewConnection()));
    connect(srv, SIGNAL(chatMessage(QString)), SLOT(onChatMessage(QString)));
    connect(srv, SIGNAL(serverMessage(QString)), SLOT(onServerMessage(QString)));
    connect(this, SIGNAL(sendMessage(QString)), srv, SLOT(sendServerMessage(QString)));
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
    broadcast("chat|"+message);
}

void WebServerPlugin::onServerMessage(const QString &message)
{
    broadcast("msg|"+message);
}

void WebServerPlugin::dealWithNewConnection()
{
    QWsSocket *s = webserver->nextPendingConnection();

    if (!s) {
        return;
    }

    s->setParent(this);
    connect(s, SIGNAL(disconnected()), s, SLOT(deleteLater()));
    connect(s, SIGNAL(disconnected()), SLOT(removeSocket()));
    connect(s, SIGNAL(frameReceived(QString)), SLOT(dealWithFrame(QString)));

    clients.insert(s);
}

void WebServerPlugin::dealWithFrame(const QString &f)
{
    QString command = f.section("|",0,0);
    QString data = f.section("|", 1);

    if (command == "msg") {
        emit sendMessage(data);
    }
}

void WebServerPlugin::removeSocket()
{
    clients.remove(qobject_cast<QWsSocket*>(sender()));
}

void WebServerPlugin::broadcast(const QString &message)
{
    foreach(QWsSocket *s, clients) {
        s->write(message);
    }
}

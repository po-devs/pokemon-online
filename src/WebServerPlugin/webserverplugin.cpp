#include "../Utilities/functions.h"
#include "../Server/serverinterface.h"

#include "webserverconfig.h"
#include "webserverplugin.h"

ServerPlugin * createPluginClass(ServerInterface* server) {
    return new WebServerPlugin(server);
}

WebServerPlugin::WebServerPlugin(ServerInterface* server) : server(server)
{
    QSettings settings("config", QSettings::IniFormat);

    setDefaultValue(settings, "WebServer/Port", 8508);
    if (!settings.contains("WebServer/Password")) {
        QString s;

        for(int i = 0; i < 12; i++) {
            s.push_back(uchar((true_rand() % (122-49)) + 49));
        }
        settings.setValue("WebServer/Password", s);
    }

    qDebug() << "Launching web server...";
    webserver = new QWsServer(this);

    port = settings.value("WebServer/Port").toInt();
    pass = settings.value("WebServer/Password").toString();

    qDebug() << "[WebServer] Starting to listen on port " << port << "...";
    if (!webserver->listen(QHostAddress::Any, port)) {
        qCritical() << "[WebServer] Failure to listen on port " << port << "...";
        return;
    }

    QObject *srv = dynamic_cast<QObject*>(server);
    connect(webserver, SIGNAL(newConnection()), SLOT(dealWithNewConnection()));
    connect(srv, SIGNAL(chatMessage(QString)), SLOT(onChatMessage(QString)));
    connect(srv, SIGNAL(serverMessage(QString)), SLOT(onServerMessage(QString)));
    connect(this, SIGNAL(sendMessage(QString)), srv, SLOT(sendServerMessage(QString)));
    connect(this, SIGNAL(scriptsChanged(QString)), srv, SLOT(changeScript(QString)));
}

WebServerPlugin::~WebServerPlugin()
{

}

QString WebServerPlugin::pluginName() const
{
    return "Web Server";
}

QWidget *WebServerPlugin::getConfigurationWidget()
{
    return new WebServerConfig(this);
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

    QString ip = s->ip();

    /* Only one websocket per IP */
    foreach(QWsSocket *socket, clients) {
        if (socket->ip() == ip) {
            s->deleteLater();
            return;
        }
    }

    /* You only get 3 connections to get it right, otherwise you're banned
      for 15 minutes */
    attemptsPerIp[ip].push_back(time(NULL));

    if (attemptsPerIp.value(ip).size() >= 4) {
        while (attemptsPerIp.size() > 0 && attemptsPerIp[ip][0] + 15*60 < time(NULL)) {
            attemptsPerIp[ip].remove(attemptsPerIp[ip][0]);
        }

        if (attemptsPerIp[ip].size() >= 4) {
            s->deleteLater();
            return;
        }
    }

    s->setParent(this);
    connect(s, SIGNAL(disconnected()), s, SLOT(deleteLater()));
    connect(s, SIGNAL(disconnected()), SLOT(removeSocket()));
    connect(s, SIGNAL(frameReceived(QString)), SLOT(dealWithFrame(QString)));

    clients.insert(s);

    QString challenge;
    for (int i = 0; i < 20; i++) {
        challenge.push_back(uchar(49 + (true_rand()%(122-49))));
    }
    s->setProperty("challenge", challenge);
    s->write("challenge|"+challenge);
}

void WebServerPlugin::dealWithFrame(const QString &f)
{
    QWsSocket *s = qobject_cast<QWsSocket*>(sender());

    QString command = f.section("|",0,0);
    QString data = f.section("|", 1);

    if (!s->property("loggedIn").toBool()) {
        if (command == "auth") {
            QByteArray expected = md5_hash(s->property("challenge").toByteArray()+"@@"+pass.toUtf8());
            if (expected == data) {
                s->setProperty("loggedIn", true);
                s->write("msg|Successfully logged in from IP " + s->ip() + "!");
                attemptsPerIp.remove(s->ip());
            } else {
                s->disconnectFromHost();
            }
        } else {
            s->disconnectFromHost();
        }
    } else {
        if (command == "msg") {
            emit sendMessage(data);
        } else if (command == "getscripts") {
            s->write("scripts|"+QString::fromUtf8(getFileContent("scripts.js")));
        } else if (command == "changescripts") {
            emit scriptsChanged(data);
        }
    }
}

void WebServerPlugin::removeSocket()
{
    clients.remove(qobject_cast<QWsSocket*>(sender()));
}

void WebServerPlugin::broadcast(const QString &message)
{
    foreach(QWsSocket *s, clients) {
        if (s->property("loggedIn").toBool()) {
            s->write(message);
        }
    }
}

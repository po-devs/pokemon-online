#include "../Utilities/functions.h"
#include "../Utilities/antidos.h"
#include "../Server/serverinterface.h"

#include "webserverconfig.h"
#include "webserverplugin.h"

ServerPlugin * createServerPlugin(ServerInterface* server) {
    return new WebServerPlugin(server);
}

WebServerPlugin::WebServerPlugin(ServerInterface* server) : server(server)
{
    jserial.setIndentMode(QJson::IndentCompact);

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
    connect(this, SIGNAL(tiersUpdated()), srv, SLOT(reloadTiers()));

    connect(this, SIGNAL(nameChanged(QString)), srv, SLOT(regNameChanged(QString)));
    connect(this, SIGNAL(announcementChanged(QString)), srv, SLOT(announcementChanged(QString)));
    connect(this, SIGNAL(mainChannelChanged(QString)), srv, SLOT(mainChanChanged(QString)));
    connect(this, SIGNAL(privateChanged(bool)), srv, SLOT(regPrivacyChanged(bool)));
    connect(this, SIGNAL(proxyServersChanged(QString)), srv, SLOT(proxyServersChanged(QString)));
    connect(this, SIGNAL(antiDosChanged(QSettings&)), server->getAntiDos(), SLOT(loadVals(QSettings&)));
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
        while (attemptsPerIp[ip].size() > 0 && attemptsPerIp[ip][0] + 15*60 < time(NULL)) {
            attemptsPerIp[ip].remove(0);
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
            /* Write new scripts into scripts.js */
            QFile out("scripts.js");
            out.open(QIODevice::WriteOnly);
            out.write(data.toUtf8());
            out.close();

            emit scriptsChanged(data);
        } else if (command == "gettiers") {
            s->write("tiers|"+QString::fromUtf8(getFileContent("tiers.xml")));
        } else if (command == "changetiers") {
            /* Write new tiers into tiers.xml */
            QFile out("tiers.xml");
            out.open(QIODevice::WriteOnly);
            out.write(data.toUtf8());
            out.close();

            emit tiersUpdated();
        } else if (command == "getfile") {
            QFileInfo fi(data);
            QDir d;
            /* Make sure we edit only a file in bin/ */
            if (!fi.absolutePath().contains(d.absolutePath())) {
                return;
            }

            s->write("file|"+data+"|"+QString::fromUtf8(getFileContent(data)));
        } else if (command == "editfile") {
            QString path = data.section("|", 0, 0);

            QFileInfo fi(path);
            QDir d;
            /* Make sure we edit only a file in bin/ */
            if (!fi.absolutePath().contains(d.absolutePath())) {
                return;
            }
            d.mkpath(fi.absolutePath());

            QString content = data.section("|", 1);
            QFile out(path);
            out.open(QIODevice::WriteOnly);
            out.write(content.toUtf8());
            out.close();
        } else if (command == "getconfig") {
            QSettings settings("config", QSettings::IniFormat);
            QString name = settings.value("Server/Name").toString();
            QString announcement = settings.value("Server/Announcement").toString();
            bool priv = settings.value("Server/Private").toBool();
            QString proxies = settings.value("Network/ProxyServers").toString();
            QString mainChan = settings.value("Channels/MainChannel").toString();

            QVariantMap map;
            map.insert("name", name);
            map.insert("announcement", announcement);
            map.insert("private", priv);
            map.insert("proxies", proxies);
            map.insert("mainChannel", mainChan);

            s->write("config|"+QString::fromUtf8(jserial.serialize(map)));
        } else if (command == "editconfig") {
            QVariantMap map = jparser.parse(data.toUtf8()).toMap();

            QSettings settings("config", QSettings::IniFormat);
            QString name = settings.value("Server/Name").toString();
            QString announcement = settings.value("Server/Announcement").toString();
            bool priv = settings.value("Server/Private").toInt();
            QString proxies = settings.value("Network/ProxyServers").toString();
            QString mainChannel = settings.value("Channels/MainChannel").toString();

            if (map.contains("name") && map.value("name").toString() != name) {
                settings.setValue("Server/Name", map.value("name").toString());
                emit nameChanged(map.value("name").toString());
            }
            if (map.contains("mainChannel") && map.value("mainChannel").toString() != mainChannel) {
                settings.setValue("Channels/MainChannel", map.value("mainChannel").toString());
                emit mainChannelChanged(map.value("mainChannel").toString());
            }
            if (map.contains("announcement") && map.value("announcement").toString() != announcement) {
                settings.setValue("Server/Announcement", map.value("announcement").toString());
                emit announcementChanged(map.value("announcement").toString());
            }
            if (map.contains("private") && map.value("private").toBool() != priv) {
                settings.setValue("Server/Private", int(map.value("private").toBool()));
                emit privateChanged(map.value("private").toBool());
            }
            if (map.contains("proxies") && map.value("proxies").toString() != proxies) {
                settings.setValue("Network/ProxyServers", map.value("proxies").toString());
                emit proxyServersChanged(map.value("proxies").toString());
            }
        } else if (command == "getdos") {
            QSettings settings("config", QSettings::IniFormat);

            QVariantMap map;
            map.insert("trustedIPs", settings.value("AntiDOS/TrustedIps").toString());
            map.insert("connections", settings.value("AntiDOS/MaxPeoplePerIp").toInt());
            map.insert("activity", settings.value("AntiDOS/MaxCommandsPerUser").toInt());
            map.insert("upload", settings.value("AntiDOS/MaxKBPerUser").toInt());
            map.insert("channel", settings.value("AntiDOS/NotificationsChannel").toString());
            map.insert("logins", settings.value("AntiDOS/MaxConnectionRatePerIP").toInt());
            map.insert("ban", settings.value("AntiDOS/NumberOfInfractionsBeforeBan").toInt());
            map.insert("on", !settings.value("AntiDOS/Disabled").toBool());

            s->write("dos|"+QString::fromUtf8(jserial.serialize(map)));
        } else if (command == "editdos") {
            QSettings settings("config", QSettings::IniFormat);
            QVariantMap map = jparser.parse(data.toUtf8()).toMap();

            settings.setValue("AntiDOS/TrustedIps", map.value("trustedIPs").toString());
            settings.setValue("AntiDOS/MaxPeoplePerIp", map.value("connections").toInt());
            settings.setValue("AntiDOS/MaxCommandsPerUser", map.value("activity").toInt());
            settings.setValue("AntiDOS/MaxKBPerUser", map.value("upload").toInt());
            settings.setValue("AntiDOS/NotificationsChannel", map.value("channel").toString());
            settings.setValue("AntiDOS/MaxConnectionRatePerIP", map.value("logins").toInt());
            settings.setValue("AntiDOS/NumberOfInfractionsBeforeBan", map.value("ban").toInt());
            settings.setValue("AntiDOS/Disabled", !map.value("on").toBool());

            emit antiDosChanged(settings);
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

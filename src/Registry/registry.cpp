#include "registry.h"
#include "antidos.h"
#include "server.h"
#include "player.h"
#ifdef USE_WEBCONF
#include "webinterface.h"
#endif

Registry::Registry() {
    linecount = 0;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    if (!forPlayers[1].listen(QHostAddress::Any, 5090))
    {
        printLine("Unable to listen to port 5090 (players)");
    } else {
        printLine("Starting to listen to port 5090");
    }
    if (!forPlayers[0].listen(QHostAddress::Any, 8080))
    {
        printLine("Unable to listen to port 8080 (players)");
    } else {
        printLine("Starting to listen to port 8080");
    }

    if (!forServers.listen(QHostAddress::Any, 8081))
    {
        printLine("Unable to listen to port 8081 (servers)");
    } else {
        printLine("Starting to listen to port 8081");
    }

#ifdef USE_WEBCONF
    web_interface = new RegistryWebInterface(this);
#endif

    registry_announcement = "";

    connect(&forPlayers[0], SIGNAL(newConnection()), SLOT(incomingPlayer()));
    connect(&forPlayers[1], SIGNAL(newConnection()), SLOT(incomingPlayer()));
    connect(&forServers, SIGNAL(newConnection()), SLOT(incomingServer()));

    AntiDos::obj()->init();

    connect(AntiDos::obj(), SIGNAL(kick(int)), SLOT(kick(int)));
    connect(AntiDos::obj(), SIGNAL(ban(QString)), SLOT(ban(QString)));

    QTimer *t = new QTimer(this);
    t->setInterval(60*1000);
    t->start();
    connect(t, SIGNAL(timeout()), SLOT(updateTBanList()));
    connect(t, SIGNAL(timeout()), SLOT(updateRegistryAnnouncement()));
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(tbanListReceived(QNetworkReply*)));
    updateTBanList();
    updateRegistryAnnouncement();
}

void Registry::printLine(const QString &line)
{
    qDebug() << line;
}

int Registry::freeid() const
{
    for (int i = 1; ; i++) {
        if (!players.contains(i) && !servers.contains(i)) {
            return i;
        }
    }
}

void Registry::updateTBanList()
{
    QNetworkRequest request;

    request.setUrl(QUrl("http://pokemon-online.eu/files/ips.txt"));
    request.setRawHeader("User-Agent", "Pokemon-Online serverscript");

    manager.get(request);
}

void Registry::tbanListReceived(QNetworkReply* reply){
    //escape reply before sending it to the javascript evaluator
    QList<QString> x = QString::fromUtf8(reply->readAll()).split('\n');
    tbanIPs = x.toSet();

    foreach(Server *s, servers) {
        if (tbanIPs.contains(s->ip())) {
            s->kick();
        }
    }

    reply->deleteLater();
}

void Registry::updateRegistryAnnouncement() {
    QFile file("announcement.txt");
    QString newAnnouncement;
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(file.readAll());
        newAnnouncement = in.readAll();
        if(registry_announcement != newAnnouncement) {
            printLine(QString("New Registry announcement: %1").arg(newAnnouncement));
            registry_announcement = newAnnouncement;
        }
    }
    file.close();
    newAnnouncement.clear();
}

void Registry::incomingServer()
{
    int id = freeid();

    QTcpSocket * newconnection = forServers.nextPendingConnection();
    QString ip = newconnection->peerAddress().toString();

    printLine(QString("Incoming server connection from IP %1 on slot %2").arg(ip).arg(id));

    if (bannedIPs.contains(ip) || tbanIPs.contains(ip.toUtf8()))
    {
        printLine("The ip is banned");
        delete newconnection;
        return;
    }

    if (ipCounter.value(ip) > 2)
    {
        printLine("A server with another IP is already there, disconnecting");
        Server *s = new Server(id, newconnection);
        s->refuseIP();
        delete s;
        return;
    }

    if (!AntiDos::obj()->connecting(ip))
    {
        printLine(QString("DoS blocker blocked the IP %1").arg(ip));
        delete newconnection;
        return;
    }

    ipCounter[ip] += 1;
    servers[id] = new Server(id, newconnection);

    connect(servers[id], SIGNAL(nameChangedReq(int,QString)), SLOT(nameChangedAcc(int,const QString&)));
    connect(servers[id], SIGNAL(portSet(int, int, int)), SLOT(portSet(int,int,int)));
    connect(servers[id], SIGNAL(disconnection(int)), SLOT(disconnection(int)));
}

void Registry::incomingPlayer()
{
    int id = freeid();

    QTcpSocket * newconnection = qobject_cast<QTcpServer*>(sender())->nextPendingConnection();
    QString ip = newconnection->peerAddress().toString();

    printLine(QString("Incoming player connection from IP %1 on slot %2").arg(ip).arg(id));

    if (bannedIPs.contains(ip))
    {
        printLine("The ip is banned");
        delete newconnection;
        return;
    }

    if (!AntiDos::obj()->connecting(ip))
    {
        printLine(QString("DoS blocker blocked the IP %1").arg(ip));
        delete newconnection;
        return;
    }

    Player *p = players[id] = new Player(id, newconnection);

    connect(players[id], SIGNAL(disconnection(int)), SLOT(disconnection(int)));

    printLine("Sending the registry announcement");

    if (registry_announcement.length() > 0) {
        p->sendRegistryAnnouncement(registry_announcement);
    }

    printLine("Sending the server list");
    foreach(Server *s, servers) {
        if (s->listed()) {
            p->sendServer(*s);
        }
    }
    // Tell client that the server list has ended.
    p->sendServerListEnd();
}

void Registry::nameChangedAcc(int id, const QString &name)
{
    if (names.contains(name)) {
        printLine(QString("Refused name %1 for %2 (%3)").arg(name).arg(id).arg(servers[id]->ip()));
        servers[id]->refuseName();
    } else {
        printLine(QString("Accepted name %1 for %2 (%3)").arg(name).arg(id).arg(servers[id]->ip()));
        servers[id]->accept();
        names.remove(servers[id]->name());
        names.insert(name);
        servers[id]->name() = name;
    }
}

void Registry::portSet(int id, int port, int oldport)
{
    Server *s = servers[id];

    serverAddresses.remove(s->getAddress(oldport));
    if (serverAddresses.contains(s->getAddress(port))) {
        s->refuseIP();
        s->kick();
    }

    serverAddresses.insert(s->getAddress(port));
}

void Registry::disconnection(int id)
{
    printLine(QString("Received disconnection from id %1").arg(id));
    if (servers.contains(id)) {
        Server *s = servers[id];
        AntiDos::obj()->disconnect(s->ip(), id);
        names.remove(s->name());
        ipCounter[s->ip()] -= 1;
        if (ipCounter[s->ip()] == 0)
            ipCounter.remove(s->ip());
        serverAddresses.remove(s->getAddress(s->port()));
        servers.remove(id);
        delete s;
    } else if (players.contains(id)) {
        Player *p = players[id];
        AntiDos::obj()->disconnect(p->ip(), id);
        players.remove(id);
        delete p;
    }
}

void Registry::kick(int id)
{
    printLine(QString("Anti DoS kicked id %1").arg(id));
    if (servers.contains(id)) {
        servers[id]->kick();
    } else if (players.contains(id)) {
        players[id]->kick();
    }
}

void Registry::ban(const QString &ip)
{
    printLine(QString("Anti DoS banned ip %1").arg(ip));
    bannedIPs.insert(ip);
}

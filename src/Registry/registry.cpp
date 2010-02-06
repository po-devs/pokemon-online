#include "registry.h"
#include "antidos.h"
#include "server.h"
#include "player.h"

Registry::Registry() {
    linecount = 0;

    mainChat = new QTextEdit(this);

    mainChat->setFixedSize(500,500);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    //AntiDos::obj()->init();

    if (!forPlayers.listen(QHostAddress::Any, 5081))
    {
        printLine("Unable to listen to port 5081 (players)");
    } else {
        printLine("Starting to listen to port 5081");
    }

    if (!forServers.listen(QHostAddress::Any, 5082))
    {
        printLine("Unable to listen to port 5082 (servers)");
    } else {
        printLine("Starting to listen to port 5082");
    }

    connect(&forPlayers, SIGNAL(newConnection()), SLOT(incomingPlayer()));
    connect(&forServers, SIGNAL(newConnection()), SLOT(incomingServer()));

    AntiDos::obj()->init();

    connect(AntiDos::obj(), SIGNAL(kick(int)), SLOT(kick(int)));
    connect(AntiDos::obj(), SIGNAL(ban(QString)), SLOT(ban(QString)));
}

void Registry::printLine(const QString &line)
{
    if (linecount > 1000) {
        mainChat->clear();
        printLine("Cleared the window (1000+ lines were displayed)");
    }

    mainChat->moveCursor(QTextCursor::End);
    mainChat->insertPlainText(line + "\n");
    qDebug() << line;
    linecount += 1;
}

int Registry::freeid() const
{
    for (int i = 1; ; i++) {
        if (!players.contains(i) && !servers.contains(i)) {
            return i;
        }
    }
}

void Registry::incomingServer()
{
    int id = freeid();

    QTcpSocket * newconnection = forServers.nextPendingConnection();
    QString ip = newconnection->peerAddress().toString();

    printLine(QString("Incoming server connection from IP %1 on slot %2").arg(ip).arg(id));

    if (bannedIPs.contains(ip))
    {
        printLine("The ip is banned");
        delete newconnection;
        return;
    }

    if (serverIPs.contains(ip))
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

    serverIPs[ip] = id;
    servers[id] = new Server(id, newconnection);

    connect(servers[id], SIGNAL(nameChangedReq(int,QString)), SLOT(nameChangedAcc(int,const QString&)));
    connect(servers[id], SIGNAL(disconnection(int)), SLOT(disconnection(int)));
}

void Registry::incomingPlayer()
{
    int id = freeid();

    QTcpSocket * newconnection = forPlayers.nextPendingConnection();
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

    printLine("Sending the server list");
    foreach(Server *s, servers) {
        if (s->listed()) {
            p->sendServer(*s);
        }
    }
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

void Registry::disconnection(int id)
{
    printLine(QString("Received disconnection from id %1").arg(id));
    if (servers.contains(id)) {
        Server *s = servers[id];
        AntiDos::obj()->disconnect(s->ip(), id);
        names.remove(s->name());
        serverIPs.remove(s->ip());
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

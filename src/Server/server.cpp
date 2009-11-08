#include "server.h"

Player::Player(QTcpSocket *sock) : myrelay(sock)
{
    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(QString)), this, SLOT(loggedIn(QString)));
    connect(&relay(), SIGNAL(messageReceived(QString)), this, SLOT(recvMessage(QString)));
    connect(&relay(), SIGNAL(teamReceived(TeamInfo)), this, SLOT(recvTeam(TeamInfo)));
}

Player::~Player()
{
}

void Player::disconnected()
{
    emit disconnected(id());
}

void Player::recvMessage(const QString &mess)
{
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), mess);
}

Analyzer & Player::relay()
{
    return myrelay;
}

void Player::loggedIn(const QString &name)
{
    team().name = name;

    emit loggedIn(id(), name);
}

QString Player::name() const
{
    return team().name;
}

void Player::setId(int id)
{
    myid = id;
}

void Player::recvTeam(const TeamInfo &team)
{
    this->team() = team;

    emit recvTeam(id());
}

int Player::id() const
{
    return myid;
}

void Player::sendMessage(const QString &mess)
{
    relay().sendMessage(mess);
}

TeamInfo & Player::team()
{
    return myteam;
}

const TeamInfo & Player::team() const
{
    return myteam;
}

Server::Server(quint16 port)
{
    mymainchat = new QTextEdit(this);
    mainchat()->setFixedSize(500,500);
    mainchat()->setReadOnly(true);

    if (!server()->listen(QHostAddress::Any, port))
    {
	printLine(tr("Unable to listen to port %1").arg(port));
    } else {
	printLine(tr("Starting to listen to port %1").arg(port));
    }

    connect(server(), SIGNAL(newConnection()), SLOT(incomingConnection()));
}

QTcpServer * Server::server()
{
    return &myserver;
}

void Server::printLine(const QString &line)
{
    mainchat()->insertPlainText(line + "\n");
}

void Server::loggedIn(int id, const QString &name)
{
    printLine(tr("Player %1 logged in as %2").arg(id).arg(name));

    sendPlayersList(id);
    sendLogin(id);

    sendMessage(id, tr("Welcome to our server, %1").arg(name));
}

void Server::sendMessage(int id, const QString &message)
{
    player(id)->sendMessage(message);
}

void Server::recvMessage(int id, const QString &mess)
{
    sendAll(tr("%1: %2").arg(name(id)).arg(mess));
}

QString Server::name(int id) const
{
    return player(id)->name();
}

void Server::incomingConnection()
{
    int id = freeid();

    myplayers[id] = new Player(server()->nextPendingConnection());

    printLine(tr("Received pending connection on slot %1").arg(id));

    player(id)->setId(id);

    connect(player(id), SIGNAL(loggedIn(int, QString)), this, SLOT(loggedIn(int,QString)));
    connect(player(id), SIGNAL(recvMessage(int, QString)), this, SLOT(recvMessage(int,QString)));
    connect(player(id), SIGNAL(recvTeam(int)), this, SLOT(recvTeam(int)));
    connect(player(id), SIGNAL(disconnected(int)), SLOT(disconnected(int)));
}

void Server::sendPlayersList(int id)
{
    Analyzer &relay = player(id)->relay();

    /* getting what to send */
    foreach(Player *p, myplayers)
    {
	relay.sendPlayer(p->id(), p->team());
    }
}

void Server::sendLogin(int id)
{
    foreach(Player *p, myplayers)
    {
	if (p->id() != id)
	    p->relay().sendLogin(id, player(id)->team());
    }
}

void Server::sendLogout(int id)
{
    foreach(Player *p, myplayers)
    {
	p->relay().sendLogout(id);
    }
}

void Server::recvTeam(int id)
{
    printLine(tr("%1 changed their team.").arg(name(id)));
}

void Server::disconnected(int id)
{
    printLine(tr("Received disconnection from player %1").arg(name(id)));
    removePlayer(id);
}

void Server::removePlayer(int id)
{
    sendLogout(id);

    QString playerName = name(id);
    printLine(tr("Removed player %1").arg(playerName));

    /* So there won't be infinite recursion with signal disconnected */
    player(id)->blockSignals(true);

    delete player(id);
    myplayers.remove(id);
}

void Server::sendAll(const QString &message)
{
    printLine(message);

    foreach (Player *p, myplayers)
	p->sendMessage(message);
}

int Server::freeid() const
{
    int prev = 0;
    for (QMap<int, Player*>::const_iterator it = myplayers.begin(); it != myplayers.end(); ++it)
    {
	if ( it.key() != prev + 1 ) {
	    return prev + 1;
	}
	prev++;
    }
    return prev + 1;
}

QTextEdit * Server::mainchat()
{
    return mymainchat;
}

Player * Server::player(int id)
{
    return myplayers[id];
}

const Player * Server::player(int id) const
{
    return myplayers[id];
}

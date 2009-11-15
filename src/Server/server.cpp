#include "server.h"
#include "player.h"
#include "battle.h"

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

    foreach(Player *p, myplayers)
	if (p->isLoggedIn() && p->name().compare(name, Qt::CaseInsensitive) == 0) {
	    printLine(tr("Name %1 already in use, disconnecting player %2").arg(name).arg(id));
	    sendMessage(id, tr("Another with the name %1 is already logged in").arg(name));
	    removePlayer(id);
	    return;
	}

    player(id)->setLoggedIn(true);

    sendPlayersList(id);
    sendLogin(id);

    sendMessage(id, tr("Welcome Message: Welcome to our server, %1").arg(name));
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

    connect(player(id), SIGNAL(loggedIn(int, QString)), this, SLOT(loggedIn(int, QString)));
    connect(player(id), SIGNAL(recvMessage(int, QString)), this, SLOT(recvMessage(int,QString)));
    connect(player(id), SIGNAL(recvTeam(int)), this, SLOT(recvTeam(int)));
    connect(player(id), SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(player(id), SIGNAL(challengeFromTo(int,int)), SLOT(dealWithChallenge(int, int)));
    connect(player(id), SIGNAL(busyForChallenge(int,int)), this, SLOT(busyForChallenge(int,int)));
    connect(player(id), SIGNAL(challengeAcc(int,int)), this, SLOT(challengeAccepted(int,int)));
    connect(player(id), SIGNAL(challengeRef(int,int)), this, SLOT(challengeRefused(int,int)));
    connect(player(id), SIGNAL(challengeCanceled(int,int)), this, SLOT(cancelChallenge(int,int)));
}

void Server::cancelChallenge(int from, int to)
{
    if (playerExist(to) && player(to)->isChallenged() && player(to)->challengedBy() == from)
	player(to)->sendChallengeCancel(from);
}

void Server::dealWithChallenge(int from, int to)
{
    if (!playerExist(to) || !player(to)->isLoggedIn()) {
	sendMessage(from, tr("That player is not online"));
	return;
    } else {
	if (player(to)->challenge(from)) {
	    printLine(tr("Challenge issued from %1 to %2").arg(name(from)).arg(name(to)));
	} else {
	    player(from)->sendBusyForChallenge(to);
	}
    }
}

void Server::challengeAccepted(int from, int to)
{
    if (!playerExist(to) || !player(to)->isLoggedIn()) {
	sendMessage(from, tr("That player is not online"));
	return;
    } else {
	startBattle(from, to);
    }
}

void Server::startBattle(int id1, int id2)
{
    printLine(tr("Battle between %1 and %2 started").arg(name(id1)).arg(name(id2)));

    BattleSituation *battle = new BattleSituation(*player(id1), *player(id2));

    mybattles.insert(id1, battle);
    mybattles.insert(id2, battle);

    player(id1)->startBattle(id2, battle->pubteam(id1));
    player(id2)->startBattle(id1, battle->pubteam(id2));
}

void Server::battleResult(int desc, int winner, int loser)
{
    if (desc == Forfeit) {
	printLine( tr("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
	player(winner)->battleResult(Win);
	player(loser)->battleResult(Forfeit);

	removeBattle(winner, loser);
    }
}

void Server::removeBattle(int winner, int loser)
{
    delete mybattles[winner];
    mybattles.remove(winner);
    mybattles.remove(loser);
}

void Server::challengeRefused(int from, int to)
{
    if (!playerExist(to) || !player(to)->isLoggedIn()) {
	return;
    } else {
	printLine(tr("Player %1 refused challenge from %2").arg(name(from)).arg(name(to)));
	player(to)->sendChallengeRefusal(from);
    }
}

void Server::busyForChallenge(int from, int to)
{
    if (!playerExist(to) || !player(to)->isLoggedIn()) {
	return;
    } else {
	printLine(tr("Player %1 is busy so can't answer challenge from %2").arg(name(from)).arg(name(to)));
	player(to)->sendBusyForChallenge(from);
    }
}


void Server::sendPlayersList(int id)
{
    Analyzer &relay = player(id)->relay();

    /* getting what to send */
    foreach(Player *p, myplayers)
    {
	if (p->isLoggedIn())
	    relay.sendPlayer(p->id(), p->basicInfo());
    }
}

void Server::sendLogin(int id)
{
    foreach(Player *p, myplayers)
    {
	if (p->id() != id && p->isLoggedIn())
	    p->relay().sendLogin(id, player(id)->basicInfo());
    }
}

void Server::sendLogout(int id)
{
    foreach(Player *p, myplayers)
    {
	if (p->isLoggedIn())
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

bool Server::playerExist(int id) const
{
    return myplayers.contains(id);
}

void Server::removePlayer(int id)
{
    if (playerExist(id))
    {
	Player *p = myplayers.take(id);

	/* Sending the notice of logout to others only if the player is already logged in */
	if (p->isLoggedIn())
	    sendLogout(id);

	p->setLoggedIn(false);
    
	QString playerName = p->name();

	delete p;

	printLine(tr("Removed player %1").arg(playerName));
    }
}

void Server::sendAll(const QString &message)
{
    printLine(message);

    foreach (Player *p, myplayers)
	if (p->isLoggedIn())
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

#include "server.h"
#include <ctime>
#include "player.h"
#include "battle.h"
#include "../PokemonInfo/pokemoninfo.h"

Server::Server(quint16 port)
{
    mymainchat = new QTextEdit(this);
    mainchat()->setFixedSize(500,500);
    mainchat()->setReadOnly(true);

    srand(time(NULL));

    printLine(tr("Starting loading pokemon database..."));

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");
    NatureInfo::init("db/");
    CategoryInfo::init("db/");
    AbilityInfo::init("db/");
    HiddenPowerInfo::init("db/");
    StatInfo::init("db/");

    printLine(tr("Pokémon database loaded"));

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

    player(id)->changeState(Player::LoggedIn);

    sendPlayersList(id);
    sendLogin(id);

    sendMessage(id, tr("Welcome Message: Welcome to our server, %1").arg(name));
}

void Server::sendBattleCommand(int id, const QByteArray &comm)
{
    player(id)->relay().sendBattleCommand(comm);
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
    connect(player(id), SIGNAL(challengeStuff(int,int,int)), SLOT(dealWithChallenge(int,int,int)));
    connect(player(id), SIGNAL(battleFinished(int,int,int)), SLOT(battleResult(int,int,int)));
}

void Server::dealWithChallenge(int desc, int from, int to)
{
    if (desc == Player::Sent) {
	if (!playerExist(to) || !player(to)->isLoggedIn()) {
	    sendMessage(from, tr("That player is not online"));
	    //INVALID BEHAVIOR
	    return;
	}
	if (!player(to)->challenge(from)) {
	    sendMessage(from, tr("%1 is busy.").arg(name(to)));
	    return;
	}
    }  else {
	if (desc == Player::Accepted) {
	    startBattle(from, to);
	} else {
	    player(to)->sendChallengeStuff(desc, from);
	}
    }
}

void Server::startBattle(int id1, int id2)
{
    printLine(tr("Battle between %1 and %2 started").arg(name(id1)).arg(name(id2)));

    BattleSituation *battle = new BattleSituation(*player(id1), *player(id2));

    mybattles.insert(id1, battle);
    mybattles.insert(id2, battle);

    player(id1)->startBattle(id2, battle->pubteam(id1), battle->configuration());
    player(id2)->startBattle(id1, battle->pubteam(id2), battle->configuration());

    connect(battle, SIGNAL(battleInfo(int,QByteArray)), SLOT(sendBattleCommand(int, QByteArray)));
    connect(player(id1), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id2), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));

    battle->start();
}

void Server::battleResult(int desc, int winner, int loser)
{
    if (desc == Forfeit) {
	printLine( tr("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
	player(winner)->battleResult(Win);
    }
    removeBattle(winner, loser);
}

void Server::removeBattle(int winner, int loser)
{
    delete mybattles[winner];
    mybattles.remove(winner);
    mybattles.remove(loser);
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
	Player *p = player(id);
    
	QString playerName = p->name();
	bool loggedIn = p->isLoggedIn();

	delete p;

	myplayers.remove(id);

	/* Sending the notice of logout to others only if the player is already logged in */
	if (loggedIn)
	    sendLogout(id);

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

#include <ctime> /* for random numbers, time(NULL) needed */
#include "server.h"
#include "player.h"
#include "battle.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "playerswindow.h"
#include "security.h"
#include "antidos.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"

Server::Server(quint16 port)
{
    linecount = 0;

    QGridLayout *mylayout = new QGridLayout (this);

    QMenuBar *bar = new QMenuBar(this);
    QMenu *options = bar->addMenu("&Options");
    options->addAction("&Players", this, SLOT(openPlayers()));
    options->addAction("&Anti DoS", this, SLOT(openAntiDos()));
    mylayout->addWidget(bar,0,0,1,2);

    mylist = new QListWidget();
    mylayout->addWidget(mylist,1,0);

    mymainchat = new QTextEdit();
    mylayout->addWidget(mymainchat,1,1);

    mylist->setContextMenuPolicy(Qt::CustomContextMenu);
    mylist->setSortingEnabled(true);
    mylist->setFixedWidth(150);

    connect(mylist, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));

    mainchat()->setFixedWidth(500);
    mainchat()->setReadOnly(true);

    srand(time(NULL));

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

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

    MoveEffect::init();
    ItemEffect::init();
    AbilityEffect::init();

    printLine(tr("Move, abilities & items special effects loaded"));

    try {
        SecurityManager::init();
    } catch (const QString &ex) {
        printLine(ex);
    }

    AntiDos::obj()->init();

    printLine(tr("Members loaded"));

    if (!server()->listen(QHostAddress::Any, port))
    {
	printLine(tr("Unable to listen to port %1").arg(port));
    } else {
	printLine(tr("Starting to listen to port %1").arg(port));
    }

    connect(server(), SIGNAL(newConnection()), SLOT(incomingConnection()));
    connect(AntiDos::obj(), SIGNAL(kick(int)), SLOT(dosKick(int)));
    connect(AntiDos::obj(), SIGNAL(ban(int)), SLOT(dosBan(int)));
}

QTcpServer * Server::server()
{
    return &myserver;
}

void Server::printLine(const QString &line)
{
    if (linecount > 1000) {
        mainchat()->clear();
        printLine("Cleared the window (1000+ lines were displayed)");
    }

    mainchat()->moveCursor(QTextCursor::End);
    mainchat()->insertPlainText(line + "\n");
    qDebug() << line;
    linecount += 1;
}

void Server::openPlayers()
{
    PlayersWindow *w = new PlayersWindow();

    w->show();

    connect(w, SIGNAL(authChanged(QString,int)), SLOT(changeAuth(QString, int)));
    connect(w, SIGNAL(banned(QString)), SLOT(banName(QString)));
}

void Server::openAntiDos()
{
    AntiDosWindow *w = new AntiDosWindow();

    w->show();
}

void Server::banName(const QString &name) {
    if (mynames.contains(name)) {
        ban(mynames[name]);
    }
}

void Server::changeAuth(const QString &name, int auth) {
    qDebug() << "Change Auth called with " << name;
    if (mynames.contains(name)) {
        qDebug() << "contained";
        int id = mynames[name];
        player(id)->setAuth(auth);
        myplayersitems[id]->setText(authedName(id));
        sendPlayer(id);
    }
}

void Server::showContextMenu(const QPoint &p) {
    QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(list()->itemAt(p));

    if (item)
    {
        QMenu *menu = new QMenu(this);

        QSignalMapper *mymapper = new QSignalMapper(menu);
        QAction *viewinfo = menu->addAction("&Kick", mymapper, SLOT(map()));
        mymapper->setMapping(viewinfo, item->id());
        connect(mymapper, SIGNAL(mapped(int)), SLOT(kick(int)));

        QSignalMapper *mymapper2 = new QSignalMapper(menu);
        QAction *viewinfo2 = menu->addAction("&Ban", mymapper2, SLOT(map()));
        mymapper2->setMapping(viewinfo2, item->id());
        connect(mymapper2, SIGNAL(mapped(int)), SLOT(ban(int)));

        menu->exec(mapToGlobal(p));
    }
}

void Server::kick(int id) {
    kick(id, 0);
}

void Server::kick(int id, int src) {
    foreach(Player *p, myplayers)
    {
        p->relay().notify(NetworkServ::PlayerKick, qint32(id), qint32(src));
    }
    if (src == 0)
        printLine("The server kicked " + name(id) + "!");
    else
        printLine(name(id) + " was kicked by " + name(src));
    player(id)->kick();
}

void Server::ban(int id) {
    ban(id, 0);
}

void Server::ban(int id, int src) {
    foreach(Player *p, myplayers)
    {
        p->relay().notify(NetworkServ::PlayerBan, qint32(id), qint32(src));
    }
    if (src == 0)
        printLine("The server banned " + name(id) + "!");
    else
        printLine(name(id) + " was banned by " + name(src));
    SecurityManager::ban(name(id));
    player(id)->kick();
}

void Server::dosKick(int id) {
    if (playerExist(id)) {
        sendAll(tr("Player %1 (IP %2) is being overactive.").arg(name(id), player(id)->ip()));
    }
    kick(id);
}

void Server::dosBan(const QString &ip) {
    sendAll(tr("IP %1 is being overactive.").arg(ip));
    SecurityManager::ban(ip);
}

QString Server::authedName(int id) const
{
    QString nick = name(id);

    if (player(id)->auth() > 0 && player(id)->auth() < 4) {
        nick += ' ';

        for (int i = 0; i < player(id)->auth(); i++) {
            nick += '*';
        }
    }

    return QString("%1    %2").arg(id).arg(nick);
}

void Server::loggedIn(int id, const QString &name)
{
    printLine(tr("Player %1 logged in as %2").arg(id).arg(name));

    if (mynames.contains(name.toLower())) {
        printLine(tr("Name %1 already in use, disconnecting player %2").arg(name).arg(id));
        sendMessage(id, tr("Another with the name %1 is already logged in").arg(name));
        removePlayer(id);
        return;
    }

    player(id)->changeState(Player::LoggedIn);

    mynames.insert(name.toLower(), id);

    myplayersitems[id]->setText(authedName(id));

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

    QTcpSocket * newconnection = server()->nextPendingConnection();
    QString ip = newconnection->peerAddress().toString();

    if (SecurityManager::bannedIP(ip)) {
        printLine(tr("Banned IP %1 tried to log in.").arg(ip));
        delete newconnection;
        return;
    }

    if (!AntiDos::obj()->connecting(ip)) {
        printLine(tr("Anti DoS manager prevented IP %1 from logging in").arg(ip));
        delete newconnection;
        return;
    }

    printLine(tr("Received pending connection on slot %1 from %2").arg(id).arg(ip));
    myplayers[id] = new Player(newconnection, id);

    QIdListWidgetItem *it = new QIdListWidgetItem(id, QString::number(id));
    list()->addItem(it);
    myplayersitems[id] = it;

    connect(player(id), SIGNAL(loggedIn(int, QString)), this, SLOT(loggedIn(int, QString)));
    connect(player(id), SIGNAL(recvMessage(int, QString)), this, SLOT(recvMessage(int,QString)));
    connect(player(id), SIGNAL(recvTeam(int)), this, SLOT(recvTeam(int)));
    connect(player(id), SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(player(id), SIGNAL(challengeStuff(int,int,int)), SLOT(dealWithChallenge(int,int,int)));
    connect(player(id), SIGNAL(battleFinished(int,int,int)), SLOT(battleResult(int,int,int)));
    connect(player(id), SIGNAL(info(int,QString)), SLOT(info(int,QString)));
    connect(player(id), SIGNAL(playerKick(int,int)), SLOT(playerKick(int, int)));
    connect(player(id), SIGNAL(playerBan(int,int)), SLOT(playerBan(int, int)));
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

void Server::info(int id, const QString &mess) {
    printLine(QString("From Player %1: %2").arg(id).arg(mess));
}

void Server::playerKick(int src, int dest)
{
    if (!playerExist(dest))
        return;
    if (player(dest)->auth() >= player(src)->auth())
        return;
    kick(dest,src);
}

void Server::playerBan(int src, int dest)
{
    if (!playerExist(dest))
        return;
    if (player(dest)->auth() >= player(src)->auth())
        return;

    int maxauth = SecurityManager::maxAuth(player(dest)->relay().ip());

    if (player(src)->auth() <= maxauth) {
        player(src)->sendMessage("That player has authority level " + QString::number(maxauth) + " under another nick.");
        return;
    }

    ban(dest,src);
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
    connect(player(id1), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));
    connect(player(id2), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id2), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));

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
            relay.sendPlayer(p->id(), p->basicInfo(), p->auth());
    }
}

void Server::sendLogin(int id)
{
    foreach(Player *p, myplayers)
    {
	if (p->id() != id && p->isLoggedIn())
            p->relay().sendLogin(id, player(id)->basicInfo(), player(id)->auth());
    }
}

void Server::sendPlayer(int id)
{
    foreach(Player *p, myplayers)
    {
        if (p->isLoggedIn())
            p->relay().sendPlayer(id, player(id)->basicInfo(), player(id)->auth());
    }
}

void Server::sendLogout(int id)
{
    qDebug() << "Sending logout notice";
    foreach(Player *p, myplayers)
    {
	if (p->isLoggedIn())
	    p->relay().sendLogout(id);
    }
    qDebug() << "End sending";
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
	qDebug() << "Removing player " << player(id)->name();
	Player *p = player(id);
    
	QString playerName = p->name();
	bool loggedIn = p->isLoggedIn();
        AntiDos::obj()->disconnect(p->ip(), id);

	delete p;

	myplayers.remove(id);
        mynames.remove(playerName.toLower());

        delete list()->takeItem(list()->row(myplayersitems[id]));
        myplayersitems.remove(id);

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
    for (QHash<int, Player*>::const_iterator it = myplayers.begin(); it != myplayers.end(); ++it)
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

QListWidget * Server::list() {
    return mylist;
}

Player * Server::player(int id)
{
    return myplayers[id];
}

const Player * Server::player(int id) const
{
    return myplayers[id];
}

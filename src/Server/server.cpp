#include <ctime> /* for random numbers, time(NULL) needed */
#include "server.h"
#include "player.h"
#include "challenge.h"
#include "battle.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "playerswindow.h"
#include "security.h"
#include "antidos.h"
#include "serverconfig.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "scriptengine.h"

Server::Server(quint16 port)
{
    linecount = 0;
    registry_connection = NULL;

    QGridLayout *mylayout = new QGridLayout (this);

    QMenuBar *bar = new QMenuBar(this);
    QMenu *options = bar->addMenu("&Options");
    options->addAction("&Players", this, SLOT(openPlayers()));
    options->addAction("&Anti DoS", this, SLOT(openAntiDos()));
    options->addAction("&Config", this, SLOT(openConfig()));
    options->addAction("&Scripts", this, SLOT(openScriptWindow()));
    mylayout->addWidget(bar,0,0,1,2);

    mylist = new QListWidget();
    mylayout->addWidget(mylist,1,0,2,1);

    mymainchat = new QScrollDownTextEdit();
    mylayout->addWidget(mymainchat,1,1);

    myline = new QLineEdit();
    mylayout->addWidget(myline, 2,1);

    mylist->setContextMenuPolicy(Qt::CustomContextMenu);
    mylist->setSortingEnabled(true);
    mylist->setFixedWidth(150);

    connect(mylist, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendServerMessage()));

    mainchat()->setFixedWidth(500);

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

    myengine = new ScriptEngine(this);

    if (!server()->listen(QHostAddress::Any, port))
    {
	printLine(tr("Unable to listen to port %1").arg(port));
    } else {
	printLine(tr("Starting to listen to port %1").arg(port));
    }

    connect(server(), SIGNAL(newConnection()), SLOT(incomingConnection()));
    connect(AntiDos::obj(), SIGNAL(kick(int)), SLOT(dosKick(int)));
    connect(AntiDos::obj(), SIGNAL(ban(QString)), SLOT(dosBan(QString)));

    QSettings s;
    serverName = s.value("server_name").toString();
    serverDesc = s.value("server_description").toString();

    myengine->serverStartUp();
    connectToRegistry();
}

QTcpServer * Server::server()
{
    return &myserver;
}

void Server::connectToRegistry()
{
    if (registry_connection != NULL) {
        if (registry_connection->isConnected()) {
            return;
        }
        else
            registry_connection->deleteLater();;
    }

    printLine("Connecting to registry...");

    QTcpSocket * s = new QTcpSocket(NULL);
    s->connectToHost("pokeymon.zapto.org", 5082);

    connect(s, SIGNAL(connected()), this, SLOT(regConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(regConnectionError()));

    registry_connection = new Analyzer(s,0);
}

void Server::regConnectionError()
{
    printLine("Error when connecting to the registry. Will restart in 30 seconds");
    QTimer::singleShot(30000, this, SLOT(connectToRegistry()));
}

void Server::regConnected()
{
    printLine("Connected to registry! Sending server info...");
    registry_connection->notify(NetworkServ::Login, serverName, serverDesc, numPlayers());
    connect(registry_connection, SIGNAL(ipRefused()), SLOT(ipRefused()));
    connect(registry_connection, SIGNAL(invalidName()), SLOT(invalidName()));
    connect(registry_connection, SIGNAL(nameTaken()), SLOT(nameTaken()));
    connect(registry_connection, SIGNAL(accepted()), SLOT(accepted()));

    /* Sending Players at regular interval */
    QTimer::singleShot(2500, this, SLOT(regSendPlayers()));
}

void Server::regSendPlayers()
{
    if (registry_connection == NULL || !registry_connection->isConnected())
        return;
    registry_connection->notify(NetworkServ::ServNumChange, numPlayers());
    /* Sending Players at regular interval */
    QTimer::singleShot(2500, this, SLOT(regSendPlayers()));
}

void Server::regNameChanged(const QString &name)
{
    serverName = name;
    sendAll("The name of the server changed to " + name + ".");

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServNameChange, name);
}

void Server::regDescChanged(const QString &desc)
{
    serverDesc = desc;
    printLine("The description of the server changed.");

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServDescChange, desc);
}

void Server::accepted()
{
    printLine("The registry acknowledged the server.");
}

void Server::invalidName()
{
    printLine("Invalid name for the registry. Please change it in server->config");
}

void Server::nameTaken()
{
    printLine("The name of the server is already in use. Please change it in server->config");
}


void Server::ipRefused()
{
    printLine("Registry wants only 1 server per IP");
}

void Server::printLine(const QString &line)
{
    mainchat()->insertPlainText(line + "\n");
    qDebug() << line;
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

void Server::openConfig()
{
    ServerWindow *w = new ServerWindow();

    w->show();

    connect(w, SIGNAL(nameChanged(QString)), SLOT(regNameChanged(const QString)));
    connect(w, SIGNAL(descChanged(QString)), SLOT(regDescChanged(const QString)));
}

void Server::openScriptWindow()
{
    myscriptswindow = new ScriptWindow();

    myscriptswindow->show();

    connect(myscriptswindow, SIGNAL(scriptChanged(QString)), myengine, SLOT(changeScript(QString)));
}

void Server::banName(const QString &name) {
    if (nameExist(name)) {
        ban(id(name));
    }
}

void Server::changeAuth(const QString &name, int auth) {
    if (nameExist(name)) {
        int id = this->id(name);
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

        QSignalMapper *mymapper3 = new QSignalMapper(menu);
        QAction *viewinfo = menu->addAction("&Silent Kick", mymapper3, SLOT(map()));
        mymapper3->setMapping(viewinfo, item->id());
        connect(mymapper3, SIGNAL(mapped(int)), SLOT(silentKick(int)));

        QSignalMapper *mymapper = new QSignalMapper(menu);
        viewinfo = menu->addAction("&Kick", mymapper, SLOT(map()));
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

void Server::silentKick(int id) {
    if (playerExist(id))
        player(id)->kick();
}

void Server::kick(int id, int src) {
    if (!playerExist(id))
        return;
    foreach(Player *p, myplayers)
    {
        p->relay().notify(NetworkServ::PlayerKick, qint32(id), qint32(src));
    }
    if (src == 0)
        printLine("The server kicked " + name(id) + "!");
    else
        printLine(name(id) + " was kicked by " + name(src));
    silentKick(id);
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
    printLine(tr("Player %1 set name to %2").arg(id).arg(name));


    if (nameExist(name)) {
        int ids = this->id(name);
        if (!playerExist(ids) || !player(ids)->isLoggedIn() || player(ids)->name().toLower() != name.toLower()) {
            printLine(QString("Critical Bug needing to be solved (kept a name too much in the name list: %1)").arg(name));
            mynames.remove(name.toLower());
        } else {
            printLine(tr("Name %1 already in use, disconnecting player %2").arg(name).arg(id));
            sendMessage(id, tr("Another with the name %1 is already logged in").arg(name));
            removePlayer(id);
            return;
        }
    }

    /* For new connections */
    if (!player(id)->isLoggedIn()) {        
        mynames.insert(name.toLower(), id);
        myplayersitems[id]->setText(authedName(id));

        if(!myengine->beforeLogIn(id)) {
            removePlayer(id);
            return;
        }

        player(id)->changeState(Player::LoggedIn, true);

        sendPlayersList(id);
        sendLogin(id);

        sendMessage(id, tr("Welcome Message: The updates are now available at www.pokeymon-online.com."));

        myengine->afterLogIn(id);
    } else { /* if already logged in */
        recvTeam(id, name);
    }
}

void Server::sendBattleCommand(int id, const QByteArray &comm)
{
    player(id)->relay().sendBattleCommand(comm);
}

void Server::sendMessage(int id, const QString &message)
{
    player(id)->sendMessage(message);
}

void Server::sendServerMessage()
{
    if (myline->text().trimmed().length() == 0) {
        return;
    }
    sendAll("~~Server~~: " + myline->text());
    myline->clear();
}

void Server::recvMessage(int id, const QString &mess)
{
    QString re = mess.trimmed();
    if (re.length() > 0) {
        if (myengine->beforeChatMessage(id, mess)) {
            sendAll(tr("%1: %2").arg(name(id)).arg(re));
            myengine->afterChatMessage(id, mess);
        }
    }
}

void Server::recvPM(int src, int dest, const QString &mess)
{
    if (playerExist(dest) && player(dest)->isLoggedIn()) {
        player(dest)->relay().sendPM(src, mess);
    }
}

QString Server::name(int id) const
{
    if (playerExist(id))
        return player(id)->name();
    else
        return "";
}

int Server::auth(int id) const
{
    return player(id)->auth();
}

void Server::incomingConnection()
{
    int id = freeid();

    QTcpSocket * newconnection = server()->nextPendingConnection();
    QString ip = newconnection->peerAddress().toString();

    if (SecurityManager::bannedIP(ip)) {
        printLine(tr("Banned IP %1 tried to log in.").arg(ip));
        newconnection->deleteLater();;
        return;
    }

    if (!AntiDos::obj()->connecting(ip)) {
        printLine(tr("Anti DoS manager prevented IP %1 from logging in").arg(ip));
        newconnection->deleteLater();
        return;
    }

    printLine(tr("Received pending connection on slot %1 from %2").arg(id).arg(ip));
    myplayers[id] = new Player(newconnection, id);

    QIdListWidgetItem *it = new QIdListWidgetItem(id, QString::number(id));
    list()->addItem(it);
    myplayersitems[id] = it;

    Player *p = player(id);

    connect(p, SIGNAL(loggedIn(int, QString)), this, SLOT(loggedIn(int, QString)));
    connect(p, SIGNAL(recvTeam(int, QString)), this, SLOT(recvTeam(int, QString)));
    connect(p, SIGNAL(recvMessage(int, QString)), this, SLOT(recvMessage(int,QString)));
    connect(p, SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(p, SIGNAL(sendChallenge(int,int,ChallengeInfo)), SLOT(dealWithChallenge(int,int,ChallengeInfo)));
    connect(p, SIGNAL(battleFinished(int,int,int)), SLOT(battleResult(int,int,int)));
    connect(p, SIGNAL(info(int,QString)), SLOT(info(int,QString)));
    connect(p, SIGNAL(playerKick(int,int)), SLOT(playerKick(int, int)));
    connect(p, SIGNAL(playerBan(int,int)), SLOT(playerBan(int, int)));
    connect(p, SIGNAL(PMReceived(int,int,QString)), this, SLOT(recvPM(int,int,QString)));
    connect(p, SIGNAL(awayChange(int,bool)), this, SLOT(awayChanged(int, bool)));
}

void Server::awayChanged(int src, bool away)
{
    foreach (Player *p, myplayers) {
        if (p->isLoggedIn()) {
            p->relay().notifyAway(src, away);
        }
    }
}

void Server::dealWithChallenge(int from, int to, const ChallengeInfo &c)
{
    if (!playerExist(to) || !player(to)->isLoggedIn()) {
        sendMessage(from, tr("That player is not online"));
        //INVALID BEHAVIOR
        return;
    }
    try {
        Challenge *_c = new Challenge(player(from), player(to), c, this);
        connect(_c, SIGNAL(battleStarted(int,int,ChallengeInfo)), SLOT(startBattle(int, int, ChallengeInfo)));
    } catch (Challenge::Exception) {
        ;
    }
}

void Server::beforeChallengeIssued(int src, int dest, Challenge *c)
{
    if (!myengine->beforeChallengeIssued(src, dest, c->description())) {
        c->cancelFromServer();
    }
}

void Server::afterChallengeIssued(int src, int dest, Challenge *c)
{
    myengine->afterChallengeIssued(src, dest, c->description());
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


void Server::startBattle(int id1, int id2, const ChallengeInfo &c)
{
    printLine(tr("Battle between %1 and %2 started").arg(name(id1)).arg(name(id2)));

    BattleSituation *battle = new BattleSituation(*player(id1), *player(id2), c);

    mybattles.insert(id1, battle);
    mybattles.insert(id2, battle);

    player(id1)->startBattle(id2, battle->pubteam(id1), battle->configuration());
    player(id2)->startBattle(id1, battle->pubteam(id2), battle->configuration());

    foreach(Player *p, myplayers) {
        if (p->isLoggedIn() && p->id() != id1 && p->id() != id2) {
            p->relay().notifyBattle(id1,id2);
        }
    }

    connect(battle, SIGNAL(battleInfo(int,QByteArray)), SLOT(sendBattleCommand(int, QByteArray)));
    connect(battle, SIGNAL(battleFinished(int,int,int)), SLOT(battleResult(int,int,int)));
    connect(player(id1), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id1), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));
    connect(player(id2), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id2), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));

    battle->start();
}

void Server::battleResult(int desc, int winner, int loser)
{
    if (desc == Forfeit && mybattles[winner]->finished()) {
        player(winner)->battleResult(Close, winner, loser);
        player(loser)->battleResult(Close, winner, loser);
    } else {
        foreach(Player *p, myplayers) {
            if (p->isLoggedIn()) {
                p->battleResult(desc, winner, loser);
            }
        }
    }

    if (desc == Forfeit) {
        if (!mybattles[winner]->finished())
            printLine( tr("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
    } else if (desc == Win) {
        printLine( tr("%1 won his battle against %2").arg(name(winner), name(loser)));
    } else if (desc == Tie) {
        printLine( tr("%1 and %2 tied").arg(name(winner), name(loser)));
    }

    if (desc == Forfeit) {
        removeBattle(winner, loser);
    }
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
            relay.sendPlayer(p->bundle());
    }
}

void Server::sendLogin(int id)
{
    PlayerInfo bundle = player(id)->bundle();

    foreach(Player *p, myplayers)
    {
	if (p->id() != id && p->isLoggedIn())
            p->relay().sendLogin(bundle);
    }
}

void Server::sendPlayer(int id)
{
    PlayerInfo bundle = player(id)->bundle();

    foreach(Player *p, myplayers)
    {
        if (p->isLoggedIn())
            p->relay().sendPlayer(bundle);
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

void Server::recvTeam(int id, const QString &_name)
{
    printLine(tr("%1 changed their team, and their name to %2").arg(name(id), _name));

    /* Normally all checks have been made to ensure the authentification is right and the
       name isn't taken.

       That's done by calling loggedIn() before to test some things if needed.

       So here all i have to do is delete the old name */
    QString oldname = player(id)->name();

    if (oldname == _name) {
        /* Haha, same name so no need to do anything! */

    } else {
        /* Changing the name! */
        mynames.remove(oldname.toLower());
        mynames.insert(_name.toLower(), id);
        player(id)->setName(_name);
    }

    PlayerInfo bundle = player(id)->bundle();

    /* Sending the team change! */
    foreach(Player *p, myplayers) {
        if (p->isLoggedIn()) {
            p->relay().sendTeamChange(bundle);
        }
    }

    /* Displaying the change */
    myplayersitems[id]->setText(authedName(id));
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

bool Server::playerLoggedIn(int id) const
{
    return playerExist(id) && player(id)->isLoggedIn();
}

void Server::removePlayer(int id)
{
    if (playerExist(id))
    {        
	Player *p = player(id);
        bool loggedIn = p->isLoggedIn();

        if (loggedIn) {
            myengine->beforeLogOut(id);
        }

        p->doWhenDC();

        p->blockSignals(true);

	QString playerName = p->name();

        AntiDos::obj()->disconnect(p->ip(), id);

	delete p;

	myplayers.remove(id);
        if (loggedIn)
            mynames.remove(playerName.toLower());

        delete list()->takeItem(list()->row(myplayersitems[id]));
        myplayersitems.remove(id);

	/* Sending the notice of logout to others only if the player is already logged in */
        if (loggedIn) {
	    sendLogout(id);
            myengine->afterLogOut(id);
        }

	printLine(tr("Removed player %1").arg(playerName));
    }
}

bool Server::nameExist(const QString &name) const
{
    return mynames.contains(name.toLower());
}

int Server::id(const QString &name) const
{
    return mynames.value(name.toLower());
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
    for (int i = 1; ; i++) {
        if (!myplayers.contains(i)) {
            return i;
        }
    }
}

QScrollDownTextEdit * Server::mainchat()
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

Player * Server::player(int id) const
{
    return myplayers[id];
}

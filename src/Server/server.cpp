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
#include "../PokemonInfo/movesetchecker.h"
#include "../Utilities/otherwidgets.h"
#include "scriptengine.h"
#include "../Shared/config.h"
#include "tiermachine.h"
#include "battlingoptions.h"
#include "sql.h"
#include "sqlconfig.h"

Server *Server::serverIns = NULL;

Server::Server(quint16 port)
{
    try {
    serverIns = this;

    linecount = 0;
    registry_connection = NULL;
    myengine = NULL;

    QGridLayout *mylayout = new QGridLayout (this);

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

    mainchat()->setMinimumWidth(500);

    srand(time(NULL));

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QSettings s;

    if (!s.contains("sql_driver")) {
        s.setValue("sql_driver", SQLCreator::SQLite);
    }
    if (!s.contains("sql_db_name")) {
        s.setValue("sql_db_name", "pokemon");
    }
    if (!s.contains("sql_db_port")) {
        s.setValue("sql_db_port", 5432);
    }
    if (!s.contains("sql_db_user")) {
        s.setValue("sql_db_user", "postgres");
    }
    if (!s.contains("sql_db_pass")) {
        s.setValue("sql_db_pass", "admin");
    }
    if (!s.contains("sql_db_host")) {
        s.setValue("sql_db_host", "localhost");
    }

    try {
        SQLCreator::createSQLConnection();
    } catch (const QString &ex) {
        printLine(ex);
    }

    printLine(tr("Starting loading pokemon database..."));

    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");

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

    TierMachine::init();

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

    serverPort = port;

    if (s.value("battles_with_same_ip_unrated").isNull()) {
        s.setValue("battles_with_same_ip_unrated", true);
    }
    if (s.value("rated_battles_memory_number").isNull()) {
        s.setValue("rated_battles_memory_number", 5);
    }

    loadRatedBattlesSettings();

    /*
      The timer for clearing the last rated battles memory, set to 3 hours
     */
    QTimer *t= new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(clearRatedBattlesHistory()));
    t->start(3*3600*1000);

    serverName = s.value("server_name").toString();
    serverDesc = s.value("server_description").toString();
    serverAnnouncement = s.value("server_announcement").toString();
    serverPlayerMax = quint16(s.value("server_maxplayers").toInt());
    serverPrivate = quint16(s.value("server_private").toInt());

    myengine->serverStartUp();

    if (serverPrivate != 1)
        connectToRegistry();
} catch (const QString &e) {
    qDebug() << "Exception" << e;
}
}

QMenuBar* Server::createMenuBar() {
    QMenuBar *bar = new QMenuBar(this);
    QMenu *options = bar->addMenu("&Options");
    options->addAction("&Players", this, SLOT(openPlayers()));
    options->addAction("&Anti DoS", this, SLOT(openAntiDos()));
    options->addAction("&Config", this, SLOT(openConfig()));
    options->addAction("&Scripts", this, SLOT(openScriptWindow()));
    options->addAction("&Tiers", this, SLOT(openTiersWindow()));
    options->addAction("&Battle Config", this, SLOT(openBattleConfigWindow()));
    options->addAction("&SQL Config", this, SLOT(openSqlConfigWindow()));
    return bar;
}


void Server::print(const QString &line)
{
    serverIns->printLine(line, false);
}

QTcpServer * Server::server()
{
    return &myserver;
}

void Server::loadRatedBattlesSettings()
{
    QSettings s;
    allowRatedWithSameIp = !s.value("battles_with_same_ip_unrated").toBool();
    diffIpsForRatedBattles = s.value("rated_battles_memory_number").toInt();
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
    s->connectToHost("pokemon-online.dynalias.net", 5082);

    connect(s, SIGNAL(connected()), this, SLOT(regConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(regConnectionError()));

    registry_connection = new Analyzer(s,0);
}

void Server::disconnectFromRegistry()
{
    registry_connection->deleteLater();
    printLine("Disconnected from registry.");
    registry_connection = NULL;
}


void Server::regPrivacyChanged(const int &priv)
{
    if (serverPrivate == priv)
        return;

    serverPrivate = priv;

    if (serverPrivate == 1)
    {
        printLine("The server is now private.");
        disconnectFromRegistry();
    }
    else
    {
        printLine("The server is now public.");
        connectToRegistry();
    }
}

void Server::regConnectionError()
{
    printLine("Error when connecting to the registry. Will restart in 30 seconds");
    QTimer::singleShot(30000, this, SLOT(connectToRegistry()));
}

void Server::regConnected()
{
    printLine("Connected to registry! Sending server info...");
    registry_connection->notify(NetworkServ::Login, serverName, serverDesc, quint16(AntiDos::obj()->numberOfDiffIps()), serverPlayerMax, serverPort);
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
    registry_connection->notify(NetworkServ::ServNumChange, quint16(AntiDos::obj()->numberOfDiffIps()));
    /* Sending Players at regular interval */
    QTimer::singleShot(2500, this, SLOT(regSendPlayers()));
}

void Server::regNameChanged(const QString &name)
{
    if (serverName == name)
        return;

    serverName = name;
    sendAll("The name of the server changed to " + name + ".");

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServNameChange, name);
}

void Server::regDescChanged(const QString &desc)
{
    if (serverDesc == desc)
        return;

    serverDesc = desc;
    printLine("The description of the server changed.");

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServDescChange, desc);
}

void Server::regMaxChanged(const int &numMax)
{
    if (numMax == serverPlayerMax)
        return;

    serverPlayerMax = numMax;
    printLine("Maximum Players Changed.");

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServMaxChange,numMax);
}

void Server::announcementChanged(const QString &announcement)
{
    if (announcement == serverAnnouncement)
        return;

    serverAnnouncement = announcement;

    printLine("Announcement changed.");
}

void Server::clearRatedBattlesHistory()
{
    printLine("Auto Clearing the last rated battles history (every 3 hours)");
    lastRatedIps.clear();
}

void Server::accepted()
{
    printLine("The registry acknowledged the server.");
}

void Server::invalidName()
{
    printLine("Invalid name for the registry. Please change it in Options -> Config.");
}

void Server::nameTaken()
{
    printLine("The name of the server is already in use. Please change it in Options -> Config.");
}


void Server::ipRefused()
{
    printLine("Registry wants only 1 server per IP");
}


/* Returns false if the event "newMessage" was stopped (nothing to do with "chatMessage" */
bool Server::printLine(const QString &line, bool chatMessage)
{
    if (myengine == NULL) {
        mainchat()->insertPlainText(line + "\n");
        qDebug() << line;
        return true;
    }
    if (chatMessage || myengine->beforeNewMessage(line)) {
        mainchat()->insertPlainText(line + "\n");
        qDebug() << line;
        if (!chatMessage)
            myengine->afterNewMessage(line);
        return true;
    }
    return false;
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
    connect(w, SIGNAL(maxChanged(int)), SLOT(regMaxChanged(int)));
    connect(w, SIGNAL(privacyChanged(int)), SLOT(regPrivacyChanged(int)));
    connect(w, SIGNAL(announcementChanged(QString)), SLOT(announcementChanged(QString)));
}

void Server::openScriptWindow()
{
    myscriptswindow = new ScriptWindow();

    myscriptswindow->show();

    connect(myscriptswindow, SIGNAL(scriptChanged(QString)), myengine, SLOT(changeScript(QString)));
}

void Server::openTiersWindow()
{
    TierWindow *w = new TierWindow();

    w->show();

    connect(w, SIGNAL(tiersChanged()), SLOT(tiersChanged()));
}

void Server::openBattleConfigWindow()
{
    BattlingOptionsWindow *w = new BattlingOptionsWindow();

    w->show();

    connect(w, SIGNAL(settingsChanged()), SLOT(loadRatedBattlesSettings()));
}

void Server::openSqlConfigWindow()
{
    SQLConfigWindow *w = new SQLConfigWindow();

    w->show();
}

void Server::tiersChanged()
{
    sendAll("Tiers have been updated!");

    foreach(Player *p, myplayers) {
        sendTierList(p->id());
    }

    foreach(Player *p, myplayers) {
        if (!TierMachine::obj()->isValid(p->team(),p->tier())) {
            p->findTierAndRating();
            if (p->isLoggedIn()) {
                sendPlayer(p->id());
            }
        }
    }
}

void Server::banName(const QString &name) {
    if (nameExist(name)) {
        ban(id(name));
    }
}

void Server::changeAuth(const QString &name, int auth) {
    if (nameExist(name)) {
        int id = this->id(name);
        if (auth == player(id)->auth())
            return;
        player(id)->setAuth(auth);
        myplayersitems[id]->setText(authedName(id));
        if (SecurityManager::member(name).authority() != auth) {
            SecurityManager::setauth(name, auth);
        }
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
        if (p->isLoggedIn())
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

        if(!myengine->beforeLogIn(id) || !playerExist(id)) {
            removePlayer(id);
            return;
        }

        player(id)->changeState(Player::LoggedIn, true);

        player(id)->relay().notify(NetworkServ::VersionControl, VERSION);

        if (serverAnnouncement.length() > 0)
            player(id)->relay().notify(NetworkServ::Announcement, serverAnnouncement);

        sendTierList(id);

        sendPlayersList(id);

        sendLogin(id);

        sendMessage(id, tr("Welcome Message: The updates are now available at www.pokemon-online.eu. Report any bug on the forums."));

        myengine->afterLogIn(id);
    } else { /* if already logged in */
        recvTeam(id, name);
    }
}

void Server::sendTierList(int id)
{
    player(id)->relay().notify(NetworkServ::TierSelection, TierMachine::obj()->tierList());
}

void Server::sendBattleCommand(int publicId, int id, const QByteArray &comm)
{
    /* As things are threaded, the player may have logged off before
       receiving the command.

       An extreme thing that might crash the client is a player logging off,
       and another logging on and starting a battle right away with the same player id
       and same battle id */
    if (!playerExist(id))
        return;

    if (player(id)->battling() && player(id)->battleId() == publicId)
        player(id)->relay().sendBattleCommand(comm);
    else {
        if (player(id)->battlesSpectated.contains(publicId))
            player(id)->relay().sendWatchingCommand(publicId, comm);
    }
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

void Server::spectatingChat(int player, int battle, const QString &chat)
{
    if (!mybattles.contains(battle)) {
        return;
    }
    mybattles[battle]->spectatingChat(player, chat);
}

void Server::recvMessage(int id, const QString &mess)
{
    QString re = mess.trimmed();
    if (re.length() > 0) {
        if (myengine->beforeChatMessage(id, mess)) {
            sendAll(tr("%1: %2").arg(name(id)).arg(re), true);
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

    if (numPlayers() >= serverPlayerMax && serverPlayerMax != 0){
        printLine(tr("Stopped IP %1 from logging in, server full.").arg(ip));
        newconnection->deleteLater();
        return;
    }

    if (SecurityManager::bannedIP(ip)) {
        printLine(tr("Banned IP %1 tried to log in.").arg(ip));
        newconnection->deleteLater();
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

    connect(p, SIGNAL(loggedIn(int, QString)), SLOT(loggedIn(int, QString)));
    connect(p, SIGNAL(recvTeam(int, QString)), SLOT(recvTeam(int, QString)));
    connect(p, SIGNAL(recvMessage(int, QString)), SLOT(recvMessage(int,QString)));
    connect(p, SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(p, SIGNAL(sendChallenge(int,int,ChallengeInfo)), SLOT(dealWithChallenge(int,int,ChallengeInfo)));
    connect(p, SIGNAL(battleFinished(int,int,int,bool,const QString&)), SLOT(battleResult(int,int,int,bool, const QString&)));
    connect(p, SIGNAL(info(int,QString)), SLOT(info(int,QString)));
    connect(p, SIGNAL(playerKick(int,int)), SLOT(playerKick(int, int)));
    connect(p, SIGNAL(playerBan(int,int)), SLOT(playerBan(int, int)));
    connect(p, SIGNAL(PMReceived(int,int,QString)), SLOT(recvPM(int,int,QString)));
    connect(p, SIGNAL(awayChange(int,bool)), this, SLOT(awayChanged(int, bool)));
    connect(p, SIGNAL(spectatingRequested(int,int)), SLOT(spectatingRequested(int,int)));
    connect(p, SIGNAL(spectatingStopped(int,int)), SLOT(spectatingStopped(int,int)));
    connect(p, SIGNAL(spectatingChat(int,int, QString)), SLOT(spectatingChat(int,int, QString)));
    connect(p, SIGNAL(updated(int)), SLOT(sendPlayer(int)));
    connect(p, SIGNAL(findBattle(int,FindBattleData)), SLOT(findBattle(int, FindBattleData)));
    connect(p, SIGNAL(battleSearchCancelled(int)), SLOT(cancelSearch(int)));
}

void Server::awayChanged(int src, bool away)
{
    if (myengine->beforePlayerAway(src, away)) {
        if (!playerExist(src))
            return;
        foreach (Player *p, myplayers) {
            if (p->isLoggedIn()) {
                p->relay().notifyAway(src, away);
            }
        }
        myengine->afterPlayerAway(src, away);
    }
}

void Server::cancelSearch(int id)
{
    player(id)->battleSearch() = false;
    delete battleSearchs.take(id);
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

void Server::findBattle(int id, const FindBattleData &f)
{
    printLine(tr("%1 initiated a battle search.").arg(name(id)));

    Player *p1 = player(id);

    QHash<int, FindBattleData*>::iterator it;
    for(it = battleSearchs.begin(); it != battleSearchs.end(); ++it)
    {
        int key = it.key();
        FindBattleData *data = it.value();
        Player *p2 = player(key);

        /* First look if this not a repeat */
        if (p2->lastFindBattleIp() == p1->ip() || p1->lastFindBattleIp() == p2->ip()) {
            continue;
        }

        /* We check the tier thing */
        if ( (f.sameTier || data->sameTier) && p1->tier() != p2->tier() )
            continue;

        /* The double battle thing */
        if ( f.mode != data->mode) {
            continue;
        }

        /* We check both allow rated if needed */
        if (f.rated || data->rated) {
            if (!canHaveRatedBattle(id, key, p1->tier() == "Challenge Cup", f.rated, data->rated))
                continue;
        }

        /* Then the range thing */
        if (f.ranged)
            if (p1->rating() - f.range > p2->rating() || p1->rating() + f.range < p2->rating() )
                continue;
        if (data->ranged)
            if (p1->rating() - data->range > p2->rating() || p1->rating() + data->range < p2->rating() )
                continue;

        //We have a match!
        ChallengeInfo c;
        c.opp = key;
        c.rated = (p1->ladder() && p2->ladder() && p1->tier() == p2->tier()) || f.rated || data->rated;

        if (p1->tier() == p2->tier() && p1->tier() == "Challenge Cup") {
            c.clauses = ChallengeInfo::ChallengeCup;
        } else if (p1->tier() == p2->tier() && p1->tier() == "VGC") {
            c.clauses = ChallengeInfo::SpeciesClause;
        } else {
            c.clauses = ChallengeInfo::SleepClause | ChallengeInfo::EvasionClause | ChallengeInfo::OHKOClause | ChallengeInfo::SpeciesClause;
        }

        c.mode = f.mode;

        if (myengine->beforeBattleMatchup(id,key,c)) {
            player(id)->lastFindBattleIp() = player(key)->ip();
            player(key)->lastFindBattleIp() = player(id)->ip();
            startBattle(id,key,c);
            myengine->afterBattleMatchup(id,key,c);
            return;
        }
    }

    /* Not reached if a match was found */
    battleSearchs.insert(id, new FindBattleData(f));
    p1->battleSearch() = true;
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
    if (myengine->beforePlayerKick(src, dest)) {
        if (!playerExist(src) || !playerExist(dest))
            return;
        kick(dest,src);
        myengine->afterPlayerKick(src, dest);
    }
}

void Server::playerBan(int src, int dest)
{
    if (!playerExist(dest))
        return;
    if (player(dest)->auth() >= player(src)->auth())
        return;

    int maxauth = SecurityManager::maxAuth(player(dest)->relay().ip());

    if (player(src)->auth() <= maxauth) {
        player(src)->sendMessage("That player has authority level superior or equal to yours under another nick.");
        return;
    }

    if (myengine->beforePlayerBan(src, dest)) {
        if (!playerExist(src) || !playerExist(dest))
            return;
        ban(dest,src);
        myengine->afterPlayerBan(src, dest);
    }
}


void Server::startBattle(int id1, int id2, const ChallengeInfo &c)
{
    myengine->beforeBattleStarted(id1,id2,c);

    if (!playerExist(id1) || !playerExist(id2))
        return;

    printLine(tr("Battle between %1 and %2 started").arg(name(id1)).arg(name(id2)));

    /* Storing the battle in the last ips to battle */
    if (c.rated && diffIpsForRatedBattles > 0) {
        QList<QString> &lastIps1 = lastRatedIps[player(id1)->ip()];

        lastIps1.push_back(player(id2)->ip());
        if (lastIps1.size() > diffIpsForRatedBattles) {
            lastIps1.pop_front();
        }

        QList<QString> &lastIps2 = lastRatedIps[player(id2)->ip()];

        lastIps1.push_back(player(id1)->ip());
        if (lastIps2.size() > diffIpsForRatedBattles) {
            lastIps2.pop_front();
        }
    }

    int id = freebattleid();

    BattleSituation *battle = new BattleSituation(*player(id1), *player(id2), c, id);
    mybattles.insert(id, battle);

    player(id1)->startBattle(id2, battle->pubteam(id1), battle->configuration(), battle->doubles());
    player(id2)->startBattle(id1, battle->pubteam(id2), battle->configuration(), battle->doubles());

    foreach(Player *p, myplayers) {
        if (p->isLoggedIn() && p->id() != id1 && p->id() != id2) {
            p->relay().notifyBattle(id1,id2);
        }
    }

    connect(battle, SIGNAL(battleInfo(int,int,QByteArray)), SLOT(sendBattleCommand(int,int, QByteArray)));
    connect(battle, SIGNAL(battleFinished(int,int,int,bool,QString)), SLOT(battleResult(int,int,int,bool,QString)));
    connect(player(id1), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id1), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));
    connect(player(id2), SIGNAL(battleMessage(int,BattleChoice)), battle, SLOT(battleChoiceReceived(int,BattleChoice)));
    connect(player(id2), SIGNAL(battleChat(int,QString)), battle, SLOT(battleChat(int, QString)));

    battle->start();

    myengine->afterBattleStarted(id1,id2,c);
}

bool Server::canHaveRatedBattle(int id1, int id2, bool cc, bool force1, bool force2)
{
    Player *p1 = player(id1);
    Player *p2 = player(id2);
    if (!force1 && !p1->ladder())
        return false;
    if (!force2 && !p2->ladder())
        return false;
    if (p1->tier() != p2->tier())
        return false;
    if (cc != (p1->tier() == "Challenge Cup")) {
        return false;
    }
    if (!allowRatedWithSameIp && p1->ip() == p2->ip())
        return false;
    if (diffIpsForRatedBattles > 0) {
        QList<QString> l1 = lastRatedIps.value(p1->ip());
        if (l1.contains(p2->ip()))
            return false;
        QList<QString> l2 = lastRatedIps.value(p2->ip());
        if (l2.contains(p1->ip()))
            return false;
    }
    return true;
}

void Server::battleResult(int desc, int winner, int loser, bool rated, const QString &tier)
{
    if (desc == Forfeit && player(winner)->battle->finished()) {
        player(winner)->battleResult(Close, winner, loser);
        player(loser)->battleResult(Close, winner, loser);
        foreach(int id, player(winner)->battle->getSpectators()) {
            player(id)->battleResult(Close, winner, loser);
        }
    } else {
        if (desc != Tie && rated) {
            QString winn = player(winner)->name();
            QString lose = player(loser)->name();
            TierMachine::obj()->changeRating(winn, lose, tier);
            player(winner)->rating() = TierMachine::obj()->rating(winn, tier);
            player(loser)->rating() = TierMachine::obj()->rating(lose, tier);
            sendPlayer(winner);
            sendPlayer(loser);
        }
        myengine->beforeBattleEnded(winner, loser, desc);
        foreach(Player *p, myplayers) {
            if (p->isLoggedIn()) {
                p->battleResult(desc, winner, loser);
            }
        }

        if (desc == Forfeit) {
            printLine( tr("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
        } else if (desc == Win) {
            printLine( tr("%1 won his battle against %2").arg(name(winner), name(loser)));
        } else if (desc == Tie) {
            printLine( tr("%1 and %2 tied").arg(name(winner), name(loser)));
        }
        myengine->afterBattleEnded(winner, loser, desc);
    }


    if (desc == Forfeit) {
        removeBattle(winner, loser);
    }
}

void Server::removeBattle(int winner, int loser)
{
    BattleSituation *battle = player(winner)->battle;

    mybattles.remove(battle->publicId());
    foreach(int id, battle->getSpectators()) {
        player(id)->relay().finishSpectating(battle->publicId());
        player(id)->battlesSpectated.remove(battle->publicId());
    }
    /* When manipulating threaded objects, you need to be careful... */
    battle->deleteLater();
    player(winner)->battle = NULL;
    player(winner)->battleId() = -1;
    player(loser)->battle = NULL;
    player(loser)->battleId() = -1;
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
    myengine->beforeChangeTeam(id);

    /* In case that person was script kicked */
    if (!playerExist(id))
        return;

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

    myengine->afterChangeTeam(id);
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

void Server::spectatingRequested(int id, int idOfBattler)
{
    if (!playerLoggedIn(idOfBattler)) {
        return; //INVALID BEHAVIOR
    }
    if (!player(idOfBattler)->battling()) {
        return; //INVALID BEHAVIOR
    }
    BattleSituation *battle = player(idOfBattler)->battle;
    if (!battle->acceptSpectator(id, auth(id) > 0)) {
        sendMessage(id, "The battle refused you watching (maybe Disallow Spectator clause is enabled?)");
        return;
    }

    battle->addSpectator(player(id));
}

void Server::spectatingStopped(int id, int idOfBattle)
{
    BattleSituation *battle = mybattles[idOfBattle];

    battle->removeSpectator(id);
}

void Server::removePlayer(int id)
{
    if (playerExist(id))
    {
        qDebug() << "Starting removing player " << id;
	Player *p = player(id);
        bool loggedIn = p->isLoggedIn();

        if (loggedIn) {
            myengine->beforeLogOut(id);
        }

        p->doWhenDC();

        p->blockSignals(true);

	QString playerName = p->name();

        AntiDos::obj()->disconnect(p->ip(), id);

        p->deleteLater(); myplayers.remove(id);

        if (loggedIn)
            mynames.remove(playerName.toLower());

        int row = list()->row(myplayersitems[id]);
        delete list()->takeItem(row);
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

void Server::sendAll(const QString &message, bool chatMessage)
{
    if (printLine(message, chatMessage)) {
        foreach (Player *p, myplayers)
            if (p->isLoggedIn())
                p->sendMessage(message);
    }
}

int Server::freeid() const
{
    for (int i = 1; ; i++) {
        if (!myplayers.contains(i)) {
            return i;
        }
    }
}

int Server::freebattleid() const
{
    for (int i = 1; ; i++) {
        if (!mybattles.contains(i)) {
            return i;
        }
    }
}

void Server::atServerShutDown() {
    myengine->serverShutDown();
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
    if (!myplayers.contains(id))
        qDebug() << "Fatal! player called for non existing ID " << id;
    return myplayers.value(id);
}

Player * Server::player(int id) const
{
    return myplayers[id];
}

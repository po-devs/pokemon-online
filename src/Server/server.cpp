#include <QtNetwork>
#include <ctime> /* for random numbers, time(NULL) needed */
#include <algorithm>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/movesetchecker.h"
#include "../Utilities/otherwidgets.h"
#include "server.h"
#include "player.h"
#include "challenge.h"
#include "battle.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "security.h"
#include "antidos.h"
#include "serverconfig.h"
#include "scriptengine.h"
#include "tiermachine.h"
#include "tier.h"
#include "battlingoptions.h"
#include "sql.h"
#include "sqlconfig.h"
#include "pluginmanager.h"
#include "analyze.h"

Server *Server::serverIns = NULL;

Server::Server(quint16 port) : registry_connection(NULL), serverPorts(), showLogMessages(true),
    lastDataId(0), playercounter(0), battlecounter(0), channelcounter(0), numberOfPlayersLoggedIn(0),
    myengine(NULL)
{
    serverPorts << port;
}

Server::Server(QList<quint16> ports) : registry_connection(NULL), serverPorts(), showLogMessages(true),
    lastDataId(0), playercounter(0), battlecounter(0), channelcounter(0), numberOfPlayersLoggedIn(0),
    myengine(NULL)
{
    foreach(quint16 port, ports)
        serverPorts << port;
}


Server::~Server()
{
#ifndef SFML_SOCKETS
    foreach (QTcpServer* myserver, myservers)
        myserver->deleteLater();
#endif
    delete pluginManager;
}

/**
 * The following code is not placed in the constructor,
 * because view-components may want to show startup messages (printLine).
 *
 * This can be only acheived (in a clean way) by first letting a view listen
 * to the signal "servermessage". Therefore, the serverobject must be passed
 * to that view (by construction of the view), and then the server should start.
 */
void Server::start(){
    serverIns = this;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

#ifndef SFML_SOCKETS
    for (int i = 0; i < serverPorts.size(); ++i) {
        myservers.append(new QTcpServer());
    }
#else
    for (int i = 0; i < serverPorts.size(); ++i) {
        myservers.append(manager.createServerSocket());
    }
#endif
    pluginManager = new PluginManager(this);

    srand(time(NULL));

    QSettings s("config", QSettings::IniFormat);

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
    if (!s.contains("show_log_messages")) {
        s.setValue("show_log_messages", true);
    }
    if (!s.contains("safe_scripts")) {
        s.setValue("safe_scripts", true);
    }
    if (!s.contains("server_password")) {
        s.setValue("server_password", "");
    }
    if (!s.contains("require_password")) {
        s.setValue("require_password", false);
    }
    if (!s.contains("show_tray_popup")) {
        s.setValue("show_tray_popup", true);
    }
    if (!s.contains("minimize_to_tray")) {
        s.setValue("minimize_to_tray", true);
    }

    try {
        SQLCreator::createSQLConnection();
    } catch (const QString &ex) {
        printLine(ex);
    }

    printLine(tr("Starting loading pokemon database..."));

    /* Really useful for headless servers */
    PokemonInfo::init("db/pokes/", FillMode::Server);
    MoveSetChecker::init("db/pokes/");
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
    GenderInfo::init("db/genders/"); //needed by battlelogs plugin

    printLine(tr("Pokemon database loaded"));

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
    connect(TierMachine::obj(), SIGNAL(tiersChanged()), SLOT(tiersChanged()));

    AntiDos::obj()->init();

    printLine(tr("Members loaded"));

    battleThread.start();
    printLine(tr("Battle Thread started"));

    bool listenSuccess;

    QSignalMapper *mymapper = new QSignalMapper(this);
    connect(mymapper, SIGNAL(mapped(int)), SLOT(incomingConnection(int)));
    for (int i = 0; i < serverPorts.size(); ++i) {
        quint16 port = serverPorts.at(i);
#ifndef SFML_SOCKETS
        listenSuccess = server(i)->listen(QHostAddress::Any, port);
#else
        listenSuccess = server(i)->listen(port);
#endif

        if (!listenSuccess)
        {
            printLine(tr("Unable to listen to port %1").arg(port));
        } else {
            printLine(tr("Starting to listen to port %1").arg(port));
        }

        mymapper->setMapping(&*server(i), i);
#ifndef SFML_SOCKETS
        connect(server(i), SIGNAL(newConnection()), mymapper, SLOT(map()));
#else
        connect(&*server(i), SIGNAL(active()), mymapper, SLOT(map()));
#endif
    }
#ifdef SFML_SOCKETS
    manager.start();
#endif
    connect(AntiDos::obj(), SIGNAL(kick(int)), SLOT(dosKick(int)));
    connect(AntiDos::obj(), SIGNAL(ban(QString)), SLOT(dosBan(QString)));

    if (s.value("battles_with_same_ip_unrated").isNull()) {
        s.setValue("battles_with_same_ip_unrated", true);
    }
    if (s.value("rated_battles_memory_number").isNull()) {
        s.setValue("rated_battles_memory_number", 5);
    }

    loadRatedBattlesSettings();

    if (s.value("logs_channel_files").isNull()) {
        s.setValue("logs_channel_files", false);
    }
    if (s.value("logs_battle_files").isNull()) {
        s.setValue("logs_battle_files", false);
    }
    useChannelFileLog = s.value("logs_channel_files").toBool();

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
    lowTCPDelay = quint16(s.value("low_TCP_delay").toBool());
    safeScripts = s.value("safe_scripts").toBool();
    proxyServers = s.value("proxyservers").toString().split(",");
    passwordProtected = s.value("require_password").toBool();
    serverPassword = s.value("server_password").toString();
    showTrayPopup = s.value("show_tray_popup").toBool();
    minimizeToTray = s.value("minimize_to_tray").toBool();

    /* Adds the main channel */
    addChannel();

    /* Processes the daily run */
    if (s.value("process_ratings_on_startup", true).toBool())
        TierMachine::obj()->processDailyRun();
    QTimer *t2= new QTimer(this);
    connect(t2, SIGNAL(timeout()), this, SLOT(processDailyRun()));
    t2->start(24*3600*1000);

    myengine = new ScriptEngine(this);
    myengine->serverStartUp();

    this->showLogMessages = s.value("show_log_messages").toBool();

    if (serverPrivate != 1)
        connectToRegistry();
}

void Server::print(const QString &line)
{
    serverIns->printLine(line, false, true);
}

#ifndef SFML_SOCKETS
QTcpServer * Server::server(int i)
{
    return myservers.at(i);
}
#else
GenericSocket Server::server(int i)
{
    return myservers.at(i);
}
#endif
void Server::processDailyRun()
{
    sendAll("The server is updating all the ratings, as it does daily. It may take a bit of time.");

    /* Running delayed as otherwise the message would be sent after the lag, not before */
    QTimer::singleShot(1000, this, SLOT(updateRatings()));
}

void Server::updateRatings()
{
    TierMachine::obj()->processDailyRun();

    sendAll("All ratings updated!");

    /* Updating ratings of the players online */
    foreach(Player *p, myplayers) {
        if (p->isLoggedIn()) {
            p->findRating();
        }
    }
}

int Server::addChannel(const QString &name, int playerid) {
    if (channelids.contains(name.toLower())) {
        return -1; //Teehee
    }

    int chanid = 0;
    QString chanName = name;

    if (channelids.size() == 0) {
        /* Time to add the default channel */
        QSettings s("config", QSettings::IniFormat);
        chanName = s.value("mainchanname").toString();
        if (!Channel::validName(chanName)) {
            static const char* places [] = {
                "Radio Tower", "Pallet Town", "Icy cave", "Stark Mountain", "Mount Silver", "Route 202", "Old Power Plant", "Mewtwo's Cave",
                "Silph. Corp.", "Dragon's Lair", "Cinnabar Island"
            };

            chanName = places[true_rand() % sizeof(places)/sizeof(const char *)];
        }
    } else {
        if (!Channel::validName(name)) {
            return -1;
        }
        chanid = freechannelid();

        if (!myengine->beforeChannelCreated(chanid, chanName, playerid))
            return -1;
    }

    printLine(QString("Channel %1 was created").arg(chanName));

    channels[chanid] = new Channel(chanName);
    channelids[chanName.toLower()] = chanid;
    channelNames[chanid] = chanName;

    foreach(Player *p, myplayers) {
        if (p->isLoggedIn())
            p->relay().notify(NetworkServ::AddChannel, chanName, qint32(chanid));
    }

    if (chanid != 0)
        myengine->afterChannelCreated(chanid, chanName, playerid);

    return chanid;
}

void Server::joinChannel(int playerid, int channelid) {
    if (!channels.contains(channelid)) {
        return;
    }
    if (!myengine->beforeChannelJoin(playerid, channelid)) {
        return;
    }
    /* Because the script might have kicked the player */
    if (!playerExist(playerid)) {
        return;
    }

    Channel &channel = this->channel(channelid);
    QVector<qint32> ids;
    ids.reserve(channel.players.size());

    Player *player = this->player(playerid);
    PlayerInfo bundle = player->bundle();

    Analyzer &relay = player->relay();
    foreach(Player *p, channel.players) {
        if (!p->isInSameChannel(player)) {
            relay.sendPlayer(p->bundle());
            p->relay().sendPlayer(bundle);
        }
        ids.push_back(p->id());
    }

    relay.sendChannelPlayers(channelid, ids);
    channel.players.insert(player);
    player->addChannel(channelid);

    printLine(QString("%1 joined channel %2.").arg(player->name(), channel.name));

    foreach(Player *p, channel.players) {
        p->relay().sendJoin(playerid, channelid);
    }

    relay.sendBattleList(channelid, channel.battleList);

    foreach(int battleid, player->getBattles()) {
        if (!channel.battleList.contains(battleid)) {
            channel.battleList.insert(battleid, battleList[battleid]);
            foreach(Player *p, channel.players) {
                p->relay().sendChannelBattle(channelid, battleid, battleList[battleid]);
            }
        }
    }

    myengine->afterChannelJoin(playerid, channelid);
}

void Server::leaveRequest(int playerid, int channelid)
{
    Channel &channel = this->channel(channelid);

    Player *player = this->player(playerid);

    myengine->beforeChannelLeave(playerid, channelid);

    foreach(Player *p, channel.players) {
        p->relay().notify(NetworkServ::LeaveChannel, qint32(channelid), qint32(playerid));
    }

    foreach(int battleid, player->getBattles()) {
        Battle &b = battleList[battleid];
        /* We remove the battle only if only one of the player is in the channel */
        if (int(channel.players.contains(this->player(b.id1))) + int(channel.players.contains(this->player(b.id2))) < 2) {
            channel.battleList.remove(battleid);
        }
    }

    printLine(QString("%1 left channel %2.").arg(player->name(), channel.name));
    channel.players.remove(player);
    player->removeChannel(channelid);

    myengine->afterChannelLeave(playerid, channelid);

    if (channel.players.size() <= 0 && channelid != 0) {
        removeChannel(channelid);
    }
}

void Server::ipChangeRequested(int player, const QString& ip)
{
    if (SecurityManager::bannedIP(ip)) {
        this->player(player)->kick();
        return;
    }
    if (!AntiDos::obj()->changeIP(ip, this->player(player)->proxyIp())) {
        this->player(player)->kick();
        return;
    }
}

void Server::removeChannel(int channelid) {
    if (!myengine->beforeChannelDestroyed(channelid))
        return;

    QString chanName = channelNames.take(channelid);
    printLine(QString("Channel %1 was removed.").arg(chanName));
    channelids.remove(chanName.toLower());
    delete channels.take(channelid);

    foreach(Player *p, myplayers) {
        if (p->isLoggedIn())
            p->relay().notify(NetworkServ::RemoveChannel, qint32(channelid));
    }

    myengine->afterChannelDestroyed(channelid);
}

void Server::loadRatedBattlesSettings()
{
    QSettings s("config", QSettings::IniFormat);
    allowRatedWithSameIp = !s.value("battles_with_same_ip_unrated").toBool();
    diffIpsForRatedBattles = s.value("rated_battles_memory_number").toInt();
    allowThroughChallenge = s.value("rated_battle_through_challenge").toInt();

    TierMachine::obj()->loadDecaySettings();
}

void Server::connectToRegistry()
{
    if (registry_connection != NULL) {
        if (registry_connection->isConnected()) {
            return;
        }
        else
            registry_connection->deleteLater();
    }

    registry_connection = NULL;

    if (serverPrivate)
        return;

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
        printLine("The server is now private.", false, true);
        disconnectFromRegistry();
    }
    else
    {
        printLine("The server is now public.", false, true);
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
    registry_connection->notify(NetworkServ::Login, serverName, serverDesc, quint16(AntiDos::obj()->numberOfDiffIps()), serverPlayerMax, serverPorts.at(0));
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
    foreach (Player *p, myplayers) {
        if (p->isLoggedIn()) {
            p->relay().notify(NetworkServ::ServerName, name);
        }
    }

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServNameChange, name);
}

void Server::regDescChanged(const QString &desc)
{
    if (serverDesc == desc)
        return;

    serverDesc = desc;
    printLine("The description of the server changed.", false, true);

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServDescChange, desc);
}

void Server::regMaxChanged(const int &numMax)
{
    if (numMax == serverPlayerMax)
        return;

    serverPlayerMax = numMax;
    printLine("Maximum Players Changed.", false, true);

    if (registry_connection == NULL || !registry_connection->isConnected())
        return;

    registry_connection->notify(NetworkServ::ServMaxChange,numMax);
}

void Server::changeScript(const QString &script)
{
    myengine->changeScript(script);
}

void Server::setAllAnnouncement(const QString &html) {
    foreach(Player *p, myplayers) {
        p->relay().notify(NetworkServ::Announcement, html);
    }
}

void Server::announcementChanged(const QString &announcement)
{
    if (announcement == serverAnnouncement)
        return;

    serverAnnouncement = announcement;

    printLine("Announcement changed.", false, true);

    foreach(Player *p, myplayers) {
        p->relay().notify(NetworkServ::Announcement, serverAnnouncement);
    }
}

void Server::mainChanChanged(const QString &name) {
    if (name == channel(0).name) {
        return;
    }

    if (channelExist(name)) {
        printLine("Another channel with that name already exists, doing nothing.");
        return;
    }

    channelids.remove(channel(0).name.toLower());
    channel(0).name = name;
    channelids[name.toLower()] = 0;
    channelNames[0] = name;

    printLine("Main channel name changed", false, true);

    foreach(Player *p, myplayers) {
        p->relay().notify(NetworkServ::ChanNameChange, qint32(0), name);
    }
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
    printLine("Invalid name for the registry. Please change it in Options -> Config.", false, true);
}

void Server::nameTaken()
{
    printLine("The name of the server is already in use. Please change it in Options -> Config.", false, true);
}

void Server::ipRefused()
{
    printLine("Registry wants only 1 server per IP", false, true);
}

/* Returns false if the event "newMessage" was stopped (nothing to do with "chatMessage") */
bool Server::printLine(const QString &line, bool chatMessage, bool forcedLog)
{
    if (!chatMessage && !showLogMessages && !forcedLog)
        return false;

    if (myengine == NULL) {
        qDebug() << line;
        emit servermessage(line);
        return true;
    }
    if (chatMessage || myengine->beforeNewMessage(line)) {
        //notify possible views (if any)
        if(chatMessage){
            qDebug() << line;
            emit chatmessage(line);
        } else {
            qDebug() << line;
            emit servermessage(line);
        }
        if (!chatMessage)
            myengine->afterNewMessage(line);
        return true;
    }
    return false;
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

        emit player_authchange(id, authedName(id));
        if (SecurityManager::member(name).authority() != auth) {
            SecurityManager::setAuth(name, auth);
        }
        sendPlayer(id);
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
    silentKick(id);
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
    printLine(QString("Player %1 set name to %2").arg(id).arg(name));


    if (nameExist(name)) {
        int ids = this->id(name);
        if (!playerLoggedIn(ids) || player(ids)->name().toLower() != name.toLower()) {
            printLine(QString("Critical Bug needing to be solved (kept a name too much in the name list: %1)").arg(name));
            mynames.remove(name.toLower());
        } else {
            // If registered - kick old one (ghost), otherwise - kick new one to prevent wars.
            if (SecurityManager::member(name).isProtected()) {
                printLine(tr("%1: replaced by new connection.").arg(name));
                sendMessage(ids, QString("You logged in from another client with the same name. Logging off."));
                silentKick(ids);
            } else {
                printLine(tr("Name %1 already in use, disconnecting player %2").arg(name, QString::number(id)));
                sendMessage(id, QString("Another with the name %1 is already logged in").arg(name));
                silentKick(id);
                return;
            }
        }
    }

    /* For new connections */
    if (!player(id)->isLoggedIn()) {
        mynames.insert(name.toLower(), id);
        emit player_authchange(id, authedName(id));

        Player *p = player(id);

        p->changeState(Player::LoggedIn, true);

        if(!myengine->beforeLogIn(id) && playerExist(id)) {
            mynames.remove(name.toLower());
            p->changeState(Player::LoggedIn, false);
            silentKick(id);
            return;
        }
        if (!playerExist(id))
            return;

        p->relay().sendLogin(p->bundle());

        if (serverAnnouncement.length() > 0)
            p->relay().notify(NetworkServ::Announcement, serverAnnouncement);

        sendTierList(id);
        sendChannelList(id);
        numberOfPlayersLoggedIn += 1;

        /* Makes the player join the default channel */
        joinChannel(id, 0);
#ifndef PO_NO_WELCOME
        sendMessage(id, tr("<font color=blue><b>Welcome Message:</b></font> The updates are available at <a href=\"http://pokemon-online.eu/\">pokemon-online.eu</a>. Report any bugs on the forum."),true);
#endif

        myengine->afterLogIn(id);
    } else { /* if already logged in */
        recvTeam(id, name);
    }
}

void Server::sendChannelList(int player) {
    Player *p = this->player(player);

    p->relay().notify(NetworkServ::ChannelsList, channelNames);
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

    if (player(id)->hasBattle(publicId))
        player(id)->relay().sendBattleCommand(publicId, comm);
    else {
        if (player(id)->battlesSpectated.contains(publicId))
            player(id)->relay().sendWatchingCommand(publicId, comm);
    }
}

void Server::sendMessage(int id, const QString &message, bool html)
{
    player(id)->sendMessage(message, html);
}

void Server::sendServerMessage(const QString &message)
{
    sendAll("~~Server~~: " + message);
}

void Server::battleMessage(int player, int battle, const BattleChoice &choice)
{
    if (!mybattles.contains(battle)) {
        return;
    }

    mybattles[battle]->battleChoiceReceived(player, choice);
}

void Server::battleChat(int player, int battle, const QString &chat)
{
    if (!mybattles.contains(battle)) {
        return;
    }

    mybattles[battle]->battleChat(player, chat);
}

void Server::spectatingChat(int player, int battle, const QString &chat)
{
    if (!mybattles.contains(battle)) {
        return;
    }
    mybattles[battle]->spectatingChat(player, chat);
}

void Server::joinRequest(int player, const QString &channel)
{
    if (!channelExist(channel)) {
        if (channels.size() >= 1000) {
            sendMessage(player, "The server is limited to 1000 channels.");
            return;
        }
        if (addChannel(channel, player) == -1)
            return;
    }

    /* Because scripts might have caused the destruction of the previous channel,
       if the scripter puts some code in addChannel that would cause a masskick */
    if (!channelExist(channel))
        return;

    int channelid = channelids[channel.toLower()];

    if (this->channel(channelid).players.contains(this->player(player))) {
        //already in the channel
        return;
    }

    joinChannel(player, channelid);
}

void Server::recvMessage(int id, int channel, const QString &mess)
{
    QString re = mess.trimmed();
    if (re.length() > 0) {
        if (myengine->beforeChatMessage(id, mess, channel)) {
            sendChannelMessage(channel, QString("%1: %2").arg(name(id)).arg(re), true);
            myengine->afterChatMessage(id, mess, channel);
        }
    }
}

void Server::recvPM(int src, int dest, const QString &mess)
{
    if (playerLoggedIn(dest)) {
        Player *d = player(dest);

        d->acquireRoughKnowledgeOf(player(src));
        d->relay().sendPM(src, mess);
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

void Server::incomingConnection(int i)
{
    GenericSocket newconnection = server(i)->nextPendingConnection();

    if (!newconnection)
        return;

    int id = freeid();
#ifndef SFML_SOCKETS
    QString ip = newconnection->peerAddress().toString();
#else
    QString ip = newconnection->ip();
#endif
    if (SecurityManager::bannedIP(ip)) {
        newconnection->deleteLater();
        return;
    }

    if (!AntiDos::obj()->connecting(ip)) {
        /* Useless to waste lines on that especially if it is DoS'd */
        //printLine(tr("Anti DoS manager prevented IP %1 from logging in").arg(ip));
        newconnection->deleteLater();
        return;
    }

    if (numPlayers() >= serverPlayerMax && serverPlayerMax != 0) {
        printLine(QString("Stopped IP %1 from logging in, server full.").arg(ip));
        Player* p = new Player(newconnection,-1);
        connect(p, SIGNAL(disconnected(int)), p, SLOT(deleteLater()));
        p->sendMessage("The server is full.");
        AntiDos::obj()->disconnect(p->ip(), -1);
        p->kick();
        p->deleteLater();
        return;
    }

    printLine(QString("Received pending connection on slot %1 from %2").arg(id).arg(ip));

#ifndef SFML_SOCKETS
    newconnection->setSocketOption(QAbstractSocket::LowDelayOption, lowTCPDelay);
#else
    newconnection->setLowDelay(lowTCPDelay);
#endif
    myplayers[id] = new Player(newconnection, id);

    emit player_incomingconnection(id);

    Player *p = player(id);

    connect(p, SIGNAL(loggedIn(int, QString)), SLOT(loggedIn(int, QString)));
    connect(p, SIGNAL(recvTeam(int, QString)), SLOT(recvTeam(int, QString)));
    connect(p, SIGNAL(recvMessage(int, int, QString)), SLOT(recvMessage(int, int, QString)));
    connect(p, SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(p, SIGNAL(sendChallenge(int,int,ChallengeInfo)), SLOT(dealWithChallenge(int,int,ChallengeInfo)));
    connect(p, SIGNAL(battleFinished(int,int,int,int)), SLOT(battleResult(int,int,int,int)));
    connect(p, SIGNAL(info(int,QString)), SLOT(info(int,QString)));
    connect(p, SIGNAL(playerKick(int,int)), SLOT(playerKick(int, int)));
    connect(p, SIGNAL(playerBan(int,int)), SLOT(playerBan(int, int)));
    connect(p, SIGNAL(PMReceived(int,int,QString)), SLOT(recvPM(int,int,QString)));
    connect(p, SIGNAL(awayChange(int,bool)), this, SLOT(awayChanged(int, bool)));
    connect(p, SIGNAL(spectatingRequested(int,int)), SLOT(spectatingRequested(int,int)));
    connect(p, SIGNAL(spectatingStopped(int,int)), SLOT(spectatingStopped(int,int)));
    connect(p, SIGNAL(battleChat(int,int,QString)), SLOT(battleChat(int,int,QString)));
    connect(p, SIGNAL(battleMessage(int,int,BattleChoice)), SLOT(battleMessage(int,int,BattleChoice)));
    connect(p, SIGNAL(spectatingChat(int,int, QString)), SLOT(spectatingChat(int,int, QString)));
    connect(p, SIGNAL(updated(int)), SLOT(sendPlayer(int)));
    connect(p, SIGNAL(findBattle(int,FindBattleData)), SLOT(findBattle(int, FindBattleData)));
    connect(p, SIGNAL(battleSearchCancelled(int)), SLOT(cancelSearch(int)));
    connect(p, SIGNAL(joinRequested(int,QString)), SLOT(joinRequest(int,QString)));
    connect(p, SIGNAL(leaveRequested(int,int)), SLOT(leaveRequest(int,int)));
    connect(p, SIGNAL(ipChangeRequested(int,QString)), SLOT(ipChangeRequested(int,QString)));
}

void Server::awayChanged(int src, bool away)
{
    if (!playerLoggedIn(src))
        return;

    ++lastDataId;
    foreach(int chanid, player(src)->getChannels()) {
        foreach(Player *p, channel(chanid).players) {
            /* That test avoids to send twice the same data to the client */
            if (!p->hasSentCommand(lastDataId)) {
                p->relay().notifyAway(src, away);
            }
        }
    }
}

void Server::cancelSearch(int id)
{
    player(id)->battleSearch() = false;
    delete battleSearchs.take(id);
}

void Server::dealWithChallenge(int from, int to, const ChallengeInfo &c)
{
    if (!playerLoggedIn(to)) {
        sendMessage(from, tr("That player is not online"));
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
    printLine(QString("%1 initiated a battle search.").arg(name(id)));

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

        if (p1->gen() != p2->gen())
            continue;

        /* We check the tier thing */
        if ( (f.sameTier || data->sameTier) && p1->tier() != p2->tier() )
            continue;

        /* The double battle thing */
        if ( f.mode != data->mode) {
            continue;
        }

        /* We check both allow rated if needed */
        if (f.rated || data->rated) {
            if (!canHaveRatedBattle(id, key, f.mode, f.rated, data->rated))
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
        c.rated =  f.rated || data->rated || (canHaveRatedBattle(p1->id(), p2->id(), f.mode, f.rated, data->rated));
        c.clauses = TierMachine::obj()->tier(p1->tier()).getClauses();

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

bool Server::beforeChangeTier(int src, const QString &old, const QString &dest)
{
    return myengine->beforeChangeTier(src, old, dest);
}

void Server::afterChangeTier(int src, const QString &old, const QString &dest)
{
    myengine->afterChangeTier(src, old, dest);
}

bool Server::beforeFindBattle(int src) {
    return myengine->beforeFindBattle(src);
}

void Server::afterFindBattle(int src) {
    myengine->afterFindBattle(src);
}

bool Server::beforePlayerAway(int src, bool away)
{
    return myengine->beforePlayerAway(src, away);
}

void Server::afterPlayerAway(int src, bool away)
{
    myengine->afterPlayerAway(src, away);
}

void Server::logSavingChanged(bool logging)
{
    if (logging == showLogMessages)
        return;
    showLogMessages = logging;
    printLine("Logging changed", false, true);
}

void Server::useChannelFileLogChanged(bool logging)
{
    if (useChannelFileLog == logging)
        return;
    useChannelFileLog = logging;
    printLine("Channel File Logging changed", false, true);
}

void Server::TCPDelayChanged(bool lowTCP)
{
    if (lowTCPDelay == lowTCP)
        return;
    lowTCPDelay = true;
    printLine("Low TCP Delay setting changed", false, true);

    foreach(Player *p, myplayers) {
        p->relay().setLowDelay(lowTCP);
    }
}

void Server::safeScriptsChanged(bool safeScripts)
{
    if (this->safeScripts == safeScripts)
        return;
    this->safeScripts = safeScripts;
    printLine("Safe scripts setting changed", false, true);
}

void Server::proxyServersChanged(const QString &ips)
{
    QStringList newlist = ips.split(",");
    if (proxyServers == newlist)
        return;
    proxyServers = ips.split(",");
    printLine("Proxy Servers setting changed", false, true);
}

void Server::serverPasswordChanged(const QString &pass)
{
    if (serverPassword == pass)
        return;
    serverPassword = pass;
    printLine("Server Password changed", false, true);
}

void Server::usePasswordChanged(bool usePass)
{
    if (passwordProtected == usePass)
        return;
    passwordProtected = usePass; 
    printLine("Require Server Password changed", false, true);
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

    int maxauth = SecurityManager::maxAuth(player(dest)->ip());

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
    int id = freebattleid();

    myengine->beforeBattleStarted(id1,id2,c,id);

    if (!playerExist(id1) || !playerExist(id2))
        return;

    printLine(QString("Battle between %1 and %2 started").arg(name(id1)).arg(name(id2)));

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

    BattleSituation *battle = new BattleSituation(*player(id1), *player(id2), c, id, pluginManager);
    mybattles.insert(id, battle);
    battleList.insert(id, Battle(id1, id2));
    myengine->battleSetup(id1, id2, id); // dispatch script event

    Player *p1 (player(id1)), *p2 (player(id2));

    if (!p1->isInSameChannel(p2)) {
        p1->relay().sendPlayer(p2->bundle());
        p2->relay().sendPlayer(p1->bundle());
    }

    p1->startBattle(id, id2, battle->pubteam(id1), battle->configuration());
    p2->startBattle(id, id1, battle->pubteam(id2), battle->configuration());

    ++lastDataId;
    foreach(int chanid, p1->getChannels()) {
        Channel &chan = channel(chanid);
        if (!chan.battleList.contains(id)) {
            chan.battleList.insert(id,Battle(id1, id2));
        }
        foreach(Player *p, chan.players) {
            /* That test avoids to send twice the same data to the client */
            if (p->id() != id1 && p->id() != id2 && !p->hasSentCommand(lastDataId)) {
                p->relay().notifyBattle(id,id1,id2);
            }
        }
    }
    foreach(int chanid, p2->getChannels()) {
        Channel &chan = channel(chanid);
        if (!chan.battleList.contains(id)) {
            chan.battleList.insert(id,Battle(id1, id2));
        }
        foreach(Player *p, chan.players) {
            /* That test avoids to send twice the same data to the client */
            if (p->id() != id1 && p->id() != id2 && !p->hasSentCommand(lastDataId)) {
                p->relay().notifyBattle(id,id1,id2);
            }
        }
    }

    connect(battle, SIGNAL(battleInfo(int,int,QByteArray)), SLOT(sendBattleCommand(int,int,QByteArray)));
    connect(battle, SIGNAL(battleFinished(int,int,int,int)), SLOT(battleResult(int, int,int,int)));

    battle->start(battleThread);

    myengine->afterBattleStarted(id1,id2,c,id);
}

bool Server::canHaveRatedBattle(int id1, int id2, int mode, bool force1, bool force2)
{
    Player *p1 = player(id1);
    Player *p2 = player(id2);
    if (!force1 && !p1->ladder())
        return false;
    if (!force2 && !p2->ladder())
        return false;
    if (p1->tier() != p2->tier())
        return false;
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
    Tier *t = &TierMachine::obj()->tier(p1->tier());
    if (!t->allowMode(mode))
        return false;
    t = &TierMachine::obj()->tier(p2->tier());
    if (!t->allowMode(mode))
        return false;
//    if (std::abs(p1->rating()-p2->rating()) > 300)
//        return false;
    return true;
}

void Server::battleResult(int battleid, int desc, int winner, int loser)
{
    if (!mybattles.contains(battleid)) {
        /* If a player forfeits at the same time a battle ends, as the signal is asynchorneous
           because of different threads, it can be emitted twice. So that's why we may fall into
           this if */
        return;
    }

    bool rated = mybattles.value(battleid)->rated();
    QString tier = mybattles.value(battleid)->tier();
    BattleSituation *battle = mybattles[battleid];

    if (winner == 0) {
        winner = battle->id(battle->opponent(battle->spot(loser)));
    }

    Player *pw = player(winner);
    Player *pl = player(loser);

    if (desc == Forfeit && battle->finished()) {
        pw->battleResult(battleid, Close, winner, loser);
        pl->battleResult(battleid, Close, winner, loser);
    } else {
        if (desc != Tie && rated) {
            QString winn = pw->name();
            QString lose = pl->name();
            TierMachine::obj()->changeRating(winn, lose, tier);
            pw->rating() = TierMachine::obj()->rating(winn, tier);
            pl->rating() = TierMachine::obj()->rating(lose, tier);
            sendPlayer(winner);
            sendPlayer(loser);
        }
        myengine->beforeBattleEnded(winner, loser, desc, battleid);

        ++lastDataId;
        foreach(int chanid, pw->getChannels()) {
            Channel &chan = channel(chanid);
            chan.battleList.remove(battleid);
            foreach(Player *p, chan.players) {
                /* That test avoids to send twice the same data to the client */
                if (!p->hasSentCommand(lastDataId)) {
                    p->battleResult(battleid, desc, winner, loser);
                }
            }
        }
        foreach(int chanid, pl->getChannels()) {
            Channel &chan = channel(chanid);
            chan.battleList.remove(battleid);
            foreach(Player *p, chan.players) {
                /* That test avoids to send twice the same data to the client */
                if (!p->hasSentCommand(lastDataId)) {
                    p->battleResult(battleid, desc, winner, loser);
                }
            }
        }
        if (desc == Forfeit) {
            printLine(QString("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
        } else if (desc == Win) {
            printLine(QString("%1 won his battle against %2").arg(name(winner), name(loser)));
        } else if (desc == Tie) {
            printLine(QString("%1 and %2 tied").arg(name(winner), name(loser)));
        }
        myengine->afterBattleEnded(winner, loser, desc, battleid);
    }


    if (desc == Forfeit) {
        removeBattle(battleid);
    }
}

void Server::removeBattle(int battleid)
{
    BattleSituation *battle = mybattles.value(battleid);

    mybattles.remove(battleid);
    battleList.remove(battleid);

    typedef QPair<int, QString> pair;
    foreach(pair p, battle->getSpectators()) {
        player(p.first)->relay().finishSpectating(battleid);
        player(p.first)->battlesSpectated.remove(battleid);
    }
    /* When manipulating threaded objects, you need to be careful... */
    battle->deleteLater();

    Player* p1 = player(battle->id(0));
    Player* p2 = player(battle->id(1));

    p1->removeBattle(battleid);
    p2->removeBattle(battleid);
}

void Server::sendBattlesList(int playerid, int chanid)
{
    Analyzer &relay = player(playerid)->relay();

    relay.sendBattleList(chanid, channel(chanid).battleList);
}

void Server::sendPlayer(int id)
{
    Player *source = player(id);
    PlayerInfo bundle = source->bundle();

    ++lastDataId;
    foreach(int chanid, source->getChannels()) {
        foreach(Player *p, channel(chanid).players) {
            /* That test avoids to send twice the same data to the client */
            if (!p->hasSentCommand(lastDataId)) {
                p->relay().sendPlayer(bundle);
            }
        }
    }
}

void Server::sendLogout(int id)
{
    numberOfPlayersLoggedIn -= 1;
    Player *source = player(id);

    ++lastDataId;
    foreach(int chanid, source->getChannels()) {
        foreach(Player *p, channel(chanid).players) {
            /* That test avoids to send twice the same data to the client */
            if (!p->hasSentCommand(lastDataId)) {
                p->relay().sendLogout(id);
            }
        }
    }
}

void Server::recvTeam(int id, const QString &_name)
{
    myengine->beforeChangeTeam(id);

    /* In case that person was script kicked */
    if (!playerExist(id))
        return;

    printLine(QString("%1 changed their team, and their name to %2").arg(name(id), _name));

    Player *source = player(id);

    /* Normally all checks have been made to ensure the authentication is right and the
       name isn't taken.

       That's done by calling loggedIn() before to test some things if needed.

       So here all i have to do is delete the old name */
    QString oldname = source->name();

    if (oldname == _name) {
        /* Haha, same name so no need to do anything! */

    } else {
        /* Changing the name! */
        mynames.remove(oldname.toLower());
        mynames.insert(_name.toLower(), id);
        source->setName(_name);
    }

    PlayerInfo bundle = source->bundle();

    /* Sending the team change! */
    ++lastDataId;
    foreach(int chanid, source->getChannels()) {
        foreach(Player *p, channel(chanid).players) {
            /* That test avoids to send twice the same data to the client */
            if (!p->hasSentCommand(lastDataId)) {
                p->relay().sendTeamChange(bundle);
            }
        }
    }

    emit player_authchange(id, authedName(id));

    myengine->afterChangeTeam(id);
}

void Server::disconnected(int id)
{
    printLine(QString("Received disconnection from player %1").arg(name(id)));
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

void Server::spectatingRequested(int id, int idOfBattle)
{
    if (!mybattles.contains(idOfBattle)) {
        return; // Invalid behavior
    }
    BattleSituation *battle = mybattles.value(idOfBattle);
    bool forced_allow = myengine->attemptToSpectateBattle(id, battle->id(0), battle->id(1));
    if (!battle->acceptSpectator(id, (auth(id) > 0) || forced_allow)) {
        sendMessage(id, "The battle refused you watching (maybe Disallow Spectator clause is enabled?)");
        return;
    }
    if (!myengine->beforeSpectateBattle(id, battle->id(0),battle->id(1))) {
        sendMessage(id, "The battle refused you watching (maybe Disallow Spectator clause is enabled?)");
        myengine->afterSpectateBattle(id, battle->id(0),battle->id(1));
        return;
    }

    Player *source = player(id);
    Player *p1(player(battle->id(0))), *p2(player(battle->id(1)));

    PlayerInfo bundle = source->bundle();

    if (!p1->isInSameChannel(source)) {
        p1->relay().sendPlayer(bundle);
        source->relay().sendPlayer(p1->bundle());
    }
    if (!p2->isInSameChannel(source)) {
        p2->relay().sendPlayer(bundle);
        source->relay().sendPlayer(p2->bundle());
    }
    typedef QPair<int, QString> pair;
    foreach(pair mp, battle->getSpectators()) {
        Player *p = player(mp.first);
        if (!p->isInSameChannel(source)) {
            p->relay().sendPlayer(bundle);
            source->relay().sendPlayer(p->bundle());
        }
    }
    foreach(QPointer<Player> p, battle->getPendingSpectators()) {
        if (!p)
            continue;

        if (!p->isInSameChannel(source)) {
            p->relay().sendPlayer(bundle);
            source->relay().sendPlayer(p->bundle());
        }
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
        Player *p = player(id);
        bool loggedIn = p->isLoggedIn();

        if (loggedIn) {
            myengine->beforeLogOut(id);
        }

        p->doWhenDC();

        p->blockSignals(true);

        QString playerName = p->name();

        AntiDos::obj()->disconnect(p->ip(), id);

        foreach(int chanid, p->getChannels()) {
            leaveRequest(id, chanid);
        }

        emit player_logout(id);

        /* Sending the notice of logout to others only if the player is already logged in */
        if (loggedIn) {
            sendLogout(id);
            myengine->afterLogOut(id);
        }

        p->deleteLater(); myplayers.remove(id);

        if (loggedIn)
            mynames.remove(playerName.toLower());

        printLine(QString("Removed player %1").arg(playerName));
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

void Server::sendAll(const QString &message, bool chatMessage, bool html)
{
    if (printLine(message, chatMessage, true)) {
        foreach (Player *p, myplayers)
            if (p->isLoggedIn())
                p->sendMessage(message, html);
    }
}

void Server::sendChannelMessage(int channel, const QString &message, bool chat, bool html)
{
    if(useChannelFileLog) {
        this->channel(channel).log(message);
    }
    printLine(QString("[#%1] %2").arg(this->channel(channel).name, message), chat, true);
    foreach (Player *p, this->channel(channel).players)
        p->sendChanMessage(channel, message, html);
}

void Server::sendChannelMessage(int id, int chanid, const QString &message, bool html)
{
    player(id)->sendChanMessage(chanid, message, html);
}

int Server::freeid() const
{
    do {
        ++playercounter;
    } while (myplayers.contains(playercounter) || playercounter == 0 || playercounter == -1); /* 0, -1 are reserved */

    return playercounter;
}

int Server::freebattleid() const
{
    do {
        ++battlecounter;
    } while (mybattles.contains(battlecounter) || battlecounter == 0); /* 0 is reserved */

    return battlecounter;
}

int Server::freechannelid() const
{
    do {
        ++channelcounter;
    } while (channels.contains(channelcounter) || channelcounter == 0 || channelcounter == -1); /* 0, -1 are reserved */

    return channelcounter;
}

void Server::atServerShutDown() {
    delete pluginManager, pluginManager = NULL;

    myengine->serverShutDown();
}

void Server::setAnnouncement(int &id, const QString &html) {
    if (player(id)->isLoggedIn())
            player(id)->relay().notify(NetworkServ::Announcement, html);;
}

Player * Server::player(int id) const
{
    if (!myplayers.contains(id))
        qDebug() << "Fatal! player called for non existing ID " << id;
    return myplayers.value(id);
}

PlayerInterface * Server::playeri(int id) const
{
    return player(id);
}

BattleSituation * Server::getBattle(int battleId) const
{
    if(mybattles.contains(battleId)) {
        return mybattles.value(battleId);
    }else{
        return NULL;
    }
}

bool Server::correctPass(const QByteArray &hash, const QByteArray &salt) const {
    return hash == md5_hash(md5_hash(serverPassword.toAscii()) + salt);
}

bool Server::isLegalProxyServer(const QString &ip) const
{
    foreach (QString proxyip, proxyServers) {
        if (ip == proxyip)
            return true;
    }
    return false;
}

void Server::showTrayPopupChanged(bool show)
{
    showTrayPopup = show;
}

void Server::minimizeToTrayChanged(bool allow)
{
    minimizeToTray = allow;
}

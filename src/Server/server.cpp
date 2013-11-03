#include <QtNetwork>
#include <ctime> /* for random numbers, time(NULL) needed */
#include <algorithm>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/movesetchecker.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/backtrace.h"
#include "server.h"
#include "player.h"
#include "challenge.h"
#include "battle.h"
#include "moves.h"
#include "rbymoves.h"
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
#include "networkutilities.h"
#include "battlerby.h"
#include "relaymanager.h"

Server *Server::serverIns = NULL;

/* Todo: use those as lambda functions when mingw 4.5 is out in windows */
static void updateChannelCache(QByteArray &val) {
    val = makePacket(NetworkServ::ChannelsList, Server::serverIns->channelNames);
}

static void updateZippedChannelCache(QByteArray&val) {
    val = makeZipPacket(NetworkServ::ChannelsList, Server::serverIns->channelNames);
}

//channelCache([&](QByteArray &val) {val = makePacket(NetworkServ::ChannelsList, channelNames);}),
//zchannelCache([&](QByteArray &val) {val = makeZipPacket(NetworkServ::ChannelsList, channelNames);}),

Server::Server(quint16 port) : registry_connection(NULL), serverPorts(), showLogMessages(true),
    lastDataId(0), playercounter(0), battlecounter(0), channelcounter(0),
    channelCache(&updateChannelCache), zchannelCache(updateZippedChannelCache), numberOfPlayersLoggedIn(0), myengine(NULL)
{
    serverPorts << port;
}

Server::Server(QList<quint16> ports) : registry_connection(NULL), serverPorts(), showLogMessages(true),
    lastDataId(0), playercounter(0), battlecounter(0), channelcounter(0), channelCache(&updateChannelCache),
    zchannelCache(updateZippedChannelCache), numberOfPlayersLoggedIn(0), myengine(NULL)
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

extern bool skipChecksOnStartUp;

/**
 * The following code is not placed in the constructor,
 * because view-components may want to show startup messages (printLine).
 *
 * This can be only achieved (in a clean way) by first letting a view listen
 * to the signal "servermessage". Therefore, the serverobject must be passed
 * to that view (by construction of the view), and then the server should start.
 */
void Server::start(){
    serverIns = this;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

#ifndef SFML_SOCKETS
    for (int i = 0; i < serverPorts.size(); ++i) {
        myservers.append(new QTcpServer());
    }
#else
    for (int i = 0; i < serverPorts.size(); ++i) {
        myservers.append(manager.createServerSocket());
    }
#endif
    srand(time(NULL));

    pluginManager = new PluginManager(this);

    QSettings s("config", QSettings::IniFormat);

    auto setDefaultValue = [&s](const char* key, const QVariant &defaultValue) {
            if (!s.contains(key)) {
                s.setValue(key, defaultValue);
            }
    };

    setDefaultValue("SQL/Driver", SQLCreator::SQLite);
    setDefaultValue("SQL/Database", "pokemon");
    setDefaultValue("SQL/Port", 5432);
    setDefaultValue("SQL/User", "postgres");
    setDefaultValue("SQL/Pass", "admin");
    setDefaultValue("SQL/Host", "localhost");
    setDefaultValue("SQL/DatabaseSchema", "");
    setDefaultValue("SQL/VacuumOnStartup", true);
    setDefaultValue("Scripts/SafeMode", false);
    setDefaultValue("Server/Password", "pikachu");
    setDefaultValue("Server/RequirePassword", false);
    setDefaultValue("Server/Private", false);
    setDefaultValue("Server/Name", QString());
    setDefaultValue("Server/Announcement", QString());
    setDefaultValue("Server/Description", QString());
    setDefaultValue("Server/MaxPlayers", 0);
    setDefaultValue("Channels/LoggingEnabled", false);
    setDefaultValue("Channels/MainChannel", QString());
    setDefaultValue("Ladder/MonthsExpiration", 3);
    setDefaultValue("Ladder/PeriodDuration", 24);
    setDefaultValue("Ladder/DecayPerPeriod", 5);
    setDefaultValue("Ladder/BonusPeriods", 5);
    setDefaultValue("Ladder/MaxDecay", 50);
    setDefaultValue("Ladder/ProcessRatingsOnStartUp", true);
    setDefaultValue("Battles/ForceUnratedForSameIP", true);
    setDefaultValue("Battles/ConsecutiveFindBattlesWithDifferentIPs", 5);
    setDefaultValue("Battles/RatedThroughChallenge", false);
    setDefaultValue("Network/ProxyServers", QString());
    setDefaultValue("Network/LowTCPDelay", false);
    setDefaultValue("AntiDOS/ShowOveractiveMessages", true);
    setDefaultValue("AntiDOS/TrustedIps", "127.0.0.1");
    setDefaultValue("AntiDOS/MaxPeoplePerIp", 2);
    setDefaultValue("AntiDOS/MaxCommandsPerUser", 50);
    setDefaultValue("AntiDOS/MaxKBPerUser", 25);
    setDefaultValue("AntiDOS/MaxConnectionRatePerIP", 6);
    setDefaultValue("AntiDOS/NumberOfInfractionsBeforeBan", 10);
    setDefaultValue("AntiDOS/Disabled", false);
    setDefaultValue("Players/InactiveThresholdInDays", 182);
    setDefaultValue("Players/ClearInactivesOnStartup", true);
    setDefaultValue("GUI/ShowLogMessages", false);
    setDefaultValue("Mods/CurrentMod", "");

    try {
        SQLCreator::createSQLConnection();
    } catch (const QString &ex) {
        printLine(ex);
    }

    printLine(tr("Starting loading pokemon database..."));

    PokemonInfoConfig::setFillMode(FillMode::Server);
    PokemonInfoConfig::setDataRepo(dataRepo);
    changeDbMod(s.value("Mods/CurrentMod").toString());

    printLine(tr("Pokemon database loaded"));

    for (int i = 0; i < GenInfo::GenMax(); i++) {
        PokemonInfo::RunMovesSanityCheck(i);
    }

    MoveEffect::init();
    RBYMoveEffect::init();
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
    RelayManager::init();

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

    loadRatedBattlesSettings();

    useChannelFileLog = s.value("Channels/LoggingEnabled").toBool();

    /*
      The timer for clearing the last rated battles memory, set to 3 hours
     */
    QTimer *t= new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(clearRatedBattlesHistory()));
    t->start(3*3600*1000);

    serverName = s.value("Server/Name").toString();
    serverDesc = s.value("Server/Description").toString();
    serverAnnouncement = s.value("Server/Announcement").toByteArray();
    zippedAnnouncement = makeZipPacket(NetworkServ::Announcement, serverAnnouncement);
    serverPlayerMax = quint16(s.value("Server/MaxPlayers").toInt());
    serverPrivate = quint16(s.value("Server/Private").toInt());
    amountOfInactiveDays = s.value("Players/InactiveThresholdInDays").toInt();
    lowTCPDelay = quint16(s.value("Network/LowTCPDelay").toBool());
    safeScripts = s.value("Scripts/SafeMode").toBool();
    overactiveShow = s.value("AntiDOS/ShowOveractiveMessages").toBool();
    proxyServers = s.value("Network/ProxyServers").toString().split(",");
    passwordProtected = s.value("Server/RequirePassword").toBool();
    serverPassword = s.value("Server/Password").toByteArray();
    zippedTiers = makeZipPacket(NetworkServ::TierSelection, TierMachine::obj()->tierList());

    /* Adds the main channel */
    addChannel();

    /* Processes the daily run */
    if (s.value("Ladder/ProcessRatingsOnStartUp").toBool() && !skipChecksOnStartUp) {
        TierMachine::obj()->processDailyRun();
    }

    if (s.value("Players/ClearInactivesOnStartup").toBool() && !skipChecksOnStartUp) {
        SecurityManager::processDailyRun(amountOfInactiveDays, false);
    }

    QTimer *t2 = new QTimer(this);
    connect(t2, SIGNAL(timeout()), this, SLOT(processDailyRun()));
    t2->start(24*3600*1000);

    myengine = new ScriptEngine(this);
    myengine->serverStartUp();

    this->showLogMessages = s.value("GUI/ShowLogMessages").toBool();

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
    broadCast("The server is updating the database and clearing inactive members, as it does daily. It may take a bit of time depending on database size.");
    broadCast("The server is updating all the ratings, as it does daily. It may take a bit of time.");

    /* Running delayed as otherwise the message would be sent after the lag, not before */
    QTimer::singleShot(1000, this, SLOT(updateDatabase()));
    QTimer::singleShot(1000, this, SLOT(updateRatings()));
}

void Server::changeDbMod(const QString &mod)
{
    battleThread.pause();

    PokemonInfoConfig::changeMod(mod);

    /* Really useful for headless servers */
    GenInfo::init("db/gens/");
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
    GenderInfo::init("db/genders/"); //needed by battlelogs plugin

    PokemonInfo::loadStadiumTradebacks();

    battleThread.unpause();
}

void Server::updateDatabase()
{
    SecurityManager::processDailyRun(amountOfInactiveDays);

    broadCast("Database cleaning launched!");
}

void Server::updateRatings()
{
    TierMachine::obj()->processDailyRun();

    broadCast("All ratings updated!");

    /* Updating ratings of the players online */
    foreach(Player *p, myplayers) {
        if (p->isLoggedIn()) {
            p->findRatings(true);
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
        chanName = s.value("Channels/MainChannel").toString();
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

    channels[chanid] = new Channel(chanName, chanid);
    channelids[chanName.toLower()] = chanid;
    channelNames[chanid] = chanName;
    channelCache.outdate();
    zchannelCache.outdate();

    notifyGroup(All, NetworkServ::AddChannel, chanName, qint32(chanid));

    if (chanid != 0)
        myengine->afterChannelCreated(chanid, chanName, playerid);

    return chanid;
}

bool Server::joinChannel(int playerid, int channelid) {
    if (!channels.contains(channelid)) {
        return false;
    }
    if (!myengine->beforeChannelJoin(playerid, channelid)) {
        return false;
    }
    /* Because the script might have kicked the player */
    if (!playerExist(playerid)) {
        return false;
    }

    Channel &channel = this->channel(channelid);
    QVector<qint32> ids;
    ids.reserve(channel.players.size());

    Player *player = this->player(playerid);

    QSet<Player*> unknown;
    QVector<reference<PlayerInfo> > bundles;

    Analyzer &relay = player->relay();
    foreach(Player *p, channel.players) {
        if (!p->isInSameChannel(player)) {
            unknown.insert(p);
            bundles.push_back(&p->bundle());
        }
        ids.push_back(p->id());
    }

    notifyGroup(unknown, NetworkServ::PlayersList, player->bundle());
    player->sendPlayers(bundles);

    relay.sendChannelPlayers(channelid, ids);
    channel.disconnectedPlayers.remove(player);
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

    return true;
}

void Server::needChannelData(int playerid, int channelid)
{
    Channel &channel = this->channel(channelid);
    QVector<qint32> ids;
    ids.reserve(channel.players.size());

    Player *player = this->player(playerid);

    QVector<reference<PlayerInfo> > bundles;

    Analyzer &relay = player->relay();
    foreach(Player *p, channel.players) {
        if (!p->isInSameChannel(player)) {
            bundles.push_back(&p->bundle());
        }
        ids.push_back(p->id());
    }

    player->sendPlayers(bundles);

    relay.sendChannelPlayers(channelid, ids);
    player->addChannel(channelid);

    relay.sendBattleList(channelid, channel.battleList);
}

void Server::leaveRequest(int playerid, int channelid, bool keep)
{
    Channel &channel = this->channel(channelid);

    Player *player = this->player(playerid);

    if (channel.players.contains(player)) {
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

        if (!keep) {
            player->removeChannel(channelid);
        } else {
            channel.disconnectedPlayers.insert(player);
        }

        myengine->afterChannelLeave(playerid, channelid);
    } else if (channel.disconnectedPlayers.contains(player)) {
        channel.disconnectedPlayers.remove(player);
        player->removeChannel(channelid);
    }

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

    channelCache.outdate();zchannelCache.outdate();

    notifyGroup(All, NetworkServ::RemoveChannel, qint32(channelid));

    myengine->afterChannelDestroyed(channelid);
}

void Server::loadRatedBattlesSettings()
{
    QSettings s("config", QSettings::IniFormat);
    allowRatedWithSameIp = !s.value("Battles/ForceUnratedForSameIP").toBool();
    diffIpsForRatedBattles = s.value("Battles/ConsecutiveFindBattlesWithDifferentIPs").toInt();
    allowThroughChallenge = s.value("Battles/RatedThroughChallenge").toInt();

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
    s->connectToHost("registry.pokemon-online.eu", 8081);

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
    broadCast("The name of the server changed to " + name + ".");

    notifyAll(NetworkServ::ServerInfoChanged, Flags(1), name);

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

void Server::regPasswordChanged(bool &isEnabled) {
    if (registry_connection == NULL || !registry_connection->isConnected())
        return;
    registry_connection->notify(NetworkServ::ServerPass, isEnabled);
}

void Server::changeScript(const QString &script)
{
    myengine->changeScript(script);
}

void Server::reloadTiers()
{
    TierMachine::obj()->load();
}

QString Server::description()
{
    return this->serverDesc;
}

void Server::setAllAnnouncement(const QString &html)
{
    notifyGroup(SupportsZip, makeZipPacket(NetworkServ::Announcement, html));
    notifyOppGroup(SupportsZip, NetworkServ::Announcement, html);
}

const QSet<Player*>& Server::getGroup(PlayerGroupFlags group) const
{
    return groups[group];
}

const QSet<Player*>& Server::getOppGroup(PlayerGroupFlags group) const
{
    return oppGroups[group];
}

void Server::announcementChanged(const QString &announcement)
{
    if (announcement.toUtf8() == serverAnnouncement)
        return;

    serverAnnouncement = announcement.toUtf8();
    zippedAnnouncement = makeZipPacket(NetworkServ::Announcement, serverAnnouncement);

    printLine("Announcement changed.", false, true);

    notifyGroup(SupportsZip, zippedAnnouncement);
    notifyOppGroup(SupportsZip, NetworkServ::Announcement, serverAnnouncement);
}

void Server::mainChanChanged(const QString &name) {
    if (name == channel(0).name || name.length() == 0) {
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
    channelCache.outdate();zchannelCache.outdate();

    printLine("Main channel name changed", false, true);

    notifyGroup(All, NetworkServ::ChanNameChange, qint32(0), name);
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
        emit serverMessage(line);
        return true;
    }
    if (chatMessage || myengine->beforeNewMessage(line)) {
        //notify possible views (if any)
        if(chatMessage){
            qDebug() << line;
            emit this->chatMessage(line);
        } else {
            qDebug() << line;
            emit serverMessage(line);
        }
        if (!chatMessage)
            myengine->afterNewMessage(line);
        return true;
    }
    return false;
}


void Server::tiersChanged()
{
    broadCast("Tiers have been updated!");

    zippedTiers = makeZipPacket(NetworkServ::TierSelection, TierMachine::obj()->tierList());
    notifyGroup(SupportsZip, zippedTiers);
    notifyOppGroup(SupportsZip, NetworkServ::TierSelection, TierMachine::obj()->tierList());

    foreach(Player *p, myplayers) {
        p->findTierAndRating();
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

    notifyGroup(All, NetworkServ::PlayerKick, qint32(id), qint32(src));

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
    notifyGroup(All, NetworkServ::PlayerBan, qint32(id), qint32(src));

    if (src == 0)
        printLine("The server banned " + name(id) + "!");
    else
        printLine(name(id) + " was banned by " + name(src));

    SecurityManager::ban(name(id));
    player(id)->kick();
}

void Server::tempBan(int dest, int src, int time)
{
    time = int(std::max(1, std::min(time, 1440)));
    if(src == 0) {
        if(time == 1) {
            printLine(QString("The server banned %1 for %2 minute").arg(name(dest)).arg(time));
        } else {
            printLine(QString("The server banned %1 for %2 minutes").arg(name(dest)).arg(time));
        }
    } else {
        if(time == 1) {
            printLine(QString("%1 was banned by %2 for %3 minute").arg(name(dest)).arg(name(src)).arg(time));
        } else {
            printLine(QString("%1 was banned by %2 for %3 minutes").arg(name(dest)).arg(name(src)).arg(time));
        }
    }
    notifyGroup(All, NetworkServ::PlayerTBan, qint32(dest), qint32(src), qint32(time));
    SecurityManager::ban(name(dest), time);
    player(dest)->kick();
}


void Server::dosKick(int id) {
    if (playerExist(id) && overactiveShow) {
        if (playerLoggedIn(id)) {
            broadCast(tr("Player %1 (IP %2) is being overactive.").arg(name(id), player(id)->ip()), dosChannel());
        } else {
            broadCast(tr("IP %1 is being overactive.").arg(player(id)->ip()), dosChannel());
        }
    }
    silentKick(id);
}

void Server::dosBan(const QString &ip) {
    if (overactiveShow) {
        broadCast(tr("IP %1 is being overactive, banned.").arg(ip), dosChannel());
    }
    SecurityManager::ban(ip);
}

int Server::dosChannel() const
{
    if (AntiDos::obj()->notificationsChannel.isEmpty()) {
        return NoChannel;
    }
    return channelId(AntiDos::obj()->notificationsChannel);
}

QObject* Server::getAntiDos() const
{
    return AntiDos::obj();
}

int Server::channelId(const QString &chanName) const
{
    return channelids.value(chanName.toLower(), NoChannel);
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

void Server::sendMessage(int id, const QString &message)
{
    broadCast(message, NoChannel, NoSender, false, id);
}

void Server::loggedIn(int id, const QString &name)
{
    printLine(QString("Player %1 set name to %2").arg(id).arg(name));

    if (!playerExist(id)) {
        printLine(QString("Critical Bug needing to be solved: Server::loggedIn, playerExist(%1) = false").arg(id));
        return;
    }

    if (nameExist(name)) {
        int ids = this->id(name);
        if ((!playerExist(ids) || (!playerLoggedIn(ids) && !player(ids)->waitingForReconnect())) || player(ids)->name().toLower() != name.toLower()) {
            printLine(QString("Critical Bug needing to be solved (kept a name too much in the name list: %1)").arg(name));
            mynames.remove(name.toLower());
        } else {
            if (SecurityManager::member(name).isProtected()) {
                /* Replaces the other one */
                if (!player(ids)->waitingForReconnect()) {
                    printLine(tr("%1: replaced by new connection.").arg(name));
                    sendMessage(ids, QString("You logged in from another client with the same name. Logging off."));
                }
                //When the client didn't intend to reconnect, we transfer only if the player was battling and there's a battle to save.
                if (!player(ids)->battling()) {
                    player(ids)->kick();
                } else {
                    transferId(id, ids, true);
                    return;
                }
            } else {
                // If the other player is disconnected, we remove him
                if (player(ids)->waitingForReconnect()) {
                    player(ids)->autoKick();
                } else {
                    printLine(tr("Name %1 already in use, disconnecting player %2").arg(name, QString::number(id)));
                    sendMessage(id, QString("Another with the name %1 is already logged in").arg(name));
                    silentKick(id);
                    return;
                }
            }
            /* Should not happen but we can't be too safe */
            if (id == ids) {
                printLine(QString("Critical Bug needing to be solved: Server::loggedIn, id=ids=%1").arg(id));
                return;
            }
        }
    }

    /* For new connections */
    if (!player(id)->isLoggedIn()) {
        mynames.insert(name.toLower(), id);
        emit player_authchange(id, authedName(id));

        Player *p = player(id);

        processLoginDetails(p);
    } else { /* if already logged in */
        recvTeam(id, name);
    }
}

void Server::processLoginDetails(Player *p)
{
    bool wasLoggedIn = p->isLoggedIn();

    int id = p->id();

    QString channel;
    if (p->loginInfo() && p->loginInfo()->channel) {
        channel = *p->loginInfo()->channel;
    }

    if (!wasLoggedIn) {
        groups[All].insert(p);
        if (p->supportsZip()) {
            groups[SupportsZip].insert(p);
        } else {
            oppGroups[SupportsZip].insert(p);
        }
        if (p->spec()[Player::IdsWithMessage]) {
            groups[IdsWithMessage].insert(p);
        } else {
            oppGroups[IdsWithMessage].insert(p);
        }
        if(!myengine->beforeLogIn(id, channel) && playerExist(id)) {
            mynames.remove(p->name().toLower());
            silentKick(id);
            return;
        }

        if (!playerExist(id))
            return;
    }

    p->changeState(Player::LoggedIn, true);

    p->sendLoginInfo();

    if (serverAnnouncement.length() > 0) {
        if (p->supportsZip()) {
            p->sendPacket(zippedAnnouncement);
        } else {
            p->relay().notify(NetworkServ::Announcement, serverAnnouncement);
        }
    }

    sendTierList(id);
    sendChannelList(id);

    if (!wasLoggedIn) {
        numberOfPlayersLoggedIn += 1;
        //printLine(tr("Adding a player, count: %1").arg(numberOfPlayersLoggedIn));
    }

    if (!p->state()[Player::WaitingReconnect] && !wasLoggedIn) {
        /* Makes the player join the default channel */
        if (! (p->loginInfo() && p->loginInfo()->channel && joinRequest(p->id(), *p->loginInfo()->channel)))  {
            joinChannel(id, 0);
        }
        if (p->loginInfo()) {
            if(p->loginInfo()->additionalChannels) {
                foreach(const QString &channel, *p->loginInfo()->additionalChannels) {
                    joinRequest(p->id(), channel);
                }
            }
        }
#ifndef PO_NO_WELCOME
        broadCast(tr("Welcome Message: The updates are available at http://pokemon-online.eu/ -- report any bugs on the forum."),
              NoChannel, NoSender, false, id);
#endif
    } else {
        p->doWhenRC(wasLoggedIn);
    }

    if (!wasLoggedIn) {
        myengine->afterLogIn(id, channel);
    }

    if (p->loginInfo()) {
        delete p->loginInfo(), p->loginInfo()=NULL;
    }
}

void Server::sendChannelList(int player) {
    Player *p = this->player(player);

    p->sendPacket(p->supportsZip() ? zchannelCache.value() : channelCache.value());
}

void Server::sendTierList(int id)
{
    if (player(id)->supportsZip()) {
        player(id)->sendPacket(zippedTiers);
    } else {
        player(id)->relay().notify(NetworkServ::TierSelection, TierMachine::obj()->tierList());
    }
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

void Server::sendServerMessage(const QString &message)
{
    if (myengine->beforeServerMessage(message))
    {
        broadCast(message, NoChannel, 0);
        myengine->afterServerMessage(message);
    }
}

void Server::battleMessage(int player, int battle, const BattleChoice &choice)
{
    if (!mybattles.contains(battle)) {
        return;
    }

    mybattles[battle]->battleChoiceReceived(player, choice);
}

void Server::resendBattleInfos(int player, int battle)
{
    if (!mybattles.contains(battle)) {
        return;
    }

    mybattles[battle]->addSpectator(this->player(player));
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

bool Server::joinRequest(int player, const QString &channel)
{
    printLine(tr("Player %1 requesting to join channel %2").arg(player).arg(channel));
    if (!channelExist(channel)) {
        if (channels.size() >= 1000) {
            sendMessage(player, "The server is limited to 1000 channels.");
            return false;
        }
        if (addChannel(channel, player) == -1)
            return false;
    }

    /* Because scripts might have caused the destruction of the previous channel,
       if the scripter puts some code in addChannel that would cause a masskick */
    if (!channelExist(channel))
        return false;

    int channelid = channelids[channel.toLower()];

    if (this->channel(channelid).players.contains(this->player(player))) {
        //already in the channel
        return true;
    }

    return joinChannel(player, channelid);
}

void Server::recvMessage(int id, int channel, const QString &mess)
{
    QString re = mess.trimmed();
    if (re.length() > 0) {
        if (myengine->beforeChatMessage(id, mess, channel)) {
            broadCast(mess, channel, id);
            myengine->afterChatMessage(id, mess, channel);
        }
    }
}

void Server::recvPM(int src, int dest, const QString &mess)
{
    if (playerLoggedIn(dest)) {
        Player *d = player(dest);
        Player *s = player(src);
        if(!d->hasKnowledgeOf(s)) {
            if (!myengine->beforeNewPM(src)) {
                return;
            }
        }

        d->acquireRoughKnowledgeOf(s);
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

    if(!myengine->beforeIPConnected(ip)){
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
    connect(p, SIGNAL(logout(int)), SLOT(logout(int)));
    connect(p, SIGNAL(recvTeam(int, QString)), SLOT(recvTeam(int, QString)));
    connect(p, SIGNAL(recvMessage(int, int, QString)), SLOT(recvMessage(int, int, QString)));
    connect(p, SIGNAL(disconnected(int)), SLOT(disconnected(int)));
    connect(p, SIGNAL(sendChallenge(int,int,ChallengeInfo)), SLOT(dealWithChallenge(int,int,ChallengeInfo)));
    connect(p, SIGNAL(battleFinished(int,int,int,int)), SLOT(battleResult(int,int,int,int)));
    connect(p, SIGNAL(info(int,QString)), SLOT(info(int,QString)));
    connect(p, SIGNAL(playerKick(int,int)), SLOT(playerKick(int, int)));
    connect(p, SIGNAL(playerBan(int,int)), SLOT(playerBan(int, int)));
    connect(p, SIGNAL(playerTempBan(int,int,int)), SLOT(playerTempBan(int, int, int)));
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
    connect(p, SIGNAL(joinRequested(int,int)), SLOT(joinChannel(int,int)));
    connect(p, SIGNAL(leaveRequested(int,int)), SLOT(leaveRequest(int,int)));
    connect(p, SIGNAL(ipChangeRequested(int,QString)), SLOT(ipChangeRequested(int,QString)));
    connect(p, SIGNAL(reconnect(int,int,QByteArray)), SLOT(onReconnect(int,int,QByteArray)));
    connect(p, SIGNAL(resendBattleInfos(int,int)), SLOT(resendBattleInfos(int,int)));
    connect(p, SIGNAL(needChannelData(int,int)), SLOT(needChannelData(int,int)));
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
                p->relay().notifyOptionsChange(src, away, p->state()[Player::LadderEnabled]);
            }
        }
    }
}

void Server::onReconnect(int sender, int id, const QByteArray &hash)
{
    if (!playerExist(id) || !player(id)->hasReconnectPass()) {
        player(sender)->relay().notify(NetworkServ::Reconnect, false, quint8(PlayerFlags::NoReconnectData));
        player(sender)->kick();
        return;
    }

    if (!player(id)->testReconnectData(player(sender), hash)) {
        // sender will automatically be dealt with
        return;
    }

    //proceed to reconnect
    transferId(sender, id);
}

void Server::transferId(int sender, int id, bool copyInfo)
{
    //printLine(QString("Transferring id %1 to %2").arg(sender).arg(id));
    bool loggedIn = player(id)->isLoggedIn();
    player(id)->associateWith(player(sender));
    /* This flag is triggered when someone logs in without intending to reconnect,
      but with sufficient info to allow for a reconnect. So we give them the intend
      to reconnect and update the info */
    if (copyInfo) {
        player(id)->changeState(Player::WaitingReconnect, true);
        sendMessage(id, tr("Your player session was still active on the server, so the data was kept. If you want to update your team/player info, just open the teambuilder and close it."));
    }
    if (!loggedIn) {
        emit player_incomingconnection(id);
        emit player_authchange(id, authedName(id));
    }
    player(id)->relay().notify(NetworkServ::Reconnect, true);
    processLoginDetails(player(id));
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
        connect(_c, SIGNAL(battleStarted(int,int,ChallengeInfo,int,int)), SLOT(startBattle(int, int, ChallengeInfo,int,int)));
    } catch (Challenge::Exception) {
        ;
    }
}

void Server::findBattle(int id, const FindBattleData &_f)
{
    FindBattleDataAdv f;
    (FindBattleData&)f = _f;

    Player *p1 = player(id);

    f.shuffle(p1->teamCount());

    QHash<int, FindBattleDataAdv*>::iterator it;
    for(it = battleSearchs.begin(); it != battleSearchs.end(); ++it)
    {
        int key = it.key();
        FindBattleDataAdv *data = it.value();
        Player *p2 = player(key);

        /* First look if this not a repeat */
        if (p2->lastFindBattleIp() == p1->ip() || p1->lastFindBattleIp() == p2->ip()) {
            continue;
        }

        for (int i = 0; i < f.shuffled.count(); i++) {
            for (int j = 0; j < data->shuffled.count(); j++) {
                const TeamBattle &t1 = p1->team(f.shuffled[i]);
                const TeamBattle &t2 = p2->team(data->shuffled[j]);

                if (t1.gen != t2.gen)
                    continue;

                /* We check the tier thing */
                if ( (f.sameTier || data->sameTier) && t1.tier != t2.tier)
                    continue;

                /* We check both allow rated if needed */
                if (f.rated || data->rated) {
                    if (!canHaveRatedBattle(id, key, t1, t2, f.rated, data->rated))
                        continue;
                }

                /* Then the range thing */
                if (f.ranged)
                    if (p1->rating(t1.tier) - f.range > p2->rating(t2.tier) || p1->rating(t1.tier) + f.range < p2->rating(t2.tier) )
                        continue;
                if (data->ranged)
                    if (p1->rating(t1.tier) - data->range > p2->rating(t2.tier) || p1->rating(t1.tier) + data->range < p2->rating(t2.tier) )
                        continue;

                //We have a match!
                ChallengeInfo c;
                c.opp = key;
                c.rated =  f.rated || data->rated || canHaveRatedBattle(id, key, t1, t2, f.rated, data->rated);
                c.clauses = TierMachine::obj()->tier(t1.tier).getClauses();
                c.mode = TierMachine::obj()->tier(t1.tier).getMode();
                c.gen = t1.gen;

                /* If someone has an invalid team, and it's not CC, cancel the match */
                if (!(c.clauses & ChallengeInfo::ChallengeCup) && (t1.invalid() || t2.invalid())) {
                    continue;
                }

                if (myengine->beforeBattleMatchup(id,key,c)) {
                    player(id)->lastFindBattleIp() = player(key)->ip();
                    player(key)->lastFindBattleIp() = player(id)->ip();
                    startBattle(id,key,c,f.shuffled[i], data->shuffled[j]);
                    myengine->afterBattleMatchup(id,key,c);
                    return;
                }
            }
        }
    }

    /* Not reached if a match was found */
    battleSearchs.insert(id, new FindBattleDataAdv(f));
    p1->battleSearch() = true;
}

bool Server::beforePlayerRegister(int src)
{
    return myengine->beforePlayerRegister(src);
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

bool Server::beforeChangeTier(int src, int n, const QString &old, const QString &dest)
{
    return myengine->beforeChangeTier(src, n, old, dest);
}

void Server::afterChangeTier(int src, int n, const QString &old, const QString &dest)
{
    myengine->afterChangeTier(src, n, old, dest);
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

void Server::inactivePlayersDeleteDaysChanged(int newValue) {
    if(amountOfInactiveDays == newValue) {
        return;
    }
    printLine("The amount of days that an user can stay inactive in database has been changed", false, true);
    amountOfInactiveDays = newValue;
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

void Server::overactiveToggleChanged(bool overactiveToggle)
{
    overactiveShow = overactiveToggle;
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
    if (serverPassword == pass.toUtf8())
        return;
    serverPassword = pass.toUtf8();
    printLine("Server Password changed", false, true);
}

void Server::usePasswordChanged(bool usePass)
{
    if (passwordProtected == usePass)
        return;
    passwordProtected = usePass;
    printLine("Require Server Password changed", false, true);
    regPasswordChanged(usePass);
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

    if (myengine->beforePlayerBan(src, dest, 0)) {
        if (!playerExist(src) || !playerExist(dest))
            return;
        ban(dest,src);
        myengine->afterPlayerBan(src, dest, 0);
    }
}

void Server::playerTempBan(int src, int dest, int time)
{
    if(!playerExist(dest)) {
        return;
    }
    if(player(dest)->auth() >= player(src)->auth()) {
        return;
    }
    int maxauth = SecurityManager::maxAuth(player(dest)->ip());

    if (player(src)->auth() <= maxauth) {
        player(src)->sendMessage("That player has authority level superior or equal to yours under another nick.");
        return;
    }

    if(myengine->beforePlayerBan(src, dest, time)) {
        if(!playerExist(src) || !playerExist(dest)) {
            return;
        }
        tempBan(dest, src, time);
        myengine->afterPlayerBan(src, dest, time);
    }
}

void Server::startBattle(int id1, int id2, const ChallengeInfo &c, int team1, int team2)
{
    int id = freebattleid();

    myengine->beforeBattleStarted(id1,id2,c,id,team1,team2);

    if (!playerExist(id1) || !playerExist(id2))
        return;

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

    BattleBase *battle;
    if (c.gen <= 1) {
        battle = (BattleBase*)new BattleRBY(*player(id1), *player(id2), c, id, team1, team2, pluginManager);
    } else {
        battle = new BattleSituation(*player(id1), *player(id2), c, id, team1, team2, pluginManager);
    }

    printLine(QString("%1 battle between %2 and %3 started").arg(battle->tier()).arg(name(id1)).arg(name(id2)));

    mybattles.insert(id, battle);
    battleList.insert(id, Battle(id1, id2, battle->mode(), battle->tier()));
    myengine->battleSetup(id1, id2, id); // dispatch script event

    Player *p1 (player(id1));
    Player *p2 (player(id2));

    if (!p1->isInSameChannel(p2)) {
        p1->relay().sendPlayer(p2->bundle());
        p2->relay().sendPlayer(p1->bundle());
    }

    p1->startBattle(id, id2, battle->pubteam(id1), battle->configuration(), battle->tier());
    p2->startBattle(id, id1, battle->pubteam(id2), battle->configuration(), battle->tier());

    Battle battleS = battleList[id];

    ++lastDataId;
    foreach(int chanid, p1->getChannels()) {
        Channel &chan = channel(chanid);
        if (!chan.battleList.contains(id)) {
            chan.battleList.insert(id,battleS); //Channel Battle List //QString::number(c.rated)
        }
        foreach(Player *p, chan.players) {
            /* That test avoids to send twice the same data to the client */
            if (p->id() != id1 && p->id() != id2 && !p->hasSentCommand(lastDataId)) {
                p->relay().notifyBattle(id,battleS);
            }
        }
    }
    foreach(int chanid, p2->getChannels()) {
        Channel &chan = channel(chanid);
        if (!chan.battleList.contains(id)) {
            chan.battleList.insert(id,battleS);
        }

        foreach(Player *p, chan.players) {
            /* That test avoids to send twice the same data to the client */
            if (p->id() != id1 && p->id() != id2 && !p->hasSentCommand(lastDataId)) {
                p->relay().notifyBattle(id,battleS);
            }
        }
    }

    connect(battle, SIGNAL(battleInfo(int,int,QByteArray)), SLOT(sendBattleCommand(int,int,QByteArray)));
    connect(battle, SIGNAL(battleFinished(int,int,int,int)), SLOT(battleResult(int, int,int,int)));

    battle->start(battleThread);

    myengine->afterBattleStarted(id1,id2,c,id,team1,team2);
}

bool Server::canHaveRatedBattle(int id1, int id2, const TeamBattle &t1, const TeamBattle &t2, bool force1, bool force2)
{
    Player *p1 = player(id1);
    Player *p2 = player(id2);
    if (!force1 && !p1->ladder())
        return false;
    if (!force2 && !p2->ladder())
        return false;
    if (t1.tier != t2.tier)
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

    BattleBase *battle = mybattles[battleid];
    QString tier = battle->tier();
    bool rated = battle->rated();

    qDebug() << "battleResult " << battleid << desc << winner << loser << battle;

    if (winner == 0) {
        winner = battle->id(battle->opponent(battle->spot(loser)));
    }

    bool disconnected = (!playerLoggedIn(winner) || !playerLoggedIn(loser)) && dynamic_cast<Player*>(sender()) != NULL; // if the battle sent the event, then it's legit win

    Player *pw = player(winner);
    Player *pl = player(loser);

    if (desc == Forfeit && battle->finished()) {
        pw->battleResult(battleid, Close, battle->mode(), winner, loser);
        pl->battleResult(battleid, Close, battle->mode(), winner, loser);
    } else {
        int _desc = disconnected ? Tie : desc;

        if (_desc == Forfeit) {
            battle->playerForfeit(loser);
        }

        QString winn = pw->name();
        QString lose = pl->name();

        myengine->beforeBattleEnded(winner, loser, _desc, battleid);

        ++lastDataId;
        foreach(int chanid, pw->getChannels()) {
            Channel &chan = channel(chanid);
            chan.battleList.remove(battleid);
            foreach(Player *p, chan.players) {
                /* That test avoids to send twice the same data to the client */
                if (!p->hasSentCommand(lastDataId)) {
                    p->battleResult(battleid, _desc, battle->mode(), winner, loser);
                }
            }
        }
        foreach(int chanid, pl->getChannels()) {
            Channel &chan = channel(chanid);
            chan.battleList.remove(battleid);
            foreach(Player *p, chan.players) {
                /* That test avoids to send twice the same data to the client */
                if (!p->hasSentCommand(lastDataId)) {
                    p->battleResult(battleid, _desc, battle->mode(), winner, loser);
                }
            }
        }
        if (_desc == Forfeit) {
            printLine(QString("%1 forfeited his battle against %2").arg(name(loser), name(winner)));
        } else if (_desc == Win) {
            printLine(QString("%1 won his battle against %2").arg(name(winner), name(loser)));
        } else if (_desc == Tie) {
            printLine(QString("%1 and %2 tied").arg(name(winner), name(loser)));
        }

        if (_desc != Tie && rated) {
            TierMachine::obj()->changeRating(winn, lose, tier);
            if (playerExist(pw->id()))
                pw->findRating(tier);
            if (playerExist(pl->id()))
                pl->findRating(tier);
        }

        myengine->afterBattleEnded(winner, loser, _desc, battleid);
    }

    if (desc == Forfeit) {
        removeBattle(battleid);
    }

    qDebug() << "battleResult " << battleid << desc << winner << loser << battle << "end";
}

void Server::removeBattle(int battleid)
{
    BattleBase *battle = mybattles.value(battleid);
    qDebug() << "removing battle " << battleid << battle;

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

    if (!source->isLoggedIn()) {
        return;
    }

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
    //printLine(tr("Removing a player, count: %1").arg(numberOfPlayersLoggedIn));
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

    //Todo: not send the tier list when unnecessary (no team change)
    if (oldname == _name) {
        /* Haha, same name so no need to do anything! */
        source->relay().sendTeam(0, source->getTierList());
    } else {
        /* Changing the name! */
        mynames.remove(oldname.toLower());
        mynames.insert(_name.toLower(), id);
        source->setName(_name);
        source->relay().sendTeam(&_name, source->getTierList());
    }

    /* Sending the name change! */
    sendPlayer(id);

    emit player_authchange(id, authedName(id));

    myengine->afterChangeTeam(id);
}

void Server::disconnected(int id)
{
    printLine(QString("Received disconnection from %1 (%2)").arg(name(id)).arg(id));
    disconnectPlayer(id);
}

void Server::logout(int id)
{
    printLine(QString("Received logout from %1 (%2)").arg(name(id)).arg(id));
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
    BattleBase *battle = mybattles.value(idOfBattle);
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
    if (!mybattles.contains(idOfBattle)) {
        printLine(QString("Critical bug needing to be solved: Server::spectatingStopped, player %1 (%2) and non-existent battle %3").arg(id).arg(name(id)).arg(idOfBattle));
    } else {
        mybattles[idOfBattle]->removeSpectator(id);
    }
}

void Server::disconnectPlayer(int id)
{
    if (playerExist(id))
    {
        Player *p = player(id);
        bool loggedIn = p->isLoggedIn();

        if (!loggedIn || !p->hasReconnectPass()) {
            removePlayer(id);
            return;
        }

        if (loggedIn) {
            myengine->beforeLogOut(id);
        }

        for (int i = 0; i < LastGroup; i++) {groups[i].remove(p); oppGroups[i].remove(p);}

        p->doWhenDC();

        p->blockSignals(true);

        QString playerName = p->name();

        AntiDos::obj()->disconnect(p->ip(), id);

        foreach(int chanid, p->getChannels()) {
            leaveRequest(id, chanid, true);
        }

        emit player_logout(id);

        /* Sending the notice of logout to others only if the player is already logged in */
        if (loggedIn) {
            sendLogout(id);
            myengine->afterLogOut(id);
        }

        p->changeState(Player::LoggedIn, false);
        p->changeState(Player::WaitingReconnect, true);

        QTimer::singleShot(7*60*1000, p, SLOT(autoKick()));

        printLine(QString("Disconnected player %1 (%2)").arg(playerName).arg(id));
    }
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

        for (int i = 0; i < LastGroup; i++) {groups[i].remove(p); oppGroups[i].remove(p);}

        p->doWhenDQ();

        p->blockSignals(true);

        QString playerName = p->name();

        if (!p->waitingForReconnect() && !p->discarded()) {
            AntiDos::obj()->disconnect(p->ip(), id);
        }

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

        if ((loggedIn || p->state()[Player::WaitingReconnect]) && mynames.value(playerName.toLower()) == p->id())
            mynames.remove(playerName.toLower());

        printLine(QString("Removed player %1 (%2)").arg(playerName).arg(id));
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

void Server::broadCast(const QString &message, int channel, int sender, bool html, int target)
{
    QString fullMessage = message;
    bool chatMessage = false;

    if (sender != NoSender) {
        if (sender == 0) {
            fullMessage = QString("~~Server~~: %1").arg(message);
        } else {
            chatMessage = true;
            fullMessage = QString("%1: %2").arg(name(sender), message);
        }
    }

    if (target != NoTarget) {
        Player *p = player(target);
        if (p->spec()[Player::IdsWithMessage] && sender != NoSender) {
            if (channel != NoChannel) {
                p->relay().notify(NetworkServ::SendChatMessage, Flags(3), Flags(html), channel, sender, message);
            } else {
                p->relay().notify(NetworkServ::SendChatMessage, Flags(2), Flags(html), sender, message);
            }
        } else {
            if (channel != NoChannel) {
                p->relay().notify(NetworkServ::SendChatMessage, Flags(1), Flags(html), channel, fullMessage);
            } else {
                p->relay().notify(NetworkServ::SendChatMessage, Flags(0), Flags(html), fullMessage);
            }
        }
    } else {
        if (channel != NoChannel) {
            if(useChannelFileLog) {
                this->channel(channel).log(fullMessage);
            }
            printLine(QString("[#%1] %2").arg(this->channel(channel).name, fullMessage), chatMessage, true);
            if (sender == NoSender) {
                notifyChannel(channel, All, NetworkServ::SendChatMessage, Flags(1), Flags(html), channel, message);
            } else {
                notifyChannel(channel, IdsWithMessage, NetworkServ::SendChatMessage, Flags(3), Flags(html), channel, sender, message);
                notifyChannelOpp(channel, IdsWithMessage, NetworkServ::SendChatMessage, Flags(1), Flags(html), channel, fullMessage);
            }
        } else {
            printLine(fullMessage, chatMessage, true);

            if (sender == NoSender) {
                notifyGroup(All, NetworkServ::SendChatMessage, Flags(0), Flags(html), message);
            } else {
                notifyGroup(IdsWithMessage, NetworkServ::SendChatMessage, Flags(2), Flags(html), sender, message);
                notifyOppGroup(IdsWithMessage, NetworkServ::SendChatMessage, Flags(0), Flags(html), fullMessage);
            }
        }
    }
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

#ifdef _WIN32
    ::exit(0);
#endif
    // On linux, threads need to be cleared or the server may be left hanging...
//    TierMachine::destroy();
//    SecurityManager::destroy();
//    RelayManager::destroy();

//    connect(&battleThread, SIGNAL(finished()), this, SLOT(deleteLater()));
//    battleThread.finish();
//    exit(0);
    //The above always hangs now for some reason on linux, so here we go
    int *x = NULL;
    *x += 1;

}

void Server::setAnnouncement(int &id, const QString &html) {
    if (player(id)->isLoggedIn())
            player(id)->relay().notify(NetworkServ::Announcement, html);;
}

Player * Server::player(int id) const
{
    if (!myplayers.contains(id)) {
        qDebug() << "Fatal! player called for non existing ID " << id;
        dump_backtrace();
    }
    return myplayers.value(id);
}

PlayerInterface * Server::playeri(int id) const
{
    return player(id);
}

BattleBase * Server::getBattle(int battleId) const
{
    if(mybattles.contains(battleId)) {
        return mybattles.value(battleId);
    }else{
        return NULL;
    }
}

bool Server::correctPass(const QByteArray &hash, const QByteArray &salt) const {
    return hash == QCryptographicHash::hash(QCryptographicHash::hash(serverPassword, QCryptographicHash::Md5) + salt, QCryptographicHash::Md5);
}

bool Server::isLegalProxyServer(const QString &ip) const
{
    foreach (QString proxyip, proxyServers) {
        if (ip == proxyip)
            return true;
    }
    return false;
}

template <typename ...Params>
void Server::notifyGroup(PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    notifyGroup(group, packet);
}

template <typename ...Params>
void Server::notifyGroup(const QSet<Player*>& group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);

    foreach(Player *p, group) {
        p->sendPacket(packet);
    }
}

void Server::notifyGroup(PlayerGroupFlags group, const QByteArray &packet)
{
    const QSet<Player*> &g = getGroup(group);

    foreach(Player *p, g) {
        p->sendPacket(packet);
    }
}

template <typename ...Params>
void Server::notifyOppGroup(PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);

    const QSet<Player*> &g = getOppGroup(group);

    foreach(Player *p, g) {
        p->sendPacket(packet);
    }
}

template <typename ...Params>
void Server::notifyChannel(int channel, PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    const QSet<Player*> &g1 = getGroup(group);
    const QSet<Player*> &g2 = this->channel(channel).players;

    if (g1.size() < g2.size()) {
        foreach(Player *p, g1) {
            if (p->inChannel(channel)) {
                p->sendPacket(packet);
            }
        }
    } else {
        foreach(Player *p, g2) {
            if (group == All || p->spec()[group]) {
                p->sendPacket(packet);
            }
        }
    }
}

template <typename ...Params>
void Server::notifyChannelOpp(int channel, PlayerGroupFlags group, int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    const QSet<Player*> &g1 = getOppGroup(group);
    const QSet<Player*> &g2 = this->channel(channel).players;

    if (g1.size() < g2.size()) {
        foreach(Player *p, g1) {
            if (p->inChannel(channel)) {
                p->sendPacket(packet);
            }
        }
    } else {
        foreach(Player *p, g2) {
            if (!p->spec()[group]) {
                p->sendPacket(packet);
            }
        }
    }
}

template <typename ...Params>
void Server::notifyAll(int command, Params &&... params)
{
    QByteArray packet = makePacket(command, std::forward<Params>(params)...);
    foreach(Player *p, myplayers) {
        p->sendPacket(packet);
    }
}

#ifndef SERVER_H
#define SERVER_H

#include "../Utilities/contextswitch.h"
#include "../PokemonInfo/networkstructs.h"
#include "serverinterface.h"
#include "sfmlsocket.h"
#include "channel.h"

#define PRINTOPT(a, b) (fprintf(stdout, "  %-25s\t%s\n", a, b))

/* the server */

class FindBattleData;
class FindBattleDataAdv;
class Player;
class BattleBase;
class Analyzer;
class BattleChoice;
class ChallengeInfo;
class ScriptEngine;
class Challenge;
class QTcpServer;
class PluginManager;
class TeamBattle;

class Server: public QObject, public ServerInterface
{
    Q_OBJECT

    friend class ScriptEngine;
    friend class ServerWidget;
    friend class ConsoleReader;
public:
    enum PlayerGroupFlags {
        All = 0,
        SupportsZip = 1,
        IdsWithMessage = 2,
        LastGroup
    };
    enum {
        NoSender = -1,
        NoChannel = -1,
        NoTarget = -1
    };

    Server(quint16 port = 5080);
    Server(QList<quint16> ports);
    ~Server();

    void start();

    static void print(const QString &line);
    bool printLine(const QString &line, bool chatMessage = false, bool forcedLog = false);
    /* returns the name of that player */
    QString name(int id) const;
    QString authedName(int id) const;
    void broadCast(const QString &message, int channel = NoChannel, int sender = NoSender, bool html = false, int target=NoTarget);
    void sendMessage(int id, const QString &message);

    void sendBattlesList(int id, int chanid);
    /* Sends the login of the player to everybody but the player */
    void sendLogin(int id);
    void sendLogout(int id);
    void sendTierList(int id);
    bool playerExist(int id) const;
    bool playerLoggedIn(int id) const;
    bool nameExist(const QString &name) const;
    int id(const QString &name) const;
    int auth(int id) const;
    int dosChannel() const;
    int channelId(const QString &chanName) const;
    void removeBattle(int battleid);
    void beforeChallengeIssued(int src, int dest, Challenge *c);
    void afterChallengeIssued(int src, int dest, Challenge *c);
    bool beforeFindBattle(int src);
    void afterFindBattle(int src);
    bool beforeChangeTier(int src, int teamNum, const QString &oldTier, const QString &newTier);
    void afterChangeTier(int src, int teamNum, const QString &oldTier, const QString &newTier);
    bool beforePlayerAway(int src, bool away);
    void afterPlayerAway(int src, bool away);
    void disconnectFromRegistry();
    void setAnnouncement(int &id, const QString &html);
    void setAllAnnouncement(const QString &html);
    /* Force Rated 1 and Force Rated 2 is to ignore the ladder on / off factor for those two */
    bool canHaveRatedBattle(int id1, int id2, const TeamBattle &t1, const TeamBattle &t2, bool forceRated1 = false, bool forceRated2 = false);

    static Server *serverIns;
    static QString dataRepo;

    BattleBase * getBattle(int battleId) const;

    const QString &servName() {
        return serverName;
    }
    bool isSafeScripts() const { return safeScripts; }
    bool isPrivate() const { return serverPrivate == 1; }
    bool isLegalProxyServer(const QString &ip) const;

    bool isPasswordProtected() const { return passwordProtected; }

    int playerDeleteDays() const { return amountOfInactiveDays; }
    QString description();

    bool correctPass(const QByteArray &hash, const QByteArray &salt) const;
    void processLoginDetails(Player *p);
signals:
    void chatMessage(const QString &name);
    void serverMessage(const QString &name);

    void player_incomingconnection(int id);
    void player_logout(int id);
    void player_authchange(int id, const QString &name);

public slots:
    /* Registry slots */
    void connectToRegistry();
    void clearRatedBattlesHistory();
    void regConnected();
    void regConnectionError();
    void regSendPlayers();
    void regNameChanged(const QString &name);
    void regDescChanged(const QString &desc);
    void regMaxChanged(const int &num);
    void regPasswordChanged(bool &newValue);
    void changeScript(const QString &script);
    void reloadTiers();
    void announcementChanged(const QString &announcement);
    void mainChanChanged(const QString &mainChan);
    void regPrivacyChanged(const int &priv);
    void logSavingChanged(bool logging);
    void inactivePlayersDeleteDaysChanged(int newValue);
    void useChannelFileLogChanged(bool logging);
    void TCPDelayChanged(bool lowTCP);
    void safeScriptsChanged(bool safeScripts);
    void overactiveToggleChanged(bool overactiveToggle);
    void proxyServersChanged(const QString &ips);
    void serverPasswordChanged(const QString &pass);
    void usePasswordChanged(bool usePass);
    void changeDbMod(const QString &mod);

    void nameTaken();
    void ipRefused();
    void invalidName();
    void accepted();
    /* means a new connection is about to start from the TCP server */
    /* i is the number of the listening port */
    void incomingConnection(int i);
    /* Signals received by players */
    void loggedIn(int id, const QString &name);
    void sendServerMessage(const QString &message);
    void recvMessage(int id, int chanid, const QString &mess);
    void recvPM(int src, int dest, const QString &mess);
    void recvTeam(int id, const QString &name);
    void disconnected(int id);
    void logout(int id);
    void dealWithChallenge(int from, int to, const ChallengeInfo &c);
    void startBattle(int id1, int id2, const ChallengeInfo &c, int team1=0,int team2=0);
    void battleResult(int battleid, int desc, int winner, int loser);
    void sendBattleCommand(int battleId, int id, const QByteArray &command);
    void spectatingRequested(int id, int battle);
    void spectatingStopped(int id, int battle);
    void battleMessage(int player, int battle, const BattleChoice &message);
    void battleChat(int player, int battle, const QString &chat);
    void resendBattleInfos(int player, int battle);
    void spectatingChat(int player, int battle, const QString &chat);
    bool joinRequest(int player, const QString &chn);
    /* Makes a player join a channel */
    bool joinChannel(int playerid, int chanid);
    void leaveRequest(int player, int chan, bool keepChannelForPlayer=false);
    void ipChangeRequested(int player, const QString &ip);
    void info(int , const QString &);
    void onReconnect(int, int, const QByteArray &);
    void transferId(int id1, int id2, bool copyInfo=false);
    void needChannelData(int playerId, int chanId);

    void kick(int i);
    void silentKick(int i);
    void ban(int i);
    void dosKick(int id);
    void dosBan(const QString &ip);

    void changeAuth(const QString &name, int auth);
    void banName(const QString &name);
    void playerKick(int src, int dest);
    void playerBan(int src, int dest);
    void playerTempBan(int src, int dest, int time);
    void awayChanged(int src, bool away);
    void sendPlayer(int id);
    void tiersChanged();
    void findBattle(int id,const FindBattleData &f);
    void cancelSearch(int id);
    void loadRatedBattlesSettings();

    void processDailyRun();
    void updateDatabase();
    void updateRatings();

    void atServerShutDown();
private:
    void kick(int dest, int src);
    void ban(int dest, int src);
    void tempBan(int dest, int src, int time);

    Analyzer *registry_connection;
    QString serverName, serverDesc;
    QByteArray serverAnnouncement;
    QByteArray zippedAnnouncement;
    QByteArray zippedTiers;
    quint16 serverPrivate, serverPlayerMax;
    QList<quint16>  serverPorts;
    QStringList proxyServers;
    bool showLogMessages;
    bool useChannelFileLog;
    int amountOfInactiveDays;
    bool lowTCPDelay;
    bool safeScripts;
    bool overactiveShow;
    bool passwordProtected;
    QByteArray serverPassword;

    quint16 numPlayers() {
        return myplayers.size();
    }

    /* When sending messages to several players, we don't want to send them twice to the same person.

       We could construct a set everytime to remember players to who we already sent the command,
        but that would be doing too many allocations. Instead we use a commandId for this kind of commands,
        and check that the last command sent to the player wasn't that particular command. */
    int lastDataId;
    /* Counters for ids.

        They have two advantages: you can get a non used id fast, there's astronomically low chances that
        an id on a client doesn't corresponds to an id on the server, because the lag between the client
        and the server would have to live to see a full loop of these counters. (lag seeing a few billion
        players go by... lol ^^)

        The disavandtage is that you don't have clean ids, that are close to 0. */
    mutable int playercounter, battlecounter, channelcounter;

#ifndef SFML_SOCKETS
    QList<QTcpServer *> myservers;
#else
    QList<GenericSocket> myservers;
    SocketManager manager;
#endif
    PluginManager *pluginManager;

    /* storing players */
    QHash<int, Player*> myplayers;
    QHash<QString, int> mynames;

    QHash<int, Channel*> channels;
    QHash<QString, int> channelids;
public:
    QHash<qint32, QString> channelNames;
private:
    Cache<QByteArray, void (*)(QByteArray&)> channelCache;
    Cache<QByteArray, void (*)(QByteArray&)> zchannelCache;

    QHash<int, BattleBase *> mybattles;
    QHash<qint32, Battle> battleList;

#ifndef SFML_SOCKETS
    QTcpServer *server(int i);
#else
    GenericSocket server(int i);
#endif
public:
    Player * player(int i) const;
    PlayerInterface * playeri(int i) const;
private:
    /* gets an id that's not used */
    int freeid() const;
    int freebattleid() const;
    int freechannelid() const;
    /* removes a player */
    void disconnectPlayer(int id); // keeps info in case of a reconnect
    void removePlayer(int id); // keeps no info
    /* creates a channel */
    int addChannel(const QString &name="", int playerid=0);
    void removeChannel(int channelid);
    /* Sends the list of channels to a player */
    void sendChannelList(int player);

    inline bool channelExist(int id) const {
        return channels.contains(id);
    }

    inline bool channelExist(const QString &name) const {
        return channelids.contains(name.toLower());
    }

    inline Channel &channel(int id) const {
        return * channels[id];
    }

    inline bool channelContains(int chanid, int playerid) const {
        return channel(chanid).players.contains(player(playerid));
    }

    int numberOfPlayersLoggedIn;

public:
    bool allowRatedWithSameIp;
    bool allowThroughChallenge;
    int diffIpsForRatedBattles;
private:
    QHash<QString, QList<QString> > lastRatedIps;

    ScriptEngine *myengine;

    QHash<int, FindBattleDataAdv*> battleSearchs;

    ContextSwitcher battleThread;

    template <typename ...Params>
    void notifyGroup(PlayerGroupFlags group, int command, Params &&... params);

    template <typename ...Params>
    void notifyGroup(const QSet<Player*> &group, int command, Params &&... params);

    void notifyGroup(PlayerGroupFlags group, const QByteArray &packet);


    template <typename ...Params>
    void notifyOppGroup(PlayerGroupFlags group, int command, Params &&... params);

    template <typename ...Params>
    void notifyChannel(int channel, PlayerGroupFlags group, int command, Params &&... params);

    template <typename ...Params>
    void notifyChannelOpp(int channel, PlayerGroupFlags group, int command, Params &&... params);

    template <typename ...Params>
    void notifyAll(int command, Params &&... params);

    const QSet<Player*> &getGroup(PlayerGroupFlags group) const;
    const QSet<Player*> &getOppGroup(PlayerGroupFlags group) const;

    QSet<Player*> groups[LastGroup];
    QSet<Player*> oppGroups[LastGroup];
};
#endif // SERVER_H

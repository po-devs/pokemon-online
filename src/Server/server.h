#ifndef SERVER_H
#define SERVER_H

#include <QtNetwork>
#include "../Utilities/contextswitch.h"
#include "../PokemonInfo/networkstructs.h"
/* the server */

class FindBattleData;
class Player;
class BattleSituation;
class Analyzer;
class BattleChoice;
class ChallengeInfo;
class ScriptEngine;
class Challenge;

class Server: public QObject
{
    Q_OBJECT

    friend class ScriptEngine;
public:
    Server(quint16 port = 5080);
    void start();

    static void print(const QString &line);
    bool printLine(const QString &line, bool chatMessage = false);
    /* returns the name of that player */
    QString name(int id) const;
    QString authedName(int id) const;
    /* sends a message to all the players */
    void sendAll(const QString &message, bool chatMessage = false);
    void sendMessage(int id, const QString &message);
    void sendPlayersList(int id);
    void sendBattlesList(int id);
    /* Sends the login of the player to everybody but the player */
    void sendLogin(int id);
    void sendLogout(int id);
    void sendTierList(int id);
    bool playerExist(int id) const;
    bool playerLoggedIn(int id) const;
    bool nameExist(const QString &name) const;
    int id(const QString &name) const;
    int auth(int id) const;
    void removeBattle(int battleid);
    void beforeChallengeIssued(int src, int dest, Challenge *c);
    void afterChallengeIssued(int src, int dest, Challenge *c);
    bool beforeChangeTier(int src, const QString &oldTier, const QString &newTier);
    void afterChangeTier(int src, const QString &oldTier, const QString &newTier);
    bool beforePlayerAway(int src, bool away);
    void afterPlayerAway(int src, bool away);
    void disconnectFromRegistry();
    /* Force Rated 1 and Force Rated 2 is to ignore the ladder on / off factor for those two */
    bool canHaveRatedBattle(int id1, int id2, bool challengeCup, bool forceRated1 = false, bool forceRated2 = false);

    Player * player(int id) const;

    void sendServerMessage(const QString &message);

    static Server *serverIns;

signals:
    void chatmessage(const QString &name);
    void servermessage(const QString &name);

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
    void changeScript(const QString &script);
    void announcementChanged(const QString &announcement);
    void nameTaken();
    void ipRefused();
    void invalidName();
    void accepted();
    /* means a new connection is about to start from the TCP server */
    void incomingConnection();
    /* Signals received by players */
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void recvPM(int src, int dest, const QString &mess);
    void recvTeam(int id, const QString &name);
    void disconnected(int id);
    void dealWithChallenge(int from, int to, const ChallengeInfo &c);
    void startBattle(int id1, int id2, const ChallengeInfo &c);
    void battleResult(int battleid, int desc, int winner, int loser);
    void sendBattleCommand(int battleId, int id, const QByteArray &command);
    void spectatingRequested(int id, int battle);
    void spectatingStopped(int id, int battle);
    void battleMessage(int player, int battle, const BattleChoice &message);
    void battleChat(int player, int battle, const QString &chat);
    void spectatingChat(int player, int battle, const QString &chat);
    void info(int , const QString& );

    void kick(int i);
    void silentKick(int i);
    void ban(int i);
    void dosKick(int id);
    void dosBan(const QString &ip);

    void changeAuth(const QString &name, int auth);
    void banName(const QString &name);
    void playerKick(int src, int dest);
    void playerBan(int src, int dest);
    void awayChanged(int src, bool away);
    void sendPlayer(int id);
    void tiersChanged();
    void findBattle(int id,const FindBattleData &f);
    void cancelSearch(int id);
    void loadRatedBattlesSettings();
    void regPrivacyChanged(const int &priv);

    void atServerShutDown();
private:
    void kick(int dest, int src);
    void ban(int dest, int src);

    Analyzer *registry_connection;
    QString serverName, serverDesc, serverAnnouncement;
    quint16 serverPrivate, serverPlayerMax,serverPort;
    quint16 numPlayers() {
        return myplayers.size();
    }

    QTcpServer myserver;
    /* storing players */
    QHash<int, Player*> myplayers;
    QHash<QString, int> mynames;

    QHash<int, BattleSituation *> mybattles;
    QHash<int, Battle> battleList;

    QTcpServer *server();
    Player * player(int i);
    /* gets an id that's not used */
    int freeid() const;
    int freebattleid() const;
    /* removes a player */
    void removePlayer(int id);

    int numberOfPlayersLoggedIn;

    bool allowRatedWithSameIp;
    int diffIpsForRatedBattles;
    QHash<QString, QList<QString> > lastRatedIps;

    ScriptEngine *myengine;

    QHash<int, FindBattleData*> battleSearchs;

    ContextSwitcher battleThread;
};
#endif // SERVER_H

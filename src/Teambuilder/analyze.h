/**
 * See network protocol here: http://wiki.pokemon-online.eu/view/Network_Protocol_v2
*/

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "../Utilities/network.h"
#include "../Utilities/coreclasses.h"
#include "../PokemonInfo/networkstructs.h"

class Client;
class PlayerInfo;
struct BattleChoice;
class TeamBattle;
struct BattleConfiguration;
struct ChallengeInfo;
struct Battle;
struct UserInfo;
class TeamHolder;
struct ServerInfo;

/* Commands to dialog with the server */
namespace NetworkCli
{
#include "../Shared/networkcommands.h" 
}

/* Analyzes the messages received from the network and emits the corresponding signals.
   Also allows you to send your own messages to the network

    The client actually uses this widgets to send orders and receive commands from the outside. */

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(bool registry_connection = false);

    /* functions called by the client */
    void login(const TeamHolder &team, bool ladder, bool away=false, const QColor &color=QColor(), const QString &defaultChannel=QString(), const QStringList &autoJoin=QStringList());
    /* Sends a logout message, and deletes the analyzer */
    void logout();
    Q_INVOKABLE void sendChanMessage(int channelid, const QString &message);
    void connectTo(const QString &host, quint16 port);
    void sendTeam(const TeamHolder & team);
    void sendBattleResult(int id, int result);
    bool isConnected() const;

    QString getIp() const;
    quint32 getCommandCount() const {return commandCount;}
    void disconnectFromHost();

    /* Convenience functions to avoid writing a new one every time */
    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emit sendCommand(tosend);
    }
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void connected();
    void disconnected();
    /* Message to appear in all the mainchats */
    void messageReceived(const QString &mess);
    void htmlMessageReceived(const QString &mess);
    void channelMessageReceived(const QString &mess, int channel, bool html);
    /* Command specific to a channel */
    void channelCommandReceived(int command, int channel, DataStream *stream);
    /* player from the players list */
    void playerReceived(const PlayerInfo &p);
    /* login of a player */
    void playerLogin(const PlayerInfo &p, const QStringList& tiers);
    void teamApproved(const QStringList &tiers);
    /* logout... */
    void playerLogout(int id);
    /* challengerelated */
    void challengeStuff(const ChallengeInfo &c);
    /* battle including self */
    void battleStarted(int battleid, const Battle &battle, const TeamBattle &myteam, const BattleConfiguration &conf);
    /* battle of strangers */
    void battleStarted(int battleid, const Battle &battle);
    void battleFinished(int battleid, int res, int srcid, int destid);
    void battleMessage(int battleid, const QByteArray &mess);
    void spectatedBattle(int battleId, const BattleConfiguration &conf);
    void spectatingBattleMessage(int battleId, const QByteArray &mess);
    void spectatingBattleFinished(int battleId);
    void passRequired(const QByteArray &salt);
    void serverPassRequired(const QByteArray &salt);
    void notRegistered(bool);
    void playerKicked(int p, int src);
    void playerBanned(int p, int src);
    void playerTempBanned(int p, int src, int time);
    void regAnnouncementReceived(const QString &announcement);
    void serverReceived(const ServerInfo &info);
    void PMReceived(int id, const QString &mess);
    void awayChanged(int id, bool away);
    void ladderChanged(int id, bool ladder);
    void tierListReceived(const QByteArray &tl);
    void announcement(const QString &announcement);
    /* From the control panel */
    void userInfoReceived(const UserInfo &ui);
    void userAliasReceived(const QString &s);
    void banListReceived(const QString &n, const QString &ip, const QDateTime &expires);
    void versionDiff(const ProtocolVersion&, int level);

    void serverNameReceived(const QString &serverName);
    /* Ranking */
    void rankingStarted(int,int,int);
    void rankingReceived(const QString&,int);
    void channelsListReceived(const QHash<qint32, QString> &channels);
    void channelPlayers(int chanid, const QVector<qint32> &channels);
    void addChannel(QString name, int id);
    void channelNameChanged(int id, const QString &name);
    void removeChannel(int id);
    void reconnectPassGiven(const QByteArray&);
    void reconnectSuccess();
    void reconnectFailure(int reason);
public slots:
    /* slots called by the network */
    void error();
    void wasConnected();
    void commandReceived (const QByteArray &command);

    /* by a channel */
    void channelCommand(int command, int channelid, const QByteArray &body);

    /* by the battle window */
    void battleCommand(int, const BattleChoice &comm);
    void battleMessage(int, const QString &mess);

    /* By the pm window */
    void sendPM(int id, const QString &mess);

    /* By the control panel */
    void getUserInfo(const QString &name);
    void getBanList();
    void CPUnban(const QString &name);
    /* By the rankings window */
    void getRanking(const QString &tier, const QString &name);
    void getRanking(const QString &tier, int page);

    /* By the challenge window */
    void sendChallengeStuff(const ChallengeInfo &c);

private:
    typedef Network<QTcpSocket*> network_type;
    /* The connection to the outside */
    network_type &socket();
    const network_type &socket() const;
    /* To tell if its the registry we're connected to*/
    bool registry_socket;

    quint32 commandCount;

    QList<QByteArray> storedCommands;
    QSet<int> channelCommands;

    network_type mysocket;

    /* Server version */
    ProtocolVersion version;
};

#endif // ANALYZE_H

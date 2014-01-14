#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class Client;
class FullInfo;
class PlayerInfo;
class BattleChoice;
class TeamBattle;
class BattleConfiguration;
class ChallengeInfo;
class UserInfo;
class TrainerTeam;

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
    void login(const FullInfo &team);
    void sendMessage(const QString &message);
    void connectTo(const QString &host, quint16 port);
    void sendTeam(const TrainerTeam & team);
    void sendChallengeStuff(const ChallengeInfo &c);
    void sendBattleResult(int result);
    bool isConnected() const;
    void goAway(bool away);

    /* Convenience functions to avoid writing a new one every time */
    void notify(int command);
    template<class T>
    void notify(int command, const T& param);
    template<class T1, class T2>
    void notify(int command, const T1& param1, const T2& param2);
    template<class T1, class T2, class T3>
    void notify(int command, const T1& param1, const T2& param2, const T3 &param3);
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void connected();
    void disconnected();
    void messageReceived(const QString &mess);
    /* player from the players list */
    void playerReceived(const PlayerInfo &p);
    /* login of a player */
    void playerLogin(const PlayerInfo &p);
    /* Change of team of a player */
    void teamChanged(const PlayerInfo &p);
    /* logout... */
    void playerLogout(int id);
    /* challengerelated */
    void challengeStuff(const ChallengeInfo &c);
    /* battle including self */
    void battleStarted(int id, const TeamBattle &myteam, const BattleConfiguration &conf);
    /* battle of strangers */
    void battleStarted(int id1, int id2);
    void battleFinished(int res, int srcid, int destid);
    void battleMessage(const QByteArray &mess);
    void spectatedBattle(const QString& name0, const QString &name1, int battleId);
    void spectatingBattleMessage(int battleId, const QByteArray &mess);
    void spectatingBattleFinished(int battleId);
    void passRequired(const QString &salt);
    void notRegistered(bool);
    void playerKicked(int p, int src);
    void playerBanned(int p, int src);
    void serverReceived(const QString &name, const QString &desc, quint16 num_players, const QString &ip, quint16 max);
    void PMReceived(int id, const QString &mess);
    void awayChanged(int id, bool away);
    void tierListReceived(const QString &tl);
    /* From the control panel */
    void userInfoReceived(const UserInfo &ui);
    void userAliasReceived(const QString &s);
    void banListReceived(const QString &n, const QString &ip);
    void versionDiff(const QString &a, const QString &b);
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

    /* by the battle window */
    void battleCommand(const BattleChoice &comm);
    void battleMessage(const QString &mess);
    /* spectator window */
    void battleMessage(const QString &mess, int);

    /* By the pm window */
    void sendPM(int id, const QString &mess);

    /* By the control panel */
    void getUserInfo(const QString &name);
    void getBanList();
    void CPUnban(const QString &name);

private:
    /* The connection to the outside */
    Network &socket();
    /* To tell if its the registry we're connected to*/
    bool registry_socket;

    Network mysocket;
};

template<class T>
void Analyzer::notify(int command, const T& param)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);
    out << param;

    emit sendCommand(tosend);
}

template<class T1, class T2>
void Analyzer::notify(int command, const T1& param1, const T2 &param2)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3;

    emit sendCommand(tosend);
}

#endif // ANALYZE_H

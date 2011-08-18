#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include <QColor>
#include "network.h"

class TeamBattle;
class Battle;
class BattleChoice;
class BattleConfiguration;
class ChallengeInfo;
class UserInfo;
class PlayerInfo;
class FindBattleData;

/* Commands to dialog with the server */
namespace NetworkServ
{
#include "../Shared/networkcommands.h"
}

class TeamInfo;

/***
  WARNING! Always use deleteLater on this!

  Otherwise you may delete it when Network::onReceipt() still
  does recurive calls. Crash!
***/
class Analyzer : public QObject
{
    Q_OBJECT
public:
    template<class SocketClass>
    Analyzer(const SocketClass &sock, int id);
    ~Analyzer();

    /* functions called by the server */
    void sendMessage(const QString &message);
    void sendChannelMessage(int chanid, const QString &message);
    void sendHtmlMessage(const QString &message);
    void sendHtmlChannelMessage(int chanid, const QString &message);
    void requestLogIn();
    void sendPlayer(const PlayerInfo &p);
    void sendPlayers(const QList<PlayerInfo> &p);
    void sendBattleList(int chanid, const QHash<qint32, Battle> &battles);
    void sendChannelPlayers(int channelid, const QVector<qint32> &ids);
    void sendJoin(int channelid, int playerid);
    void sendChannelBattle(int chanid, int battleid, const Battle &battle);
    void sendLogin(const PlayerInfo &p);
    void sendLogout(int num);
    bool isConnected() const;
    QString ip() const;
    void sendChallengeStuff(const ChallengeInfo &c);
    void engageBattle(int battleid, int myid, int id, const TeamBattle &team, const BattleConfiguration &conf);
    void sendBattleResult(qint32 battleid, quint8 res, int win, int los);
    void sendBattleCommand(qint32 battleid, const QByteArray &command);
    void sendWatchingCommand(qint32 id, const QByteArray &command);
    void sendTeamChange(const PlayerInfo &p);
    void sendPM(int dest, const QString &mess);
    void sendUserInfo(const UserInfo &ui);
    void notifyBattle(qint32 battleid, qint32 id1, qint32 id2);
    void finishSpectating(qint32 battleId);
    void notifyAway(qint32 id, bool away);
    void startRankings(int page, int startingRank, int total);
    void sendRanking(const QString name, int points);
    void stopReceiving();
    void connectTo(const QString &host, quint16 port);
    void setLowDelay(bool lowDelay);

    /* Closes the connection */
    void close();

    void delay();

    /* Convenience functions to avoid writing a new one every time */
    void notify(int command);
    template<class T>
    void notify(int command, const T& param);
    template<class T1, class T2>
    void notify(int command, const T1& param1, const T2& param2);
    template<class T1, class T2, class T3>
    void notify(int command, const T1& param1, const T2& param2, const T3 &param3);
    template<class T1, class T2, class T3, class T4>
    void notify(int command, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4);
    template<class T1, class T2, class T3, class T4, class T5>
    void notify(int command, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4,const T5 &param5);
    template<class T1, class T2, class T3, class T4, class T5, class T6>
    void notify(int command, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4,const T5 &param5, const T6 &param6);
    template<class T>
    void notify_expand(int command, const T &paramList);
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(TeamInfo &team, bool ladder, bool showteam, QColor c);
    void messageReceived(int chanid, const QString &mess);
    void teamReceived(TeamInfo &team);
    void connected();
    void disconnected();
    void forfeitBattle(int id);
    void challengeStuff(const ChallengeInfo &c);
    void battleMessage(int id, const BattleChoice &choice);
    void battleChat(int id, const QString &chat);
    void battleSpectateRequested(int id);
    void battleSpectateEnded(int id);
    void battleSpectateChat(int id, const QString &chat);
    void wannaRegister();
    void sentHash(QString);
    void kick(int id);
    void ban(int id);
    void banRequested(const QString &name);
    void unbanRequested(const QString &name);
    void tempBan(int id, int time);
    void tempBanRequested(const QString &name, int time);
    void tunbanRequested(const QString &name);
    void PMsent(int id, const QString);
    void getUserInfo(const QString &name);
    void banListRequested();
    void tbanListRequested();
    /* Registry socket signals */
    void ipRefused();
    void nameTaken();
    void invalidName();
    void accepted();
    void awayChange(bool away);
    void showTeamChange(bool);
    void ladderChange(bool);
    void tierChanged(const QString &);
    void findBattle(const FindBattleData &f);
    void showRankings(const QString &tier, const QString &name);
    void showRankings(const QString &tier, int page);
    void joinRequested(const QString &channel);
    void leaveChannel(int id);
    void ipChangeRequested(const QString &ip);
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);
    void undelay();
    void keepAlive();
private:
    GenericNetwork &socket();
    const GenericNetwork &socket() const;

    void dealWithCommand(const QByteArray &command);

    QLinkedList<QByteArray> delayedCommands;
    int delayCount;

    GenericNetwork *mysocket;
    QMutex mutex;
    bool pingedBack;
};

template<class SocketClass>
Analyzer::Analyzer(const SocketClass &sock, int id) : mysocket(new Network<SocketClass>(sock, id)), pingedBack(true)
{
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));

    socket().setParent(this);

    QTimer *t = new QTimer(this);
    t->setInterval(30*1000);
    t->start();
    connect(t, SIGNAL(timeout()),SLOT(keepAlive()));
    /* Only if its not registry */
    if (id != 0) {
#ifndef SFML_SOCKETS
        sock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
#endif
    }

    delayCount = 0;
}

template<class T>
void Analyzer::notify(int command, const T& param)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param;

    emit sendCommand(tosend);
}

template<class T>
void Analyzer::notify_expand(int command, const T& paramList)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    typename T::const_iterator it = paramList.begin();

    while (it != paramList.end()) {
        out << *it;
        ++it;
    }

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

template<class T1, class T2, class T3, class T4>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3, const T4 &param4)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3 << param4;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3, class T4,class T5>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3, const T4 &param4, const T5 &param5)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3 << param4 << param5;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3, class T4,class T5, class T6>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3, const T4 &param4, const T5 &param5, const T6 &param6)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3 << param4 << param5 << param6;

    emit sendCommand(tosend);
}

#endif // ANALYZE_H

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include <QColor>
#include "network.h"

class TeamBattle;
class BattleChoice;
class BattleConfiguration;
class ChallengeInfo;
class UserInfo;
class PlayerInfo;
class FindBattleData;

/* Commands to dialog with the server */
namespace NetworkServ
{
    enum Command
    {
	WhatAreYou = 0,
	WhoAreYou,
	Login,
	Logout,
	SendMessage,
	PlayersList,
	SendTeam,
	ChallengeStuff,
	EngageBattle,
	BattleFinished,
        BattleMessage = 10,
	BattleChat,
        KeepAlive,
        AskForPass,
        Register,
        PlayerKick,
        PlayerBan,
        ServNumChange,
        ServDescChange,
        ServNameChange,
        SendPM,
        Away,
        GetUserInfo,
        GetUserAlias,
        GetBanList,
        CPBan,
        CPUnban,
        SpectateBattle,
        SpectatingBattleMessage,
        SpectatingBattleChat,
        SpectatingBattleFinished,
        LadderChange,
        ShowTeamChange,
        VersionControl,
        TierSelection,
        ServMaxChange,
        FindBattle
    };

    enum ProtocolError
    {
	UnknownCommand = 0
    };
}

class TeamInfo;

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(QTcpSocket *sock, int id);
    ~Analyzer();

    /* functions called by the server */
    void sendMessage(const QString &message);
    void requestLogIn();
    void sendPlayer(const PlayerInfo &p);
    void sendLogin(const PlayerInfo &p);
    void sendLogout(int num);
    bool isConnected() const;
    QString ip() const;
    void sendChallengeStuff(const ChallengeInfo &c);
    void engageBattle(int myid, int id, const TeamBattle &team, const BattleConfiguration &conf);
    void sendBattleResult(quint8 res, int win, int los);
    void sendBattleCommand(const QByteArray &command);
    void sendWatchingCommand(qint32 id, const QByteArray &command);
    void sendTeamChange(const PlayerInfo &p);
    void sendPM(int dest, const QString &mess);
    void sendUserInfo(const UserInfo &ui);
    void notifyBattle(qint32 id1, qint32 id2);
    void finishSpectating(qint32 battleId);
    void notifyAway(qint32 id, bool away);
    void stopRecieving();
    void connectTo(const QString &host, quint16 port);

    /* Closes the connection */
    void close();

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
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(const TeamInfo &team, bool ladder, bool showteam, QColor c);
    void messageReceived(const QString &mess);
    void teamReceived(const TeamInfo &team);
    void connected();
    void disconnected();
    void forfeitBattle();
    void challengeStuff(const ChallengeInfo &c);
    void battleMessage(const BattleChoice &choice);
    void battleChat(const QString &chat);
    void battleSpectateRequested(int id);
    void battleSpectateEnded(int id);
    void battleSpectateChat(int id, const QString &chat);
    void wannaRegister();
    void sentHash(QString);
    void kick(int id);
    void ban(int id);
    void banRequested(const QString &name);
    void unbanRequested(const QString &name);
    void PMsent(int id, const QString);
    void getUserInfo(const QString &name);
    void banListRequested();
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
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);
    void keepAlive();
private:
    Network &socket();
    const Network &socket() const;

    Network mysocket;
    QTimer *mytimer;
};

template<class T>
void Analyzer::notify(int command, const T& param)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param;

    emit sendCommand(tosend);
}

template<class T1, class T2>
void Analyzer::notify(int command, const T1& param1, const T2 &param2)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3, class T4>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3, const T4 &param4)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3 << param4;

    emit sendCommand(tosend);
}

template<class T1, class T2, class T3, class T4,class T5>
void Analyzer::notify(int command, const T1& param1, const T2 &param2, const T3 &param3, const T4 &param4, const T5 &param5)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param1 << param2 << param3 << param4 << param5;

    emit sendCommand(tosend);
}

#endif // ANALYZE_H

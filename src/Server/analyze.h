#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class BasicInfo;
class TeamBattle;
class BattleChoice;
class BattleConfiguration;

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
	BattleMessage,
	BattleChat,
        KeepAlive,
        AskForPass,
        Register,
        PlayerKick,
        PlayerBan
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

    /* functions called by the server */
    void sendMessage(const QString &message);
    void requestLogIn();
    void sendPlayer(int num, const BasicInfo &team, int auth=0);
    void sendLogin(int num, const BasicInfo &team, int auth=0);
    void sendLogout(int num);
    bool isConnected() const;
    QString ip() const;
    void sendChallengeStuff(quint8 desc, int id);
    void engageBattle(int id, const TeamBattle &team, const BattleConfiguration &conf);
    void sendBattleResult(quint8 res);
    void sendBattleCommand(const QByteArray &command);

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
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(const TeamInfo &team);
    void messageReceived(const QString &mess);
    void teamReceived(const TeamInfo &team);
    void disconnected();
    void forfeitBattle();
    void challengeStuff(int desc, int id);
    void battleMessage(const BattleChoice &choice);
    void battleChat(const QString &chat);
    void wannaRegister();
    void sentHash(QString);
    void kick(int id);
    void ban(int id);
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
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << param;

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

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class BasicInfo;
class TeamBattle;

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
	BattleFinished
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
    Analyzer(QTcpSocket *sock);

    /* functions called by the server */
    void sendMessage(const QString &message);
    void requestLogIn();
    void sendPlayer(int num, const BasicInfo &team);
    void sendLogin(int num, const BasicInfo &team);
    void sendLogout(int num);
    bool isConnected() const;
    void sendChallengeStuff(quint8 desc, int id);
    void engageBattle(int id, const TeamBattle &team);
    void sendBattleResult(quint8 res);
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
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

private:
    Network &socket();
    const Network &socket() const;

    Network mysocket;
};

#endif // ANALYZE_H

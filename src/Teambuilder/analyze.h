#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class Client;
class TrainerTeam;
class Player;
class BattleChoice;
class TeamBattle;

/* Commands to dialog with the server */
namespace NetworkCli
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
	BattleMessage
    };

    enum ProtocolError
    {
	UnknownCommand = 0
    };
}

/* Analyzes the messages received from the network and emits the corresponding signals.
   Also allows you to send your own messages to the network

    The client actually uses this widgets to send orders and receive commands from the outside. */

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer();

    /* functions called by the client */
    void login(const TrainerTeam &team);
    void sendMessage(const QString &message);
    void connectTo(const QString &host, quint16 port);
    void sendTeam(const TrainerTeam & team);
    void sendChallengeStuff(quint8 desc, int id);
    void sendBattleResult(int result);
    bool isConnected() const;

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
    void playerReceived(const Player &p);
    /* login of a player */
    void playerLogin(const Player &p);
    void playerLogout(int id);
    /* challengerelated */
    void challengeStuff(int desc, int id);
    void battleStarted(int id, const TeamBattle &myteam);
    void battleFinished(int res);
    void battleMessage(const QByteArray &mess);

public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

    /* by the battle window */
    void battleCommand(const BattleChoice &comm);

private:
    /* The connection to the outside */
    Network &socket();

    Network mysocket;
};

#endif // ANALYZE_H

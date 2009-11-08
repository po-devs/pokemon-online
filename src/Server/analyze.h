#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

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
	SendTeam
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
    void sendPlayer(int num, const TeamInfo &team);
    void sendLogin(int num, const TeamInfo &team);
    void sendLogout(int num);
    bool isConnected() const;

signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);
    void loggedIn(const QString &mess);
    void messageReceived(const QString &mess);
    void teamReceived(const TeamInfo &team);
    void disconnected();
public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

private:
    Network &socket();

    Network mysocket;
};

#endif // ANALYZE_H

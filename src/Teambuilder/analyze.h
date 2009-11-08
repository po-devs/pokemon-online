#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class Client;
class TrainerTeam;
class Player;

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
	SendTeam
    };

    enum ProtocolError
    {
	UnknownCommand = 0
    };
}

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer();

    /* functions called by the client */
    void login(const QString &name, const QString &pass);
    void sendMessage(const QString &message);
    void connectTo(const QString &host, quint16 port);
    void sendTeam(const TrainerTeam & team);
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
    void playerReceived(const Player &p);

public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

private:
    Network &socket();

    Network mysocket;
};

#endif // ANALYZE_H

#ifndef ANALYZE_H
#define ANALYZE_H

#include <QtCore>
#include "network.h"

class Client;

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
	RecvMessage,
	PlayersList
    };

    enum ProtocolError
    {
	UnknowCommand = 0
    };
}

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(Client *client);

    /* functions called by the client */
    void login(const QString &name, const QString &pass);
    void sendMessage(const QString &message);
    void connectTo(const QString &host, quint16 port);

signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);

public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

private:
    bool isValid() const;
    Network &socket();

    Network mysocket;
    Client *myClient;
};

#endif // ANALYZE_H

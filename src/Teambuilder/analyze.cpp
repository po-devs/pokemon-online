#include "analyze.h"
#include "network.h"
#include "client.h"

using namespace NetworkCli;

Analyzer::Analyzer(Client *client) : myClient(client)
{
}

void Analyzer::login(const QString &name, const QString &pass)
{
    QByteArray tosend;
    QDataStream in(&tosend, QIODevice::WriteOnly);

    in << uchar(Login) << name << pass;

    emit sendCommand(tosend);
}

void Analyzer::sendMessage(const QString &message)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);

    out << uchar(SendMessage) << message;

    emit sendCommand(tosend);
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    mysocket.connectToHost(host, port);
}

void Analyzer::error()
{
}

void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    uchar command;

    in >> command;

    switch (command) {
	default:
	    emit protocolError(UnknowCommand, tr("Protocol error: unknown command received"));
    }
}

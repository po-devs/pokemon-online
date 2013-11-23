#include "registrycommunicator.h"

#include "analyze.h"
#include "server.h"

RegistryCommunicator::RegistryCommunicator(QObject *parent) :
    QObject(parent), registry_connection(nullptr), serverPrivate(true)
{
    Server *server= Server::serverIns;

    connect(this, SIGNAL(info(QString)), server, SLOT(printLine(QString)));
}

void RegistryCommunicator::connectToRegistry()
{
    if (registry_connection) {
        if (registry_connection->isConnected()) {
            return;
        }
        else
            registry_connection->deleteLater();
    }

    registry_connection = nullptr;

    if (serverPrivate)
        return;

    emit info("Connecting to registry...");

    QTcpSocket * s = new QTcpSocket(nullptr);
    s->connectToHost("registry.pokemon-online.eu", 8081);

    connect(s, SIGNAL(connected()), this, SLOT(regConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(regConnectionError()));

    registry_connection = new Analyzer(s,0);
}

void RegistryCommunicator::disconnectFromRegistry()
{
    registry_connection->deleteLater();
    registry_connection = nullptr;
    emit info("Disconnected from registry.");
}


void RegistryCommunicator::setPrivate(bool priv)
{
    if (serverPrivate == priv)
        return;

    serverPrivate = priv;

    if (serverPrivate)
    {
        emit info("The server is now private.");
        disconnectFromRegistry();
    }
    else
    {
        emit info("The server is now public.");
        connectToRegistry();
    }
}

void RegistryCommunicator::regConnectionError()
{
    QTimer::singleShot(30000, this, SLOT(connectToRegistry()));
    emit info("Error when connecting to the registry. Will restart in 30 seconds");
}

void RegistryCommunicator::regConnected()
{
    emit info("Connected to registry! Sending server info...");
    Server *server= Server::serverIns;

    registry_connection->notify(NetworkServ::Login, server->serverName, server->serverDesc, quint16(AntiDos::obj()->numberOfDiffIps()), server->serverPlayerMax, server->serverPorts.at(0));
    connect(registry_connection, SIGNAL(ipRefused()), SLOT(ipRefused()));
    connect(registry_connection, SIGNAL(invalidName()), SLOT(invalidName()));
    connect(registry_connection, SIGNAL(nameTaken()), SLOT(nameTaken()));
    connect(registry_connection, SIGNAL(accepted()), SLOT(accepted()));
    /* Sending Players at regular interval */
    QTimer::singleShot(2500, this, SLOT(regSendPlayers()));
}

void RegistryCommunicator::regSendPlayers()
{
    if (!testConnection())
        return;

    registry_connection->notify(NetworkServ::ServNumChange, quint16(AntiDos::obj()->numberOfDiffIps()));
    /* Sending Players at regular interval */
    QTimer::singleShot(2500, this, SLOT(regSendPlayers()));
}

void RegistryCommunicator::nameChange(const QString &name)
{
    if (!testConnection())
        return;

    registry_connection->notify(NetworkServ::ServNameChange, name);
}

void RegistryCommunicator::descChange(const QString &desc)
{
    if (!testConnection())
        return;

    registry_connection->notify(NetworkServ::ServDescChange, desc);
}

void RegistryCommunicator::maxChange(int numMax)
{
    if (!testConnection())
        return;

    registry_connection->notify(NetworkServ::ServMaxChange,numMax);
}

void RegistryCommunicator::passChanged(bool enabled) {
    if (!testConnection())
        return;

    registry_connection->notify(NetworkServ::ServerPass, enabled);
}

void RegistryCommunicator::accepted()
{
    emit info("The registry acknowledged the server.");
}

void RegistryCommunicator::invalidName()
{
    emit info("Invalid name for the registry. Please change it in Options -> Config.");
}

void RegistryCommunicator::nameTaken()
{
    emit info("The name of the server is already in use. Please change it in Options -> Config.");
}

void RegistryCommunicator::ipRefused()
{
    emit info("Registry wants only 1 server per IP");
}

bool RegistryCommunicator::testConnection()
{
    return registry_connection && registry_connection->isConnected();
}

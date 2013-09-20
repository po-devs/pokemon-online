namespace Nw {
#include "../Shared/networkcommands.h"
}

#include "registrystation.h"

RegistryStation::RegistryStation()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(resetRegistryConnection()));

    connect (&network, SIGNAL(isFull(QByteArray)), SLOT(readCommand(QByteArray)));

    timer->start(10000);
}

void RegistryStation::resetRegistryConnection()
{
    network.close();

    servers.clear();

    network.connectToHost("registry.pokemon-onlnine.eu", 8080);
}

void RegistryStation::readCommand(const QByteArray &commandline)
{
    DataStream in (commandline, version.version);
    uchar command;

    in >> command;

    switch (command) {
    case Nw::PlayersList: {
        // Registry socket;
        ServerInfo s;
        in >> s;

        servers.push_back(s);
        break;
    }
    case Nw::ServerListEnd: {
        savedServers = servers;
        network.close();
        break;
    }
    }
}

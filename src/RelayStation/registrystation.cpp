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

    network.connectToHost("registry.pokemon-online.eu", 8080);
}

void RegistryStation::readCommand(const QByteArray &commandline)
{
    DataStream in (commandline, version.version);
    uchar command;

    in >> command;

    switch (command) {
    case Nw::PlayersList: {
        ServerInfo s;
        in >> s;

        servers.push_back(s);
        break;
    }
    case Nw::ServerListEnd: {
        qDebug() << "registry list updated";
        saveServers();
        network.close();
        break;
    }
    }
}

void RegistryStation::saveServers()
{
    QVariantList sservers;

    foreach(ServerInfo s, servers) {
        QVariantMap server;
        server.insert("name", s.name);
        server.insert("ip", s.ip);
        if (s.max != 0) {
            server.insert("max", s.max);
        }
        server.insert("description", s.desc);
        server.insert("num", s.num);
        server.insert("locked", s.passwordProtected);
        server.insert("port", s.port);
        sservers.push_back(server);
    }

    savedServers = "servers|"+jserial.serialize(sservers);
}

#ifndef REGISTRYSTATION_H
#define REGISTRYSTATION_H

#include <QObject>
#include "../PokemonInfo/networkstructs.h"
#include "../Teambuilder/network.h"

class Network;

class RegistryStation : public QObject
{
    Q_OBJECT
public:
    RegistryStation();
public slots:
    void resetRegistryConnection();
    void readCommand(const QByteArray &array);
private:
    Network network;

    QList<ServerInfo> servers;
    QList<ServerInfo> savedServers;
    ProtocolVersion version;
};

#endif // REGISTRYSTATION_H

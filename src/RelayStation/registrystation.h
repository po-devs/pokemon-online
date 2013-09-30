#ifndef REGISTRYSTATION_H
#define REGISTRYSTATION_H

#include <QObject>
#include "../PokemonInfo/networkstructs.h"
#include "../Teambuilder/network.h"
#include "../QJson/qjson.h"

class Network;

class RegistryStation : public QObject
{
    Q_OBJECT
public:
    RegistryStation();

    QString getServers() {
        return savedServers;
    }

public slots:
    void resetRegistryConnection();
    void readCommand(const QByteArray &array);
private:
    Network network;

    void saveServers();

    QList<ServerInfo> servers;
    QString savedServers;
    ProtocolVersion version;

    QJson::Serializer jserial;
};

#endif // REGISTRYSTATION_H

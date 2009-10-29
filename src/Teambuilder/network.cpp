#include "network.h"

void Network::connectTo(int id, const QString &ip, quint16 port)
{
    (void) id, (void) ip, (void) port;
}

void Network::sendData(int socketId, const QByteArray &Data)
{
    (void) socketId, (void) Data;
}

void Network::close(int socketId)
{
    (void) socketId;
}

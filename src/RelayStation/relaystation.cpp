#include "../QtWebsocket/QWsServer.h"

#include "dualwielder.h"
#include "relaystation.h"
#include "registrystation.h"

RelayStation::RelayStation(int port, QString host, QHash<QString, QString> aliases, QObject *parent) :
    QObject(parent), port(port), host(host), _aliases(aliases)
{
    webserver = new QWsServer(this);
    registry = new RegistryStation();
    registry->setParent(this);
}

void RelayStation::start()
{
    qDebug() << "Starting to listen to port " << port;

    if (!webserver->listen(QHostAddress::Any, port)) {
        qDebug() << "Failure to listen to port " << port;
        return;
    }

    connect(webserver, SIGNAL(newConnection()), SLOT(onNewConnection()));
}

void RelayStation::onNewConnection()
{
    QWsSocket *socket = webserver->nextPendingConnection();

    if (!socket) {
        return;
    }

    DualWielder *d = new DualWielder(this);
    d->init(socket, host, _aliases);
}

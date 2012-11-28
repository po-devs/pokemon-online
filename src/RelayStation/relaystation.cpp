#include "../QtWebsocket/QWsServer.h"

#include "dualwielder.h"
#include "relaystation.h"

RelayStation::RelayStation(int port, QString host, QObject *parent) :
    QObject(parent), port(port), host(host)
{
    webserver = new QWsServer(this);
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
    d->init(socket, host);
}

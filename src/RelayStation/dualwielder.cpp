#include "../QtWebsocket/QWsSocket.h"
#include "../Teambuilder/network.h"
#include "../Shared/networkcommands.h"

#include "dualwielder.h"

DualWielder::DualWielder(QObject *parent) : QObject(parent), web(NULL), network(NULL)
{
}

DualWielder::~DualWielder()
{
    if (network) {
        network->close();
    }
    if (web) {
        web->close(QWsSocket::CloseGoingAway);
    }

    qDebug() << "Closing connection with IP " << ip();
}

void DualWielder::init(QWsSocket *sock, QString host)
{
    web = sock;
    mIp = web->ip();

    qDebug() << "Connection accepted, IP " + ip();

    sock->write("defaultserver|"+ host);

    connect(sock, SIGNAL(frameReceived(QString)), SLOT(readWebSocket(QString)));
    connect(sock, SIGNAL(disconnected()), SLOT(webSocketDisconnected()));
    connect(sock, SIGNAL(disconnected()), sock, SLOT(deleteLater()));
}

QString DualWielder::ip() const
{
    return mIp;
}

void DualWielder::readWebSocket(const QString &frame)
{
    /* Shouldn't happen, but idk how websockets work */
    if (!web) {
        return;
    }

    QString command = frame.section("|",0,0);
    QString data = frame.section("|", 1);

    if (!network) {
        if (command == "connect") {
            qDebug() << "Connecting websocket to server at " << data;
            network = new Network(data.section(":", 0, -2), data.section(":", -1).toInt());

            connect(network, SIGNAL(connected()), SLOT(socketConnected()));
            connect(network, SIGNAL(disconnected()), SLOT(socketDisconnected()));
            connect(network, SIGNAL(disconnected()), network, SLOT(deleteLater()));
            connect(network, SIGNAL(error(QAbstractSocket::SocketError)), network, SLOT(deleteLater()));
        } else {
            web->write(QString("error|You need to choose a server to connect to."));
        }
    } else {

    }
}

void DualWielder::socketConnected()
{
    if (web) {
        web->write(QString("connected|"));
    }
}

void DualWielder::socketDisconnected()
{
    network = NULL;
    if (web) {
        web->write(QString("disconnected|"));
        web->close(QWsSocket::CloseNormal, "The Pokemon Online server closed the connection");
        web = NULL;
    }

    deleteLater();
}

void DualWielder::webSocketDisconnected()
{
    web = NULL;
    if (network) {
        /* Gives the server the curtesy to know that there will be no reconnection */
        network->send(QByteArray(1, char(Logout)));
        network->close();
        network = NULL;
    }

    deleteLater();
}

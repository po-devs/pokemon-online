#include "server.h"
#include "sfmlsocket.h"

Server::Server(GenericSocket *serv) : myserver(serv)
{
    connect(serv, SIGNAL(active()), SLOT(received()));
}


void Server::received()
{
    GenericSocket *connection = myserver->nextPendingConnection();

    if (!connection)
        return;

    connect(connection, SIGNAL(disconnected()), connection, SLOT(deleteLater()));
    connect(connection, SIGNAL(disconnected()), qApp, SLOT(quit()));

    qDebug() << "Received socket, ip: " << connection->ip();
}

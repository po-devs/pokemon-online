#include "server.h"

Player::Player(QTcpSocket *sock) : myrelay(this, sock)
{
    connect(&relay(), SIGNAL(loggedIn(QString)), this, SLOT(loggedIn(QString)));
    connect(&relay(), SIGNAL(messageReceived(QString)), this, SLOT(recvMessage(QString)));
}

void Player::recvMessage(const QString &mess)
{
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(mess);
}

void Player::loggedIn(const QString &name)
{
    team().setTrainerNick(name);

    emit loggedIn(name);
}

Server::Server(quint16 port)
{
    server().listen(QHostAddress::Any, port);
}

void Server::incomingConnection()
{
}

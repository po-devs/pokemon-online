#include "network.h"

template <>
Network<GenericSocket>::Network(GenericSocket sock, int id) : mysocket(sock), commandStarted(false), myid(id), stillValid(true)
{
#ifndef SFML_SOCKETS
    connect(socket(), SIGNAL(readyRead()), this, SLOT(onReceipt()));
#else
    connect(&*socket(), SIGNAL(active()), this, SLOT(onReceipt()));
#endif
    connect(&*socket(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(&*socket(), SIGNAL(disconnected()), this, SIGNAL(disconnected()));
#ifndef SFML_SOCKETS
    connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(manageError(QAbstractSocket::SocketError)));
#endif
    /* SO THE SOCKET IS SAFELY DELETED LATER WHEN DISCONNECTED! */
    connect(this, SIGNAL(destroyed()), &*socket(), SLOT(deleteLater()));
#ifndef SFML_SOCKETS
    _ip = socket()->peerAddress().toString();
#else
    _ip = socket()->ip();
#endif
}

/* Networks with QTcpSockets are even used when GenericSocket = boost::asio::ip::tcp::socket.
   So we need a constructor not based off GenericSockets */
#ifdef SFML_SOCKETS
template <>
Network<QTcpSocket*>::Network(QTcpSocket *sock, int id) : mysocket(sock), commandStarted(false), myid(id), stillValid(true)
{
    connect(socket(), SIGNAL(readyRead()), this, SLOT(onReceipt()));
    connect(socket(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(socket(), SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(manageError(QAbstractSocket::SocketError)));
    /* SO THE SOCKET IS SAFELY DELETED LATER WHEN DISCONNECTED! */
    connect(this, SIGNAL(destroyed()), socket(), SLOT(deleteLater()));
    _ip = socket()->peerAddress().toString();
}
#endif

#ifdef SFML_SOCKETS
template <>
bool Network<QTcpSocket*>::isConnected() const
{
    if (socket()) {
        return socket()->state() != QAbstractSocket::UnconnectedState;
    }
    else
        return false;
}

#endif

template <>
bool Network<GenericSocket>::isConnected() const
{
    if (socket()) {
#ifndef SFML_SOCKETS
        return socket()->state() != QAbstractSocket::UnconnectedState;
#else
        return socket()->sock().is_open();
#endif
    }
    else
        return false;
}

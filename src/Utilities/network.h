#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>
#include "asiosocket.h"

class GenericNetwork: public QObject
{
    Q_OBJECT
public:
    /* Functions to reimplement:
           isValid: returns whether the socket is valid or not!
           close: closes the socket after writing the pending data
           abort: abruptly closes the socket */
    virtual int error() const = 0;
    virtual QString errorString() const = 0;
    virtual void changeIP(const QString &newIp) = 0;
    virtual QString ip() const = 0;
    virtual bool isConnected() const = 0;
    /* Trades bandwith for latency */
    virtual void setLowDelay(bool lowDelay) = 0;

    virtual void connectToHost(const QString & ip, quint16 port) = 0;
    virtual void disconnectFromHost() = 0;

    virtual void close() = 0;
    virtual int id() const = 0;
    virtual void changeId(int newId) = 0;
signals:
    void isFull(QByteArray command);
    void connected();
    void disconnected();
    void _error();
public slots:
    virtual void onReceipt(){}
    virtual void onDisconnect(){}
    virtual void manageError(QAbstractSocket::SocketError){}
    virtual void send(const QByteArray &message){(void) message;}
    virtual void sendPacket(const QByteArray&){}
    virtual void onSocketConnected(){}
};

template <class S>
        class Network : public GenericNetwork
{
public:
    Network(S sock, int id=0);
    ~Network();
    /* Functions to reimplement:
           isValid: returns whether the socket is valid or not!
           close: closes the socket after writing the pending data
           abort: abruptly closes the socket */
    int error() const;
    QString errorString() const;
    void changeIP(const QString &newIp);
    QString ip() const;
    bool isConnected() const;
    /* Trades bandwith for latency */
    void setLowDelay(bool lowDelay);

    void connectToHost(const QString & ip, quint16 port);
    void disconnectFromHost();

    void close();
    int id() const {return myid;}
    void changeId(int newId) {myid = newId;}

    virtual void onReceipt();
    virtual void onDisconnect();
    virtual void onSocketConnected();
    virtual void manageError(QAbstractSocket::SocketError);
    virtual void send(const QByteArray &message);
    virtual void sendPacket(const QByteArray&);
private:
    void makeSocketConnections();
    void attributeIp();

    /* internal socket */
    S mysocket;
    /* internal variables for the protocol */
    bool commandStarted;
    quint32 remainingLength;
    /* errors stored when disconnected */
    int myerror;
    QString myerrorString;
    /* To keep track of things */
    int myid;
    /* Important.
       There were some crashes that drived me mad. If the server removes a socket, and theres still
       data pending, the socket would crash (but it took me time to discover that).
       So here it is! The thing to stop reading bullshit data. */
    bool stillValid;

    QString _ip;

    S socket();
    const S socket() const;
};

#include "antidos.h"

template <class S>
Network<S>::Network(S sock, int id) : mysocket(sock), commandStarted(false), myid(id), stillValid(true)
{
    makeSocketConnections();
}

template <class S>
void Network<S>::attributeIp()
{
    _ip = socket()->ip();
}

template <>
inline void Network<QTcpSocket*>::attributeIp()
{
    _ip = socket()->peerAddress().toString();
}

/* For Boost Sockets */
template <class S>
void Network<S>::makeSocketConnections()
{
    connect(&*socket(), SIGNAL(active()), this, SLOT(onReceipt()));
    connect(&*socket(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(&*socket(), SIGNAL(disconnected()), &*socket(), SLOT(deleteLater()));

    /* SO THE SOCKET IS SAFELY DELETED LATER WHEN DISCONNECTED! */
    connect(this, SIGNAL(destroyed()), &*socket(), SLOT(deleteLater()));
    attributeIp();
}

/* For QTcpSockets */
template <>
inline void Network<QTcpSocket*>::makeSocketConnections()
{
    connect(socket(), SIGNAL(readyRead()), this, SLOT(onReceipt()));
    connect(socket(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(socket(), SIGNAL(disconnected()), socket(), SLOT(deleteLater()));

    connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(manageError(QAbstractSocket::SocketError)));
    socket()->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    /* SO THE SOCKET IS SAFELY DELETED LATER WHEN DISCONNECTED! */
    connect(this, SIGNAL(destroyed()), socket(), SLOT(deleteLater()));
    attributeIp();
}

template <class S>
bool Network<S>::isConnected() const
{
    if (socket()) {
        return socket()->sock().is_open();
    }

    return false;
}

template <>
inline bool Network<QTcpSocket*>::isConnected() const
{
    if (socket()) {
        return socket()->state() != QAbstractSocket::UnconnectedState;
    }

    return false;
}

template <class S>
void Network<S>::manageError(QAbstractSocket::SocketError err)
{
    myerror = err;
}

template <class S>
void Network<S>::close() {
    //qDebug() << "beginning to close a network " << this;
    stillValid = false;
    if (socket()) {
        //qDebug() << "valid socket " << this;
        S sock = mysocket;

        mysocket = S(0);

        sock->disconnect(this);
        sock->deleteLater();
        sock->disconnectFromHost();

        emit disconnected();
    } else {
        //qDebug() << "invalid socket";
    }

    //qDebug() << "End of closing network";
}

template <class S>
void Network<S>::setLowDelay(bool lowDelay)
{
    (void) lowDelay;
}

template <>
inline void Network<QTcpSocket*>::setLowDelay(bool lowDelay)
{
    socket()->setSocketOption(QAbstractSocket::LowDelayOption, lowDelay);
}

template <class S>
Network<S>::~Network()
{
    close();
}

template <class S>
void Network<S>::connectToHost(const QString &ip, quint16 port)
{
    (void) ip;
    (void) port;
}

template <>
inline void Network<QTcpSocket*>::connectToHost(const QString &ip, quint16 port)
{
    if (!socket()) {
        mysocket = new QTcpSocket();
        makeSocketConnections();
    }
    connect(socket(), SIGNAL(connected()), SLOT(onSocketConnected()));

    socket()->connectToHost(ip, port);
}

template <class S>
void Network<S>::disconnectFromHost()
{
}

template <>
inline void Network<QTcpSocket*>::disconnectFromHost()
{
    if (socket()) {
        socket()->disconnectFromHost();
    }
}

template <class S>
QString Network<S>::ip() const {
    return _ip;
}

template <class S>
void Network<S>::changeIP(const QString &ip) {
    _ip = ip;
}

template <class S>
void Network<S>::onDisconnect()
{
    stillValid = false;
    if (socket()) {
        S sock = mysocket;
        mysocket = S(0);

        sock->disconnect(this);
        sock->deleteLater();

        emit disconnected();
    }
}

template <class S>
void Network<S>::onSocketConnected()
{
    attributeIp();
    emit connected();
}

template <class S>
void Network<S>::onReceipt()
{
    if (stillValid && socket())
    {
        if (commandStarted == false) {
            /* There it's a new message we are receiving.
               To start receiving it we must know its length, i.e. the 4 first bytes */
            if (socket()->bytesAvailable() < 4) {
                return;
            }
            /* Ok now we can start */
            commandStarted=true;
            /* getting the length of the message */
            char c1, c2, c3, c4;
            socket()->getChar(&c1), socket()->getChar(&c2); socket()->getChar(&c3), socket()->getChar(&c4);
            remainingLength= (uchar(c1) << 24) + (uchar(c2) << 16) + (uchar(c3) << 8) + uchar(c4);

            /* Just a little check :p */
            if (AntiDos::obj() &&  myid > 0 && !AntiDos::obj()->transferBegin(myid, remainingLength, ip())) {
                return;
            }

            /* Recursive call to write less code =) */
            onReceipt();
        } else {
            /* Checking if the command is complete! */
            if (socket()->bytesAvailable() >= remainingLength) {
                emit isFull(socket()->read(remainingLength));
                commandStarted = false;
                /* Recursive call to spare code =), there may be still data pending */
                onReceipt();
            }
        }
    }
}

template <class S>
int Network<S>::error() const
{
    return myerror;
}

template <>
inline int Network<QTcpSocket*>::error() const
{
    if (socket()) {
        return socket()->error();
    } else {
        return myerror;
    }
}

template <class S>
QString Network<S>::errorString() const
{
    return myerrorString;
}

template <>
inline QString Network<QTcpSocket*>::errorString() const
{
    if (socket()) {
        return socket()->errorString();
    } else {
        return myerrorString;
    }
}

template <class S>
void Network<S>::send(const QByteArray &message)
{
    if (!isConnected())
        return;
    quint32 length = message.length();
    uchar c1, c2, c3, c4;
    c1 = length & 0xFF;
    length >>= 8;
    c2 = length & 0xFF;
    length >>= 8;
    c3 = length & 0xFF;
    length >>= 8;
    c4 = length & 0xFF;

    socket()->putChar(c4);
    socket()->putChar(c3);
    socket()->putChar(c2);
    socket()->putChar(c1);
    socket()->write(message);
}

template <class S>
S Network<S>::socket()
{
    return mysocket;
}

template <class S>
const S Network<S>::socket() const
{
    return mysocket;
}

template <class S>
void Network<S>::sendPacket(const QByteArray &p)
{
    if (socket()) {
        socket()->write(p);
    }
}

typedef Network<QTcpSocket*> StandardNetwork;

#endif // NETWORK_H

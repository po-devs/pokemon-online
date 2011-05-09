#ifndef NETWORK_H
#define NETWORK_H

#include <QtNetwork>
#include "sfmlsocket.h"

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
    virtual QString ip() const = 0;
    virtual bool isConnected() const = 0;
    /* Trades bandwith for latency */
    virtual void setLowDelay(bool lowDelay) = 0;

    virtual void connectToHost(const QString & ip, quint16 port) = 0;

    virtual void close() = 0;
    virtual int id() const = 0;
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
};

template <class S>
        class Network : public GenericNetwork
{
public:
    Network(S sock, int id);
    ~Network();
    /* Functions to reimplement:
           isValid: returns whether the socket is valid or not!
           close: closes the socket after writing the pending data
           abort: abruptly closes the socket */
    int error() const;
    QString errorString() const;
    QString ip() const;
    bool isConnected() const;
    /* Trades bandwith for latency */
    void setLowDelay(bool lowDelay);

    void connectToHost(const QString & ip, quint16 port);

    void close();
    int id() const {return myid;}

    virtual void onReceipt();
    virtual void onDisconnect();
    virtual void manageError(QAbstractSocket::SocketError);
    virtual void send(const QByteArray &message);
private:
    /* internal socket */
    S mysocket;
    /* internal variables for the protocol */
    bool commandStarted;
    quint16 remainingLength;
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
void Network<S>::manageError(QAbstractSocket::SocketError err)
{
    myerror = err;
}

template <class S>
void Network<S>::close() {
    qDebug() << "beginning to close a network " << this;
    stillValid = false;
    if (socket()) {
        qDebug() << "valid socket " << this;
        S sock = mysocket;
        mysocket = S();
        sock->disconnect();
        sock->disconnectFromHost();
        sock->deleteLater();

        emit disconnected();
    } else {
        qDebug() << "invalid socket";
    }

    qDebug() << "End of closing network";
}

template <class S>
void Network<S>::setLowDelay(bool lowDelay)
{
#ifndef SFML_SOCKETS
    if (socket()) {
        socket()->setSocketOption(QAbstractSocket::LowDelayOption, lowDelay);
    }
#else
    (void) lowDelay;
#endif
}

template <class S>
Network<S>::~Network()
{
    close();
}

template <class S>
void Network<S>::connectToHost(const QString &ip, quint16 port)
{
#ifndef SFML_SOCKETS
    socket()->connectToHost(ip, port);
    connect(socket(), SIGNAL(connected()), SIGNAL(connected()));
#else
    (void) ip;
    (void) port;
#endif
}

template <class S>
QString Network<S>::ip() const {
    return _ip;
}

template <class S>
void Network<S>::onDisconnect()
{
    stillValid = false;
    if (socket()) {
        qDebug() << "Beginning onDisconnect " << this;
        mysocket->deleteLater();
        mysocket = S();
        qDebug() << "Ending onDisconnect " << this;
    }
}

template <class S>
void Network<S>::onReceipt()
{
    if (stillValid && socket())
    {
        if (commandStarted == false) {
            /* There it's a new message we are receiving.
               To start receiving it we must know its length, i.e. the 2 first bytes */
            if (socket()->bytesAvailable() < 2) {
                return;
            }
            /* Ok now we can start */
            commandStarted=true;
            /* getting the length of the message */
            char c1, c2;
            socket()->getChar(&c1), socket()->getChar(&c2);
            remainingLength=uchar(c1)*256+uchar(c2);

            /* Just a little check :p */
            if (!AntiDos::obj()->transferBegin(myid, remainingLength, ip())) {
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
#ifndef SFML_SOCKETS
    if (socket())
        return socket()->error();
    else
#endif
        return myerror;
}

template <class S>
QString Network<S>::errorString() const
{
#ifndef SFML_SOCKETS
    if (socket())
        return socket()->errorString();
    else
#endif
        return myerrorString;
}

template <class S>
void Network<S>::send(const QByteArray &message)
{
    if (!isConnected())
        return;
    socket()->putChar(message.length()/256);
    socket()->putChar(message.length()%256);
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

#endif // NETWORK_H

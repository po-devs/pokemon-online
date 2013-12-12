#ifndef BASEANALYZER_H
#define BASEANALYZER_H

#include "../Utilities/network.h"
#include "../Utilities/coreclasses.h"
/* for ProtocolVersion */
#include "../PokemonInfo/networkstructs.h"

/***
  WARNING! Always use deleteLater on this!

  Otherwise you may delete it when Network::onReceipt() still
  does recurive calls. Crash!
***/
class BaseAnalyzer : public QObject
{
    friend class RelayManager;

    Q_OBJECT
public:
    template<class SocketClass>
    BaseAnalyzer(const SocketClass &sock, int id, bool dummy=false);
    ~BaseAnalyzer();

    /* functions called by the server */
    bool isConnected() const;
    void changeIP(const QString &ip);
    QString ip() const;
    void stopReceiving();
    void connectTo(const QString &host, quint16 port);
    void setLowDelay(bool lowDelay);
    void sendPacket(const QByteArray &packet);

    /* Closes the connection */
    void close();

    /* Delays all commands to be sent */
    void delay();

    void swapIds(BaseAnalyzer *other);
    void setId(int id);
    void copyFrom(BaseAnalyzer *other);

    /* Convenience functions to avoid writing a new one every time */
    inline void emitCommand(const QByteArray &command) {
        emit sendCommand(command);
    }

    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly, version.version);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emitCommand(tosend);
    }
    template<class T>
    void notify_expand(int command, const T &paramList);
signals:
    /* to send to the network */
    void sendCommand(const QByteArray &command);
    void packetToSend(const QByteArray &packet);
    /* to send to the client */
    void connectionError(int errorNum, const QString &errorDesc);
    void protocolError(int errorNum, const QString &errorDesc);

    void connected();
    void disconnected();

public slots:
    /* slots called by the network */
    void error();
    void commandReceived (const QByteArray &command);

    void undelay();
    void keepAlive(){}//do something
protected:
    GenericNetwork &socket();
    const GenericNetwork &socket() const;

    virtual void dealWithCommand(const QByteArray &command);

    QLinkedList<QByteArray> delayedCommands;
    int delayCount;

    GenericNetwork *mysocket;

    /* Is it a dummy analyzer ?*/
    bool dummy;

    ProtocolVersion version;
};

template<class SocketClass>
BaseAnalyzer::BaseAnalyzer(const SocketClass &sock, int id, bool dummy) : mysocket(new Network<SocketClass>(sock, id)), dummy(dummy)
{
    socket().setParent(this);
    delayCount = 0;

    if (dummy) {
        return;
    }

    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(this, SIGNAL(packetToSend(QByteArray)), &socket(), SLOT(sendPacket(QByteArray)));

    /* Only if its not registry */
#ifndef BOOST_SOCKETS
    sock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
#endif

    QTimer *t = new QTimer(this);
    t->setInterval(30*1000);
    t->start();
    connect(t, SIGNAL(timeout()),SLOT(keepAlive()));
}


template<class T>
void BaseAnalyzer::notify_expand(int command, const T& paramList)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly, version.version);

    out << uchar(command);

    typename T::const_iterator it = paramList.begin();

    while (it != paramList.end()) {
        out << *it;
        ++it;
    }

    emit sendCommand(tosend);
}

#endif // BASEANALYZER_H

#ifndef SFMLSOCKET_H
#define SFMLSOCKET_H

#ifdef SFML_SOCKETS

#include <QtCore>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class SocketManager;

/* Never delete a Socket SQ directly */
class SocketSQ : public QObject, public boost::enable_shared_from_this<SocketSQ>
{
    Q_OBJECT
    friend class SocketManager;

    SocketSQ(SocketManager *manager, boost::asio::ip::tcp::socket *socket);
    SocketSQ(SocketManager *manager, boost::asio::ip::tcp::acceptor *socket);
public:
    ~SocketSQ();

    typedef boost::shared_ptr<SocketSQ> pointer;
    /* For server sockets */
    pointer nextPendingConnection();

    boost::asio::ip::tcp::socket &sock();
    const boost::asio::ip::tcp::socket &sock() const;
    boost::asio::ip::tcp::acceptor &server();
    const boost::asio::ip::tcp::acceptor &server() const;
    QString ip();

    void disconnectFromHost();
    int bytesAvailable();
    void getChar(char *ch);
    QByteArray read(int length);
    void putChar(char c);
    void write(const QByteArray &b);
    bool listen(quint16 port);
    void setLowDelay(bool lowdelay);
    /* For non server sockets, start the read feed */
    void start();

    bool isServer;
public slots:
    void deleteLater();
signals:
    void active();
    void disconnected();
private:
    //union {
        boost::asio::ip::tcp::socket * mysock;
        boost::asio::ip::tcp::acceptor * myserver;
    //};
    boost::asio::ip::tcp::endpoint endpoint;
    SocketManager *manager;
    QString myip;

    /* Only called by socket manager */
    void fill();
    void sendData();

    void readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void writeHandler(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void acceptHandler(const boost::system::error_code& ec);

    char innerBuffer[10000];
    QByteArray buffer;
    QByteArray toSend;
    QByteArray sending;
    QMutex m;
    /* From where in the buffer to start reading. Gets incremented when you read chars one by one.
        Counter is resetted when external actually reads a chunk. */
    int bufCounter;
    volatile bool notifiedDced;
    volatile bool freeConnection;
    boost::asio::ip::tcp::socket *incoming;
};

class SocketManager : public QThread
{
    Q_OBJECT
public:
    SocketManager();
    ~SocketManager();

    SocketSQ::pointer createSocket();
    SocketSQ::pointer createServerSocket();

    boost::asio::io_service io_service;
protected:
    void run();
private slots:
private:
    volatile bool finished;
};

typedef SocketSQ::pointer GenericSocket;
#else
class QTcpSocket;
typedef QTcpSocket* GenericSocket;
#endif

#endif // SFMLSOCKET_H

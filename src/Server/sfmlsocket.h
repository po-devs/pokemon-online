#ifndef SFMLSOCKET_H
#define SFMLSOCKET_H

#ifdef SFML_SOCKETS

#include <QtCore>
#include <iostream>
#include <boost/asio.hpp>

class SocketSQ;

class SocketManager : public QThread
{
    Q_OBJECT
public:
    SocketManager();
    ~SocketManager();

    SocketSQ * createSocket();
    SocketSQ * createServerSocket();

    boost::asio::io_service io_service;

    void deleteSocket(QObject *sock);
protected:
    void run();
private slots:
private:
    volatile bool finished;

    QMutex m;
    QVector<QObject *> toDelete;
};

/* Never delete a Socket SQ directly */
class SocketSQ : public QObject
{
    Q_OBJECT
    friend class SocketManager;

    SocketSQ(SocketManager *manager, boost::asio::ip::tcp::socket *socket);
    SocketSQ(SocketManager *manager, boost::asio::ip::tcp::acceptor *socket);
    ~SocketSQ();
public:
    /* For server sockets */
    SocketSQ * nextPendingConnection();

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


typedef SocketSQ GenericSocket;
#endif

#endif // SFMLSOCKET_H

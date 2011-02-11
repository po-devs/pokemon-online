#ifndef SFMLSOCKET_H
#define SFMLSOCKET_H

class QTcpSocket;

#ifdef SFML_SOCKETS
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <QtCore>

class SocketSQ;

class SocketManager : public sf::Thread
{
public:
    SocketManager();
    ~SocketManager();

    virtual void Run();

    SocketSQ * createSocket();

    void addSocket(SocketSQ *s);
    void deleteSocket(SocketSQ *s);
private slots:
private:
    sf::Mutex lock;
    volatile bool finished;

    QList<SocketSQ*> socketsToRemove;
    QList<SocketSQ*> socketsToAdd;

    QSet<SocketSQ*> heap;
};

/* Never delete a Socket SQ directly */
class SocketSQ : public QObject
{
    Q_OBJECT
    friend class SocketManager;

    SocketSQ(SocketManager *manager, sf::SocketTCP s = sf::SocketTCP());
public:
    void delayDeath();

    /* For server sockets */
    SocketSQ * nextPendingConnection();

    sf::SocketTCP &sock();
    const sf::SocketTCP &sock() const;
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
    sf::SocketTCP mysock;
    SocketManager *manager;
    QString myip;

    void fill();

    QByteArray buffer;
    QByteArray toSend;
    sf::Mutex m;
    int bufCounter;
    volatile bool notifiedDced;
    int mycounter;
    volatile bool freeConnection;
    SocketSQ *incoming;
};

typedef SocketSQ GenericSocket;
#else
typedef QTcpSocket GenericSocket;
#endif

#endif // SFMLSOCKET_H

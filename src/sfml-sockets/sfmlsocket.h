#ifndef SFMLSOCKET_H
#define SFMLSOCKET_H

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
    void addSendingSocket(SocketSQ *s);
private slots:
private:
    sf::Mutex lock;
    volatile bool finished;

    QList<SocketSQ*> socketsToRemove;
    QList<SocketSQ*> socketsToAdd;
    QLinkedList<SocketSQ*> waitingList;
    QLinkedList<SocketSQ*> socketsToSend;

    QSet<SocketSQ*> heap;

    /* Causes threading issues so made private in order to not be called rashly */
    bool existSocket(SocketSQ *s) const;
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
    bool connectTo(quint16 port, const std::string &ip);

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

    /* Only called by socket manager */
    void fill();
    void sendData();

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

#endif // SFMLSOCKET_H

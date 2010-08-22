#ifndef REGISTRY_H
#define REGISTRY_H

#include <QtCore>
#include <QtNetwork>

class Player;
class Server;
class QScrollDownTextEdit;

class Registry: public QObject
{
    Q_OBJECT
public:
    Registry();

    void printLine(const QString &line);
private slots:
    void incomingPlayer();
    void incomingServer();

    void nameChangedAcc(int id, const QString &name);
    void portSet(int id, int port, int oldport);
    void disconnection(int id);

    /* Called by the anti DoS */
    void kick(int id);
    void ban(const QString &ip);
private:
    QTcpServer forServers;
    QHash<int, Server *> servers;
    QSet<QString> names;
    QSet<QString> serverAddresses;

    QTcpServer forPlayers;
    QHash<int, Player *> players;

    QSet<QString> bannedIPs;
    QHash<QString, int> ipCounter;

    QScrollDownTextEdit *mainChat;
    int linecount;

    int freeid() const;
};

#endif // REGISTRY_H

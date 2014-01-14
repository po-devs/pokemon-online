#ifndef REGISTRY_H
#define REGISTRY_H

#include <QtCore>
#include <QtNetwork>

class Player;
class Server;
class QScrollDownTextBrowser;
#ifdef USE_WEBCONF
class RegistryWebInterface;
#endif

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

    /* timers */
    void updateTBanList();
    void tbanListReceived(QNetworkReply*);

    void updateRegistryAnnouncement();

private:
    QTcpServer forServers;
    QHash<int, Server *> servers;
    QSet<QString> names;
    QSet<QString> serverAddresses;

    QTcpServer forPlayers[2];
    QHash<int, Player *> players;

    QSet<QString> bannedIPs;
    QSet<QString> tbanIPs;
    QHash<QString, int> ipCounter;

    QString registry_announcement;

    QNetworkAccessManager manager;

    QScrollDownTextBrowser *mainChat;
    int linecount;

    int freeid() const;
#ifdef USE_WEBCONF
    RegistryWebInterface *web_interface;
    friend class RegistryWebInterface;
#endif
};

#endif // REGISTRY_H

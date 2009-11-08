#ifndef SERVER_H
#define SERVER_H

#include "../PokemonInfo/pokemonstructs.h"
#include "analyze.h"

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
    Player(QTcpSocket *sock);
    ~Player();

    TeamInfo &team();
    const TeamInfo &team() const;
    void sendMessage(const QString &mess);

    void setId(int id);
    int id() const;
    QString name() const;

    Analyzer& relay();
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id);
public slots:
    void loggedIn(const QString &name);
    void recvMessage(const QString &mess);
    void recvTeam(const TeamInfo &team);
    void disconnected();
private:
    TeamInfo myteam;
    Analyzer myrelay;
    int myid;
};

/* the server */

class Server: public QWidget
{
    Q_OBJECT
public:
    Server(quint16 port = 5080);

    void printLine(const QString &line);
    /* returns the name of that player */
    QString name(int id) const;
    /* sends a message to all the players */
    void sendAll(const QString &message);
    void sendMessage(int id, const QString &message);
    void sendPlayersList(int id);
    /* Sends the login of the player to everybody but the player */
    void sendLogin(int id);
    void sendLogout(int id);
public slots:
    /* means a new connection is about to start from the TCP server */
    void incomingConnection();
    /* Signals received by players */
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void recvTeam(int id);
    void disconnected(int id);
private:
    QTcpServer myserver;
    /* storing players */
    QMap<int, Player*> myplayers;

    QTcpServer *server();
    Player * player(int i);
    const Player * player(int id) const;
    /* gets an id that's not used */
    int freeid() const;
    /* removes a player */
    void removePlayer(int id);

    /** GRAPHICAL PARTS **/

    QTextEdit *mymainchat;
    /* the mainchat !*/
    QTextEdit *mainchat();
};
#endif // SERVER_H

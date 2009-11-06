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

    TrainerTeam &team();
    void sendMessage(const QString &mess);

    void setId(int id);
    int id() const;
signals:
    void loggedIn(const QString &name);
    void recvMessage(const QString &mess);
public slots:
    void loggedIn(const QString &name);
    void recvMessage(const QString &mess);
private:
    TrainerTeam myteam;
    Analyzer myrelay;
    int myid;
};

/* the server */

class Server: public QWidget
{
    Q_OBJECT
public:
    Server(quint16 port = 5080);
public slots:
    void incomingConnection();
private:
    QTcpServer *myserver;
    /* storing players */
    QMap<int, Player> myplayers;

    QTcpServer server();
};
#endif // SERVER_H

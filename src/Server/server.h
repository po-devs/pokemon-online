#ifndef SERVER_H
#define SERVER_H

#include "../PokemonInfo/networkstructs.h"
#include "analyze.h"

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
    Player(QTcpSocket *sock);
    ~Player();

    /* returns all the regular info */
    TeamInfo &team();
    const TeamInfo &team() const;
    /* Converts the content of the TeamInfo to a basicInfo and returns it */
    BasicInfo basicInfo() const;

    /* Sends a message to the player */
    void sendMessage(const QString &mess);

    void setId(int id);
    int id() const;
    QString name() const;

    bool isLoggedIn() const;
    void setLoggedIn(bool logged);

    bool connected() const;

    bool isChallenged() const;
    bool hasChallenged() const;
    int challengedBy() const;
    int challenged() const;

    /* Sends the challenge, returns false if can't even send the challenge */
    bool challenge(int id);
    void sendBusyForChallenge(int id);
    void startBattle(int id);
    void sendChallengeRefusal(int id);

    Analyzer& relay();
    const Analyzer& relay() const;
signals:
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void disconnected(int id);
    void recvTeam(int id);
    void challengeFromTo(int idfrom, int idto);
    void challengeAcc(int idfrom, int idto);
    void challengeRef(int idfrom, int idto);
    void busyForChallenge(int idfrom, int idto);
public slots:
    void loggedIn(const TeamInfo &team);
    void recvMessage(const QString &mess);
    void recvTeam(const TeamInfo &team);
    void disconnected();
    void challengeReceived(int id);
    void challengeRefused(int id);
    void challengeAccepted(int id);
    void busyForChallenge(int id);
private:
    TeamInfo myteam;
    Analyzer myrelay;
    int myid;

    int m_challenged;
    int m_challengedby;
    bool m_hasChallenged;
    bool m_isChallenged;

    bool m_isLoggedIn;
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
    bool playerExist(int id) const;
    void startBattle(int id);
public slots:
    /* means a new connection is about to start from the TCP server */
    void incomingConnection();
    /* Signals received by players */
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void recvTeam(int id);
    void disconnected(int id);
    void dealWithChallenge(int, int);
    void challengeAccepted(int, int);
    void challengeRefused(int, int);
    void busyForChallenge(int, int);

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

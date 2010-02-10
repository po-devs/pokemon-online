#ifndef SERVER_H
#define SERVER_H

#include <QtGui>
#include <QtNetwork>

/* the server */

class Player;
class BattleSituation;
class Analyzer;
class ChallengeInfo;

class QIdListWidgetItem;

class Server: public QWidget
{
    Q_OBJECT
public:
    Server(quint16 port = 5080);

    void printLine(const QString &line);
    /* returns the name of that player */
    QString name(int id) const;
    QString authedName(int id) const;
    /* sends a message to all the players */
    void sendAll(const QString &message);
    void sendMessage(int id, const QString &message);
    void sendPlayersList(int id);
    /* Sends the login of the player to everybody but the player */
    void sendLogin(int id);
    void sendLogout(int id);
    bool playerExist(int id) const;
    void removeBattle(int id1, int id2);
public slots:
    /* Registry slots */
    void connectToRegistry();
    void regConnected();
    void regConnectionError();
    void regSendPlayers();
    void sendServerMessage();
    void regNameChanged(const QString &name);
    void regDescChanged(const QString &desc);
    void openConfig();
    void nameTaken();
    void ipRefused();
    void invalidName();
    void accepted();
    /* means a new connection is about to start from the TCP server */
    void incomingConnection();
    /* Signals received by players */
    void loggedIn(int id, const QString &name);
    void recvMessage(int id, const QString &mess);
    void recvPM(int src, int dest, const QString &mess);
    void recvTeam(int id, const QString &name);
    void disconnected(int id);
    void dealWithChallenge(int from, int to, const ChallengeInfo &c);
    void startBattle(int id1, int id2, const ChallengeInfo &c);
    void battleResult(int desc, int winner, int loser);
    void sendBattleCommand(int id, const QByteArray &command);
    void info(int , const QString& );
    void showContextMenu(const QPoint &p);
    void kick(int i);
    void silentKick(int i);
    void ban(int i);
    void dosKick(int id);
    void dosBan(const QString &ip);
    void openPlayers();
    void openAntiDos();
    void changeAuth(const QString &name, int auth);
    void banName(const QString &name);
    void playerKick(int src, int dest);
    void playerBan(int src, int dest);
private:
    void kick(int dest, int src);
    void ban(int dest, int src);
    void sendPlayer(int id);

    Analyzer *registry_connection;
    QString serverName, serverDesc;

    quint16 numPlayers() {
        return myplayers.size();
    }

    QTcpServer myserver;
    /* storing players */
    QHash<int, Player*> myplayers;
    QHash<QString, int> mynames;
    QHash<int, BattleSituation*> mybattles;

    QTcpServer *server();
    Player * player(int i);
    const Player * player(int id) const;
    /* gets an id that's not used */
    int freeid() const;
    /* removes a player */
    void removePlayer(int id);

    int linecount;
    int textLength;

    /** GRAPHICAL PARTS **/

    QTextEdit *mymainchat;
    QListWidget *mylist;
    QLineEdit *myline;
    QHash<int, QIdListWidgetItem *> myplayersitems;
    /* the mainchat !*/
    QTextEdit *mainchat();
    QListWidget *list();
};
#endif // SERVER_H

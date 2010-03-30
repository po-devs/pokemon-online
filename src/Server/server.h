#ifndef SERVER_H
#define SERVER_H

#include <QtGui>
#include <QtNetwork>

/* the server */

class Player;
class BattleSituation;
class Analyzer;
class ChallengeInfo;
class QScrollDownTextEdit;
class ScriptEngine;
class ScriptWindow;
class Challenge;
class QIdListWidgetItem;

class Server: public QWidget
{
    Q_OBJECT

    friend class ScriptEngine;
public:
    Server(quint16 port = 5080);

    void printLine(const QString &line, bool chatMessage = false);
    /* returns the name of that player */
    QString name(int id) const;
    QString authedName(int id) const;
    /* sends a message to all the players */
    void sendAll(const QString &message, bool chatMessage = false);
    void sendMessage(int id, const QString &message);
    void sendPlayersList(int id);
    /* Sends the login of the player to everybody but the player */
    void sendLogin(int id);
    void sendLogout(int id);
    bool playerExist(int id) const;
    bool playerLoggedIn(int id) const;
    bool nameExist(const QString &name) const;
    int id(const QString &name) const;
    int auth(int id) const;
    void removeBattle(int id1, int id2);
    void beforeChallengeIssued(int src, int dest, Challenge *c);
    void afterChallengeIssued(int src, int dest, Challenge *c);
    void atServerShutDown();

    Player * player(int id) const;
public slots:
    /* Registry slots */
    void connectToRegistry();
    void regConnected();
    void regConnectionError();
    void regSendPlayers();
    void sendServerMessage();
    void regNameChanged(const QString &name);
    void regDescChanged(const QString &desc);
    void regMaxChanged(const int &num);
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
    void battleResult(int desc, int winner, int loser, bool rated);
    void sendBattleCommand(int id, const QByteArray &command);
    void spectatingRequested(int id, int battle);
    void spectatingStopped(int id, int battle);
    void spectatingChat(int player, int battle, const QString &chat);
    void info(int , const QString& );
    void showContextMenu(const QPoint &p);
    void kick(int i);
    void silentKick(int i);
    void ban(int i);
    void dosKick(int id);
    void dosBan(const QString &ip);
    void openPlayers();
    void openAntiDos();
    void openScriptWindow();
    void openTiersWindow();
    void changeAuth(const QString &name, int auth);
    void banName(const QString &name);
    void playerKick(int src, int dest);
    void playerBan(int src, int dest);
    void awayChanged(int src, bool away);
    void sendPlayer(int id);
    void tiersChanged();
private:
    void kick(int dest, int src);
    void ban(int dest, int src);

    Analyzer *registry_connection;
    QString serverName, serverDesc;
    quint16 serverPlayerMax;
    quint16 numPlayers() {
        return myplayers.size();
    }

    QTcpServer myserver;
    /* storing players */
    QHash<int, Player*> myplayers;
    QHash<QString, int> mynames;

    QHash<int, BattleSituation *> mybattles;

    QTcpServer *server();
    Player * player(int i);
    /* gets an id that's not used */
    int freeid() const;
    int freebattleid() const;
    /* removes a player */
    void removePlayer(int id);

    int linecount;
    int textLength;

    ScriptEngine *myengine;
    QPointer<ScriptWindow> myscriptswindow;

    /** GRAPHICAL PARTS **/

    QScrollDownTextEdit *mymainchat;
    QListWidget *mylist;
    QLineEdit *myline;
    QHash<int, QIdListWidgetItem *> myplayersitems;
    /* the mainchat !*/
    QScrollDownTextEdit *mainchat();
    QListWidget *list();
};
#endif // SERVER_H

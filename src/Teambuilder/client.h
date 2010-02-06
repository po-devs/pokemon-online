#ifndef CLIENT_H
#define CLIENT_H

#include <QtGui>
#include "analyze.h"
#include "../PokemonInfo/networkstructs.h"

class MainWindow;
class BaseChallengeWindow;
class QIdListWidgetItem;
class BattleWindow;
class QScrollDownTextEdit;
class PMWindow;

/* Struct representing a player's data */
class Player
{
public:
    qint32 id;
    BasicInfo team;
    qint8 auth;
};

QDataStream & operator >> (QDataStream &in, Player &p);
QDataStream & operator << (QDataStream &out, const Player &p);

/* The class for going online.

    It displays the mainchat, the players list, ... and also have the dialog engine in it*/

class Client : public QWidget
{
    Q_OBJECT
public:
    Client(TrainerTeam *, const QString &url);

    TrainerTeam *team();
    QMenuBar *createMenuBar(MainWindow *w);

    void printLine(const QString &line);
    void printHtml(const QString &html);
    bool playerExist(int id) const;
    QString name(int id) const;
    QString ownName() const;
    bool battling() const;
    bool busy() const;
    int id(const QString &name) const;

    void seeChallenge(const ChallengeInfo &c);
    bool challengeWindowOpen() const;
    void closeChallengeWindow();
    int  challengeWindowPlayer() const;

    Player player(int id) const;
    BasicInfo info(int id) const;

    void removeBattleWindow();
    void removePlayer(int id);
public slots:
    void errorFromNetwork(int errnum, const QString &error);
    void connected();
    void disconnected();
    /* message received from the server */
    void messageReceived(const QString & mess);
    /* sends what's in the line edit */
    void sendText();
    void playerLogin(const Player &p);
    void playerReceived(const Player &p);
    void teamChanged(const Player &p);
    void playerLogout(int);
    void sendRegister();
    /* sends the server a challenge notice */
    void sendChallenge(int id);
    /* removes the pointer to the challenge window when it is destroyed */
    void clearChallenge();
    /* sends the server a "Accept Challenge" notice */
    void acceptChallenge(int id);
    void refuseChallenge(int id);
    /* Display the info for that player */
    void seeInfo(int id);
    /* Challenge info by the server */
    void challengeStuff(const ChallengeInfo &c);
    /* battle... */
    void battleStarted(int id, const TeamBattle &team, const BattleConfiguration &conf);
    void battleFinished(int res, int winner, int loser);
    void forfeitBattle();
    /* shows the context menu for that player */
    void showContextMenu(const QPoint&);
    void loadTeam();
    /* A popup that asks for the pass */
    void askForPass(const QString &salt);
    /* When someone is kicked */
    void playerKicked(int,int);
    void playerBanned(int,int);
    /* When you kick someone */
    void kick(int);
    void ban(int);
    /* PM */
    void startPM(int);
    void removePM(int);
    void PMReceived(int, const QString);
    /* Teambuilder slots */
    void openTeamBuilder();
    void changeTeam();
    void showDock(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation);
signals:
    void done();

private:
    TrainerTeam *myteam;
    QString mynick;
    /* Main chat */
    QScrollDownTextEdit *mychat;
    /* PMs */
    QHash<int, PMWindow*> mypms;
    /* Line the user types in */
    QLineEdit *myline;
    /* Where players are displayed */
    QListWidget *myplayers;
    QHash<int, QIdListWidgetItem *> myplayersitems;
    /* Button to exit */
    QPushButton *myexit;
    /* Button to send text */
    QPushButton *mysender;
    /* Button to register a password */
    QPushButton *myregister;
    /* Network Relay */
    Analyzer myrelay;
    /* Challenge window , to emit or to receive*/
    BaseChallengeWindow *mychallenge;
    QPointer<BattleWindow> mybattle;

    /* You can call the teambuilder from here too */
    QPointer<QMainWindow> myteambuilder;

    QHash<int, Player> myplayersinfo;


    QHash<QString, int> mynames;
    QScrollDownTextEdit *mainChat();
    Analyzer & relay();

    void initRelay();
};

#endif // CLIENT_H

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
class ControlPanel;


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
    int ownId() const ;
    int ownAuth() const ;
    int auth(int id) const ;

    void seeChallenge(const ChallengeInfo &c);
    bool challengeWindowOpen() const;
    void closeChallengeWindow();
    int  challengeWindowPlayer() const;

    PlayerInfo player(int id) const;
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
    void playerLogin(const PlayerInfo &p);
    void playerReceived(const PlayerInfo &p);
    void teamChanged(const PlayerInfo &p);
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
    void battleStarted(int id1, int id2);
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
    /* CP */
    void controlPanel(int);
    void setPlayer(const UserInfo &ui);
    /* Teambuilder slots */
    void openTeamBuilder();
    void changeTeam();
    void showDock(Qt::DockWidgetArea areas,QDockWidget * dock,Qt::Orientation);
signals:
    void done();
    void userInfoReceived(const UserInfo &ui);

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

    QPointer<ControlPanel> myCP;

    QHash<int, PlayerInfo> myplayersinfo;


    QHash<QString, int> mynames;
    QScrollDownTextEdit *mainChat();
    Analyzer & relay();

    PlayerInfo playerInfo(int id) const;
    QIdListWidgetItem *item(int id);
    void updateState(int player);

    void initRelay();
};

#endif // CLIENT_H

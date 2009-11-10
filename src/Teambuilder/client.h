#ifndef CLIENT_H
#define CLIENT_H

#include <QtGui>
#include "analyze.h"
#include "../PokemonInfo/pokemonstructs.h"

class MainWindow;

class Player
{
public:
    int id;
    TeamInfo team;
};

QDataStream & operator >> (QDataStream &in, Player &p);
QDataStream & operator << (QDataStream &out, const Player &p);

/* The class for going online */

class Client : public QWidget
{
    Q_OBJECT
public:
    Client(TrainerTeam *, const QString &url);

    TrainerTeam *team();
    QMenuBar *createMenuBar(MainWindow *w);

    void printLine(const QString &line);
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
    void playerLogout(int);
signals:
    void done();

private:
    TrainerTeam *myteam;
    /* Main chat */
    QTextEdit *mychat;
    /* Line the user types in */
    QLineEdit *myline;
    /* Where players are displayed */
    QListWidget *myplayers;
    QMap<int, QListWidgetItem *> myplayersitems;
    /* Button to exit */
    QPushButton *myexit;
    /* Button to send text */
    QPushButton *mysender;
    /* Network Relay */
    Analyzer myrelay;

    QMap<int, TeamInfo> myplayersinfo;
    QTextEdit *mainChat();
    Analyzer & relay();

    void initRelay();
};

#endif // CLIENT_H

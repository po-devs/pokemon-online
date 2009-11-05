#ifndef CLIENT_H
#define CLIENT_H

#include <QtGui>
#include "analyze.h"

class TrainerTeam;
class MainWindow;

/* The class for going online */

class Client : public QWidget
{
    Q_OBJECT
public:
    Client(TrainerTeam *);

    TrainerTeam *team();
    QMenuBar *createMenuBar(MainWindow *w);
public slots:
    void errorFromNetwork(int errno, const QString &error);

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
    /* Button to exit */
    QPushButton *myexit;
    /* Network Relay */
    Analyzer myrelay;

    Analyzer & relay();

    void initRelay();
};

#endif // CLIENT_H

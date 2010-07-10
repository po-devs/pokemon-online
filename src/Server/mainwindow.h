#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "server.h"
#include "serverwidget.h"

class Server;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Server *myserver, QWidget *parent = 0);

    ~MainWindow();

 /*   void hideEvent(QHideEvent * event)
    {
        sticon->show();//affichage du tray
        hide();//"cachage" de la fenetre =)

    }
    void showEvent(QShowEvent * event)
    {
        sticon->hide();
        show();
    }
*/
protected:
    void closeEvent(QCloseEvent *e);
private:
    ServerWidget * myserverwidget;
    QSystemTrayIcon *sticon;
};

#endif // MAINWINDOW_H

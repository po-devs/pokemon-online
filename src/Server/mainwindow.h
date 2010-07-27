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

public slots:
    void reloadMenuBar();
protected:
    void closeEvent(QCloseEvent *e);
private:
    ServerWidget * myserverwidget;
    QSystemTrayIcon *sticon;
};

#endif // MAINWINDOW_H

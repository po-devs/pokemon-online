#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

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
    void createTrayIcon();
    void openWindow(); // Hehe, no idea of other way of doing this. Though.
    void systemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void reloadMenuBar();

protected:
    void closeEvent(QCloseEvent *e);
    virtual bool event(QEvent *event);

private:
    ServerWidget * myserverwidget;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

};

#endif // MAINWINDOW_H





#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class Server;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
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
    quint16 serverPort;
    Server * myserver;
    QSystemTrayIcon *sticon;
};

#endif // MAINWINDOW_H

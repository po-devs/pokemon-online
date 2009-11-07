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
private:
    Server * myserver;
};

#endif // MAINWINDOW_H

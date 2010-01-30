#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class Registry;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Registry * myserver;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>

class Registry;

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject *parent = 0);
    ~MainWindow();
private:
    Registry * myserver;
};

#endif // MAINWINDOW_H

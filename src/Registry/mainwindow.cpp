#include "mainwindow.h"
#include "registry.h"

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    myserver = new Registry();
}

MainWindow::~MainWindow()
{
}

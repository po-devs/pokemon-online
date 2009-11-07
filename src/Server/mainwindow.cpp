#include "mainwindow.h"
#include "server.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Server for Pokemon Online"));

    setCentralWidget(myserver = new Server());
    resize(500,500);
}

MainWindow::~MainWindow()
{
}

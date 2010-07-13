#include "mainwindow.h"
#include "server.h"
#include "serverwidget.h"

MainWindow::MainWindow(Server *myserver, QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    setWindowTitle(tr("Server for Pokemon Online"));
    setCentralWidget(myserverwidget = new ServerWidget(myserver));
    resize(500,500);
    setMenuBar(myserverwidget->createMenuBar());
}

MainWindow::~MainWindow()
{
    delete myserverwidget;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    myserver->atServerShutDown();
    exit(0);
}

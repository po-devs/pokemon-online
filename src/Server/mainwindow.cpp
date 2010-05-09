#include "mainwindow.h"
#include "server.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    setWindowTitle(tr("Server for Pokemon Online"));
    QSettings s;
    if (s.value("server_port").isNull())
         s.setValue("server_port", 5080);

    serverPort = quint16(s.value("server_port").toInt());

    setCentralWidget(myserver = new Server(serverPort));
    resize(500,500);
}

MainWindow::~MainWindow()
{
    delete myserver;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    myserver->atServerShutDown();
    exit(0);
}

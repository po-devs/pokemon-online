#include "mainwindow.h"
#include "server.h"
#include "serverwidget.h"

MainWindow::MainWindow(Server *myserver, QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    setWindowTitle(tr("Pokémon Online Server"));
    //setWindowIcon(QIcon("db/icon.png"));

    setCentralWidget(myserverwidget = new ServerWidget(myserver));
    resize(500,500);
    setMenuBar(myserverwidget->createMenuBar());

    createTrayIcon();

    connect(myserverwidget, SIGNAL(menuBarChanged()), this, SLOT(reloadMenuBar()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));

}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized()) {
            trayIcon->show();
            trayIcon->showMessage("Pokemon Online is Still runnning", "The program will keep running in system Tray. You can left click the icon to get a menu of options.");
            setParent(NULL, Qt::SubWindow); // We're avoiding to make the Application show again in the Taskbar.
            hide();
            event->ignore();
        }
    }
    QMainWindow::event(event);
    return true;
}

void MainWindow::systemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger) {
        setParent(NULL, Qt::Window);
        showNormal();
        trayIcon->hide();
    }
}

void MainWindow::openWindow()
{
    setParent(NULL, Qt::Window);
    showNormal();
    trayIcon->hide();
}

void MainWindow::reloadMenuBar()
{
    setMenuBar(myserverwidget->createMenuBar());
}

void MainWindow::createTrayIcon()
{
    trayMenu = new QMenu(this);
    trayMenu->addAction("&Players", myserverwidget, SLOT(openPlayers()));
    trayMenu->addAction("&Anti DoS", myserverwidget, SLOT(openAntiDos()));
    trayMenu->addAction("&Config", myserverwidget, SLOT(openConfig()));
    trayMenu->addAction("&Script Window", myserverwidget, SLOT(openScriptWindow()));
    trayMenu->addAction("&Battle Config", myserverwidget, SLOT(openBattleConfigWindow()));
    trayMenu->addAction("&SQL Config", myserverwidget, SLOT(openSqlConfigWindow()));
    trayMenu->addSeparator();
    trayMenu->addAction("&Plugin Manager", myserverwidget, SLOT(openPluginManager()));
    trayMenu->addSeparator();
    trayMenu->addAction("&Open", this, SLOT(openWindow()));
    trayMenu->addAction("&Close", qApp, SLOT(quit()));
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->setIcon(QIcon("db/icon.png"));
}

MainWindow::~MainWindow()
{
    delete myserverwidget;
    delete trayIcon;
    delete trayMenu;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    myserverwidget->atShutDown();
    exit(0);
}

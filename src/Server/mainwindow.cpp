#include "mainwindow.h"
#include "server.h"
#include "serverwidget.h"

MainWindow::MainWindow(Server *myserver, QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    setWindowTitle(tr("Pokémon Online Server"));
    setWindowIcon(QIcon("db/icon.png"));

    setCentralWidget(myserverwidget = new ServerWidget(myserver));
    resize(500,500);
    setMenuBar(myserverwidget->createMenuBar());

    createTrayActions();
    createTrayIcon();

    trayIcon->show();

    connect(myserverwidget, SIGNAL(menuBarChanged()), this, SLOT(reloadMenuBar()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));

}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized()) {
            if(trayIcon->isVisible()) {
                trayIcon->showMessage("Pokemon Online is Still runnning", "The program will keep running in system Tray. You can left click the icon to get a menu of options.");
                setParent(NULL, Qt::SubWindow); // We're avoiding to make the Application show again in the Taskbar.
                hide();
                event->ignore();
            }else{
                event->accept();
            }
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
    }
}

void MainWindow::openWindow()
{
    setParent(NULL, Qt::Window);
    showNormal();
}

void MainWindow::reloadMenuBar()
{
    setMenuBar(myserverwidget->createMenuBar());
}

void MainWindow::createTrayIcon()
{
    trayMenu = new QMenu(this);
    trayMenu->addAction(playersAction);
    trayMenu->addAction(antiDOSAction);
    trayMenu->addAction(configAction);
    trayMenu->addAction(scriptsAction);
    trayMenu->addAction(battleCAction);
    trayMenu->addAction(SQLCAction);
    trayMenu->addSeparator();
    trayMenu->addAction(pulginsWindow);
    trayMenu->addSeparator();
    trayMenu->addAction(openAction);
    trayMenu->addAction(closeAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->setIcon(QIcon("db/icon.png"));
}

void MainWindow::createTrayActions()
{
    playersAction = new QAction(tr("&Players"), this);
    antiDOSAction = new QAction(tr("&Anti DoS"), this);
    configAction = new QAction(tr("&Config"), this);
    scriptsAction = new QAction(tr("&Script Window"), this);
    battleCAction = new QAction(tr("&Battle Config"), this);
    SQLCAction = new QAction(tr("&SQL Config"), this);

    pulginsWindow = new QAction(tr("&Pulgin Manager"), this);

    openAction = new QAction(tr("Open"), this);
    closeAction = new QAction(tr("Close"), this);

    connect(playersAction, SIGNAL(triggered()), myserverwidget, SLOT(openPlayers()));
    connect(antiDOSAction, SIGNAL(triggered()), myserverwidget, SLOT(openAntiDos()));
    connect(configAction, SIGNAL(triggered()), myserverwidget, SLOT(openConfig()));
    connect(scriptsAction, SIGNAL(triggered()), myserverwidget, SLOT(openScriptWindow()));
    connect(battleCAction, SIGNAL(triggered()), myserverwidget, SLOT(openBattleConfigWindow()));
    connect(SQLCAction, SIGNAL(triggered()), myserverwidget, SLOT(openSqlConfigWindow()));

    connect(pulginsWindow, SIGNAL(triggered()), myserverwidget, SLOT(openPluginManager()));

    connect(openAction, SIGNAL(triggered()), this, SLOT(openWindow()));
    connect(closeAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    /* Note! @Latios
      If you do show() on openAction, it'll not work. If you do showNormal() it will show but you can't resize, minimize or
      close it.
      If there's otherway to fix this than creating another function, please fix it. :-)!
    */

}

MainWindow::~MainWindow()
{
    delete myserverwidget;
    delete trayIcon;
    delete trayMenu;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    myserverwidget->atShutDown();
    exit(0);
}

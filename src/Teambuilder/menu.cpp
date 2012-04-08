#include "menu.h"
#include "ui_menu.h"
#include "../Utilities/functions.h"
#include "mainwindow.h"

Menu::Menu(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    setWindowTitle(tr("Menu"));

    connect (ui->teamBuilder, SIGNAL(clicked()), SIGNAL(goToTeambuilder()));
    connect (ui->goOnline, SIGNAL(clicked()), SIGNAL(goToOnline()));
    connect (ui->credits, SIGNAL(clicked()), SIGNAL(goToCredits()));
    connect (ui->exit, SIGNAL(clicked()), SIGNAL(goToExit()));

    loadSettings(this);
}

Menu::~Menu()
{
    writeSettings(this);
    delete ui;
}

QMenuBar * Menu::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),w,SLOT(loadTeamDialog()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &replay"),w,SLOT(loadReplayDialog()), Qt::CTRL+Qt::Key_R);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

    w->addThemeMenu(menuBar);

    QMenu *langMenu = menuBar->addMenu(tr("&Language"));
    QFile in ("languages.txt");
    in.open(QIODevice::ReadOnly);
    QStringList langs = QString::fromUtf8(in.readAll()).split('\n');
    foreach(QString a, langs) {
        langMenu->addAction(a,w, SLOT(changeLanguage()));
    }

    return menuBar;
}

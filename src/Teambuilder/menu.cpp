#include "menu.h"
#include "mainwindow.h"
#include "../PokemonInfo/pokemoninfo.h"

TB_Menu::TB_Menu()
{
    setWindowTitle(tr("Menu"));
    setFixedSize(200,200);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QPushButton *teambuilder, *online, *credits, *exit;

    layout->addWidget(teambuilder = new QPushButton(tr("&Teambuilder")), 0, Qt::AlignCenter);
    layout->addWidget(online = new QPushButton(tr("Go &Online")), 0, Qt::AlignCenter);
    layout->addWidget(credits = new QPushButton(tr("See &Credits")), 0, Qt::AlignCenter);
    layout->addWidget(exit = new QPushButton(tr("&Exit")), 0, Qt::AlignCenter);

    connect (teambuilder, SIGNAL(clicked()), SLOT(launchTeambuilder()));
    connect (online, SIGNAL(clicked()), SLOT(goOnline()));
    connect (credits, SIGNAL(clicked()), SLOT(launchCredits()));
    connect (exit, SIGNAL(clicked()), SLOT(exit()));
}

QMenuBar * TB_Menu::createMenuBar(MainWindow *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu("&File");
    menuFichier->addAction(tr("&Load Team"),w,SLOT(loadTeamDialog()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("&Quit"),w,SLOT(close()),Qt::CTRL+Qt::Key_Q);

    return menuBar;
}

void TB_Menu::launchCredits()
{
    emit goToCredits();
}

void TB_Menu::launchTeambuilder()
{
    emit goToTeambuilder();
}

void TB_Menu::goOnline()
{
    emit goToOnline();
}

void TB_Menu::exit()
{
    emit goToExit();
}

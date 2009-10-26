#include "menu.h"

TB_Menu::TB_Menu()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QPushButton *teambuilder, *online, *credits, *exit;

    layout->addWidget(teambuilder = new QPushButton(tr("Teambuilder")), 0, Qt::AlignCenter);
    layout->addWidget(online = new QPushButton(tr("Go Online")), 0, Qt::AlignCenter);
    layout->addWidget(credits = new QPushButton(tr("See Credits")), 0, Qt::AlignCenter);
    layout->addWidget(exit = new QPushButton(tr("Exit")), 0, Qt::AlignCenter);

    connect (teambuilder, SIGNAL(clicked()), SLOT(launchTeambuilder()));
    connect (online, SIGNAL(clicked()), SLOT(goOnline()));
    connect (credits, SIGNAL(clicked()), SLOT(launchCredits()));
    connect (exit, SIGNAL(clicked()), SLOT(deleteLater()));
}

void TB_Menu::launchCredits()
{
}

void TB_Menu::launchTeambuilder()
{
    TeamBuilder *teambuilder = new TeamBuilder();
    teambuilder->show();

    hide();

    connect(teambuilder, SIGNAL(destroyed()), SLOT(show()));
}

void TB_Menu::goOnline()
{
}

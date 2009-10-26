#include "menu.h"

TB_Menu::TB_Menu()
{
    resize(200,200);
    setWindowTitle(tr("Menu"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QPushButton *teambuilder, *online, *credits, *exit;

    layout->addWidget(teambuilder = new QPushButton(tr("Teambuilder")), 0, Qt::AlignCenter);
    layout->addWidget(online = new QPushButton(tr("Go Online")), 0, Qt::AlignCenter);
    layout->addWidget(credits = new QPushButton(tr("See Credits")), 0, Qt::AlignCenter);
    layout->addWidget(exit = new QPushButton(tr("Exit")), 0, Qt::AlignCenter);

    connect (teambuilder, SIGNAL(clicked()), SLOT(launchTeambuilder()));
    connect (online, SIGNAL(clicked()), SLOT(goOnline()));
    connect (credits, SIGNAL(clicked()), SLOT(launchCredits()));
    connect (exit, SIGNAL(clicked()), qApp, SLOT(quit()));
}

void TB_Menu::launchCredits()
{
}

void TB_Menu::launchTeambuilder()
{
    TeamBuilder *teambuilder = new TeamBuilder(team());
    teambuilder->show();

    hide();

    connect(teambuilder, SIGNAL(destroyed()), SLOT(show()));
}

Team * TB_Menu::team()
{
    return &m_Team;
}

void TB_Menu::goOnline()
{
}

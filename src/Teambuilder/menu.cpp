#include "menu.h"

TB_Menu::TB_Menu()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");
    NatureInfo::init("db/");
    CategoryInfo::init("db/");
    AbilityInfo::init("db/");
    GenderInfo::init("db/");
    HiddenPowerInfo::init("db/");

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

    connect (this, SIGNAL(destroyed()), qApp, SLOT(quit()));
}

void TB_Menu::launchCredits()
{
}

void TB_Menu::launchTeambuilder()
{
    TeamBuilder *teambuilder = new TeamBuilder(trainerTeam());
    teambuilder->show();

    hide();

    connect(teambuilder, SIGNAL(destroyed()), SLOT(show()));
}

Team * TB_Menu::team()
{
    return & m_Team.team();
}

TrainerTeam * TB_Menu::trainerTeam()
{
    return & m_Team;
}

void TB_Menu::goOnline()
{
}

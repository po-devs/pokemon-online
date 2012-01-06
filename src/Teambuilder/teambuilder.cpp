#include "../Utilities/functions.h"
#include "teambuilder.h"
#include "trainermenu.h"
#include "teamholder.h"
#include "mainwindow.h"

TeamBuilder::TeamBuilder(TeamHolder *team) : m_team(team)
{
    addWidget(trainer = new TrainerMenu(team));

    loadSettings(this, defaultSize());

    connect(trainer, SIGNAL(done()), SIGNAL(done()));
}

TeamBuilder::~TeamBuilder()
{
    writeSettings(this);
}

QSize TeamBuilder::defaultSize() const {
    return QSize(600,400);
}

QMenuBar *TeamBuilder::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("TeamBuilder");
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&New"),this,SLOT(newTeam()),tr("Ctrl+N", "New"));
    menuFichier->addAction(tr("&Save all"),this,SLOT(saveAll()),tr("Ctrl+S", "Save"));
    menuFichier->addAction(tr("&Load all"),this,SLOT(loadAll()),tr("Ctrl+L", "Load"));
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),tr("Ctrl+Q", "Quit"));

    w->addThemeMenu(menuBar);

    return menuBar;
}

void TeamBuilder::saveAll()
{
    team().save();
}

void TeamBuilder::loadAll()
{
    team().load();
    trainer->updateAll();
}

void TeamBuilder::newTeam()
{
    team() = TeamHolder();
    trainer->updateAll();
}

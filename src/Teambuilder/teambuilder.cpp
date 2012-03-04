#include "../Utilities/functions.h"
#include "teambuilder.h"
#include "trainermenu.h"
#include "teamholder.h"
#include "mainwindow.h"
#include "teammenu.h"

TeamBuilder::TeamBuilder(TeamHolder *team) : m_team(team), teamMenu(NULL)
{
    addWidget(trainer = new TrainerMenu(team));

    loadSettings(this, defaultSize());

    connect(trainer, SIGNAL(teamChanged()), SLOT(markTeamUpdated()));
    connect(trainer, SIGNAL(done()), SIGNAL(done()));
    connect(trainer, SIGNAL(editPoke(int)), SLOT(editPoke(int)));
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
    markAllUpdated();
    currentWidget()->updateAll();
}

void TeamBuilder::newTeam()
{
    team() = TeamHolder();
    markTeamUpdated();
    currentWidget()->updateTeam();
}

void TeamBuilder::markAllUpdated()
{
    for (int i = 0; i < count(); i++) {
        if (i != currentIndex()) {
            widget(i)->setProperty("all-to-update", true);
        }
    }
}

void TeamBuilder::markTeamUpdated()
{
    for (int i = 0; i < count(); i++) {
        if (i != currentIndex()) {
            widget(i)->setProperty("team-to-update", true);
        }
    }
}

void TeamBuilder::editPoke(int index)
{
    if (!teamMenu) {
        addWidget(teamMenu = new TeamMenu(&team(), index));
        connect(teamMenu, SIGNAL(teamChanged()), SLOT(markTeamUpdated()));
        connect(teamMenu, SIGNAL(switchToTrainer()), SLOT(switchToTrainer()));
    }

    switchTo(teamMenu);
}

void TeamBuilder::switchToTrainer()
{
    switchTo(trainer);
}

TeamBuilderWidget *TeamBuilder::currentWidget()
{
    return (TeamBuilderWidget*)(QStackedWidget::currentWidget());
}

TeamBuilderWidget *TeamBuilder::widget(int i)
{
    return (TeamBuilderWidget*)(QStackedWidget::widget(i));
}

void TeamBuilder::switchTo(TeamBuilderWidget *w)
{
    if (w->property("all-to-update").toBool()) {
        w->updateAll();
        w->setProperty("all-to-update", false);
        w->setProperty("team-to-update", false);
    } else if (w->property("team-to-update").toBool()) {
        w->updateTeam();
        w->setProperty("team-to-update", false);
    }
    setCurrentWidget(w);
}

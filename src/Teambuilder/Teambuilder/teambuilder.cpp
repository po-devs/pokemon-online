#include "../Utilities/functions.h"
#include "Teambuilder/teambuilder.h"
#include "Teambuilder/trainermenu.h"
#include "Teambuilder/teamholder.h"
#include "mainwindow.h"
#include "Teambuilder/teammenu.h"
#include "Teambuilder/poketablemodel.h"

TeamBuilder::TeamBuilder(TeamHolder *team) : m_team(team), teamMenu(NULL)
{
    addWidget(trainer = new TrainerMenu(team));
    pokemonModel = new PokeTableModel(team->team().gen(), this);

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

    /* Loading mod menu */
    QSettings s_mod(PoModLocalPath + "mods.ini", QSettings::IniFormat);
    QStringList mods = s_mod.childGroups();
    QActionGroup *modActionGroup = new QActionGroup(menuBar);
    if (mods.size() > 0) {
        int general_pos = mods.indexOf("General");
        if (general_pos != -1) {
            mods.removeAt(general_pos);
        }
        if (mods.size() > 0) {
            int mod_selected = s_mod.value("active", 0).toInt();
            bool is_mod_selected = mod_selected > 0;
            QMenu *menuMods = menuBar->addMenu(tr("&Mods"));

            // No mod option.
            QAction *action_no_mod = menuMods->addAction(tr("No mod"), this, SLOT(setNoMod()));
            action_no_mod->setCheckable(true);
            modActionGroup->addAction(action_no_mod);
            if (!is_mod_selected) action_no_mod->setChecked(true);
            menuMods->addSeparator();

            // Add mods to menu.
            QStringListIterator mods_it(mods);
            while (mods_it.hasNext()) {
                QString current = mods_it.next();
                QAction *ac = menuMods->addAction(current, this, SLOT(changeMod()));
                ac->setCheckable(true);
                if (is_mod_selected && (mod_selected == s_mod.value(current + "/id", 0).toInt())) {
                    ac->setChecked(true);
                }
                modActionGroup->addAction(ac);
            }
        }
    }

    w->addThemeMenu(menuBar);

    if (currentWidget()) {
        currentWidget()->addMenus(menuBar);
    }

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
    switchToTrainer();
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
        addWidget(teamMenu = new TeamMenu(pokemonModel, &team(), index));
        connect(teamMenu, SIGNAL(teamChanged()), SLOT(markTeamUpdated()));
        connect(teamMenu, SIGNAL(switchToTrainer()), SLOT(switchToTrainer()));
    }

    switchTo(teamMenu);

    teamMenu->switchToTab(index);
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

    emit reloadMenuBar();
}

//TODO
void TeamBuilder::setTierList(const QStringList &tiers)
{
    trainer->setTiers(tiers);
}

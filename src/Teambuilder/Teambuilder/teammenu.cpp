#include "../PokemonInfo/pokemonstructs.h"

#include <QStackedWidget>
#include <QTabBar>
#include <QGridLayout>
#include <QStringListModel>
#include <QMenuBar>
#include <QShortcut>
#include <QDockWidget>
#include <QMainWindow>

#include "../PokemonInfo/pokemoninfo.h"
#include "Teambuilder/teammenu.h"
#include "Teambuilder/pokeedit.h"
#include "Teambuilder/teamholder.h"
#include "Teambuilder/pokeboxes.h"
#include "teambuilder.h"

TeamMenu::TeamMenu(TeamBuilder *tb, QAbstractItemModel *pokeModel, TeamHolder *team, int index) :
    ui(new _ui()), m_team(team), lastGen(team->team().gen())
{
    setMainWindow(tb);
    setTeambuilder(tb);

    ui->pokemonModel = pokeModel;
    setupUi();
    updateTabs();

    if (0) {
        addDock(PokeEdit::EVDock, Qt::RightDockWidgetArea, new QDockWidget(tr("EVs"), this));
        addDock(PokeEdit::LevelDock, Qt::RightDockWidgetArea, new QDockWidget(tr("Level && Gender"), this));
        addDock(PokeEdit::MoveDock, Qt::BottomDockWidgetArea, new QDockWidget(tr("Moves"), this));
        addDock(PokeEdit::IVDock, Qt::BottomDockWidgetArea, new QDockWidget(tr("IVs, Ability && Hidden Power"), this));

        window->splitDockWidget(getDock(PokeEdit::EVDock), getDock(PokeEdit::LevelDock), Qt::Horizontal);

//        getDock(PokeEdit::MoveDock)->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
//        getDock(PokeEdit::EVDock)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
//        getDock(PokeEdit::IVDock)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    switchToTab(index);
}

void TeamMenu::addMenus(QMenuBar *b)
{
    QMenu *options = b->addMenu(tr("&Options"));
    QAction *adv = options->addAction(tr("&Advanced menu"), this, SLOT(toggleAdvanced()));
    adv->setCheckable(true);
    adv->setChecked(!PokeEdit::advancedWindowClosed);

    advancedMenu = adv;
}

void TeamMenu::setupUi()
{
    ui->boxes = NULL;
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(ui->stack = new QStackedWidget(), 1, 0, 1, 2);
    grid->addWidget(ui->pokemonTabs = new QTabBar(), 0, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop);
    grid->addWidget(ui->done = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_ArrowBack), tr("&Trainer Home")),
                    0, 1, 1, 1, Qt::AlignTop|Qt::AlignRight);
    ui->done->setObjectName("done");
    grid->setContentsMargins(-1,0,-1,-1);

    QSignalMapper *shortcutMapper = new QSignalMapper(this);
    for (int i = 0; i < 7; i++) {
        if (i < 6) {
            ui->pokemonTabs->addTab(PokemonInfo::Icon(i+1), tr("Slot #&%1").arg(i+1));
        } else {
            ui->pokemonTabs->addTab(QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon), tr("Boxes (#&%1)").arg(i+1));
        }
        QShortcut *shortcut = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(i+1)), this);
        connect(shortcut, SIGNAL(activated()), shortcutMapper, SLOT(map()));
        shortcutMapper->setMapping(shortcut, i);
    }
    connect(shortcutMapper, SIGNAL(mapped(int)), SLOT(switchToTab(int)));

    ui->pokemonTabs->setCurrentIndex(0);
    ui->pokemonTabs->setObjectName("pokemonTabs");

    QSettings s;
    QStringList itemList = s.value("TeamBuilder/ShowAllItems").toBool() ? ItemInfo::SortedNames(team().team().gen()) : ItemInfo::SortedUsefulNames(team().team().gen());
    ui->itemsModel = new QStringListModel(itemList, this);

    QStringList natures;
    for (int i = 0; i < NatureInfo::NumberOfNatures(); i++) {
        natures.push_back(NatureInfo::Name(i));
    }
    ui->natureModel = new QStringListModel(natures, this);

    connect(ui->pokemonTabs, SIGNAL(currentChanged(int)), SLOT(switchToTab(int)));
    connect(ui->done, SIGNAL(clicked()), SIGNAL(switchToTrainer()));
}

void TeamMenu::switchToTab(int index)
{
    if (ui->pokemonTabs->currentIndex() != index) {
        /* The signal/slot connection will call us again, thus we return */
        ui->pokemonTabs->setCurrentIndex(index);
        return;
    }
    createIndexIfNeeded(index);

    ui->stack->setCurrentWidget(widget(index));
}

void TeamMenu::createIndexIfNeeded(int index)
{
    if (index < 6) {
        if (!ui->pokemons.contains(index)) {
            ui->stack->addWidget(ui->pokemons[index]= new PokeEdit(this, &team().team().poke(index), ui->pokemonModel, ui->itemsModel, ui->natureModel));
            ui->pokemons[index]->setProperty("slot", index);
            connect(ui->pokemons[index], SIGNAL(numChanged()), SLOT(tabIconChanged()));
            connect(ui->pokemons[index], SIGNAL(nameChanged()), SIGNAL(teamChanged()));
            connect(ui->pokemons[index], SIGNAL(itemChanged()), SIGNAL(teamChanged()));
            connect(ui->pokemons[index], SIGNAL(closeAdvanced()), SLOT(closeAdvanced()));
        }
    } else if (index == 6) {
        if (!ui->boxes) {
            ui->stack->addWidget(ui->boxes = new PokeBoxes(this, &team()));
            connect(ui->boxes, SIGNAL(teamChanged()), SLOT(updatePokemons()));
            connect(ui->boxes, SIGNAL(teamChanged()), SIGNAL(teamChanged()));
        }
    }
}

QWidget *TeamMenu::widget(int index)
{
    return index < 6 ? (QWidget*)ui->pokemons[index] : (QWidget*)ui->boxes;
}

void TeamMenu::closeAdvanced()
{
    PokeEdit::advancedWindowClosed = true;
    foreach(PokeEdit *p, ui->pokemons) {
        p->closeAdvancedTab();
    }

    if (advancedMenu) {
        advancedMenu->setChecked(!PokeEdit::advancedWindowClosed);
    }
}

void TeamMenu::updatePokemons()
{
    foreach(PokeEdit *p, ui->pokemons) {
        p->setPoke(&team().team().poke(p->property("slot").toInt()));
    }
    for (int i = 0; i < 6; i++) {
        ui->pokemonTabs->setTabIcon(i, PokemonInfo::Icon(team().team().poke(i).num()));
    }
}

void TeamMenu::toggleAdvanced()
{
    if (!PokeEdit::advancedWindowClosed) {
        closeAdvanced();
    } else {
        PokeEdit::advancedWindowClosed = false;
        foreach(PokeEdit *p, ui->pokemons) {
            p->showAdvancedTab();
        }
    }
}

void TeamMenu::tabIconChanged()
{
    int slot = sender()->property("slot").toInt();

    ui->pokemonTabs->setTabIcon(slot, PokemonInfo::Icon(team().team().poke(slot).num()));
    emit teamChanged();
}

void TeamMenu::updateTeam()
{
    updatePokemons();

    if (lastGen != team().team().gen()) {
        lastGen = team().team().gen();
        updateItemModel();
    }

    if (ui->boxes) {
        ui->boxes->updateTeam();
    }

    updateTabs();
}

void TeamMenu::updateItemModel()
{
    /* Updating the item model causes a reset of the team, so...*/
    Team t = team().team();

    QSettings s;
    QStringList itemList = s.value("TeamBuilder/ShowAllItems").toBool() ? ItemInfo::SortedNames(team().team().gen()) : ItemInfo::SortedUsefulNames(team().team().gen());
    ui->itemsModel->setStringList(itemList);

    team().team() = t;

    foreach(PokeEdit *p, ui->pokemons) {
        p->setItem(team().team().poke(p->property("slot").toInt()).item());
    }
}

void TeamMenu::updateTabs()
{
    for (int i = 0; i < 6; i++) {
        ui->pokemonTabs->setTabIcon(i, PokemonInfo::Icon(team().team().poke(i).num()));
    }
}

TeamMenu::~TeamMenu()
{
    delete ui;
}

void TeamMenu::choosePokemon()
{
    ui->pokemons[ui->pokemonTabs->currentIndex()]->openPokemonSelection();
}

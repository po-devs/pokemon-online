#include "../PokemonInfo/pokemonstructs.h"

#include <QStackedWidget>
#include <QTabBar>
#include <QGridLayout>
#include <QStringListModel>
#include <QMenuBar>
#include <QShortcut>

#include "../PokemonInfo/pokemoninfo.h"
#include "Teambuilder/teammenu.h"
#include "Teambuilder/pokeedit.h"
#include "Teambuilder/teamholder.h"

TeamMenu::TeamMenu(QAbstractItemModel *pokeModel, TeamHolder *team, int index) :
    ui(new _ui()), m_team(team), lastGen(team->team().gen())
{
    ui->pokemonModel = pokeModel;
    setupUi();
    updateTabs();

    switchToTab(index);
}

void TeamMenu::setupUi()
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(ui->stack = new QStackedWidget(), 0, 0, 1, 1);
    grid->addWidget(ui->pokemonTabs = new QTabBar(), 0, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop);
    grid->setContentsMargins(-1,0,-1,-1);

    QSignalMapper *shortcutMapper = new QSignalMapper(this);
    for (int i = 0; i < 6; i++) {
        ui->pokemonTabs->addTab(PokemonInfo::Icon(i+1), tr("Slot #&%1").arg(i+1));
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
}

void TeamMenu::switchToTab(int index)
{
    if (ui->pokemonTabs->currentIndex() != index) {
        /* The signal/slot connection will call us again, thus we return */
        ui->pokemonTabs->setCurrentIndex(index);
        return;
    }
    if (!ui->pokemons.contains(index)) {
        ui->stack->addWidget(ui->pokemons[index]= new PokeEdit(&team().team().poke(index), ui->pokemonModel, ui->itemsModel, ui->natureModel));
        ui->pokemons[index]->setProperty("slot", index);
        connect(ui->pokemons[index], SIGNAL(switchToTrainer()), SIGNAL(switchToTrainer()));
        connect(ui->pokemons[index], SIGNAL(numChanged()), SLOT(tabIconChanged()));
        connect(ui->pokemons[index], SIGNAL(nameChanged()), SIGNAL(teamChanged()));
        connect(ui->pokemons[index], SIGNAL(itemChanged()), SIGNAL(teamChanged()));
    }
    ui->stack->setCurrentWidget(ui->pokemons[index]);
}

void TeamMenu::tabIconChanged()
{
    int slot = sender()->property("slot").toInt();

    ui->pokemonTabs->setTabIcon(slot, PokemonInfo::Icon(team().team().poke(slot).num()));
    emit teamChanged();
}

void TeamMenu::updateTeam()
{
    if (lastGen != team().team().gen()) {
        lastGen = team().team().gen();
        updateItemModel();
    }

    foreach(PokeEdit *p, ui->pokemons) {
        p->setPoke(&team().team().poke(p->property("slot").toInt()));
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

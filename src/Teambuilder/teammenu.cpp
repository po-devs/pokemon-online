#include <QStackedWidget>
#include <QTabBar>
#include <QGridLayout>
#include <QStringListModel>

#include "../PokemonInfo/pokemoninfo.h"
#include "teammenu.h"
#include "pokeedit.h"
#include "teamholder.h"

TeamMenu::TeamMenu(TeamHolder *team, int index) :
    ui(new _ui()), m_team(team)
{
    setupUi();
    updateTabs();

    switchToTab(index);
}

void TeamMenu::setupUi()
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(ui->stack = new QStackedWidget(), 0, 0, 1, 1);
    grid->addWidget(ui->pokemonTabs = new QTabBar(), 0, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop);

    for (int i = 0; i < 6; i++) {
        ui->pokemonTabs->addTab(PokemonInfo::Icon(i+1), tr("Slot #&%1").arg(i+1));
    }
    ui->pokemonTabs->setCurrentIndex(0);

    QSettings s;
    QStringList itemList = s.value("show_all_items").toBool() ? ItemInfo::SortedNames(team().team().gen()) : ItemInfo::SortedUsefulNames(team().team().gen());
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
        ui->stack->addWidget(ui->pokemons[index]= new PokeEdit(&team().team().poke(index), ui->itemsModel, ui->natureModel));
        connect(ui->pokemons[index], SIGNAL(switchToTrainer()), SIGNAL(switchToTrainer()));
    }
    ui->stack->setCurrentWidget(ui->pokemons[index]);
}

void TeamMenu::updateTeam()
{
    updateTabs();
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

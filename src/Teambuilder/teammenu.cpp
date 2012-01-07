#include <QStackedWidget>
#include <QTabBar>
#include <QGridLayout>

#include "../PokemonInfo/pokemoninfo.h"
#include "teammenu.h"


TeamMenu::TeamMenu(QWidget *parent) :
    TeamBuilderWidget(parent),
    ui(new _ui())
{
    setupUi();
}

void TeamMenu::setupUi()
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(ui->stack = new QStackedWidget(), 0, 0, 1, 1);
    grid->addWidget(ui->pokemonTabs = new QTabBar(), 0, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop);

    for (int i = 0; i < 6; i++) {
        ui->pokemonTabs->addTab(PokemonInfo::Icon(i+1), tr("Slot #&%1").arg(i+1));
    }
}

TeamMenu::~TeamMenu()
{
    delete ui;
}

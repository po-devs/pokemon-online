#include <QButtonGroup>

#include <PokemonInfo/pokemonstructs.h>
#include "Teambuilder/pokebuttonsholder.h"
#include "ui_pokebuttonsholder.h"

PokeButtonsHolder::PokeButtonsHolder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PokeButtonsHolder)
{
    ui->setupUi(this);

    pokemonButtons[0] = ui->pokemon0;
    pokemonButtons[1] = ui->pokemon1;
    pokemonButtons[2] = ui->pokemon2;
    pokemonButtons[3] = ui->pokemon3;
    pokemonButtons[4] = ui->pokemon4;
    pokemonButtons[5] = ui->pokemon5;

    pokemonButtons[0]->setChecked(true);

    group = new QButtonGroup(this);
    QSignalMapper *mapper = new QSignalMapper(this);
    QSignalMapper *map2 = new QSignalMapper(this);

    for (int i = 0; i < 6; i++) {
        pokemonButtons[i]->setNumber(i);
        group->addButton(pokemonButtons[i], i);
        mapper->setMapping(pokemonButtons[i], i);
        map2->setMapping(pokemonButtons[i], i);

        connect(pokemonButtons[i], SIGNAL(doubleClicked()), mapper, SLOT(map()));
        connect(pokemonButtons[i], SIGNAL(clicked()), map2, SLOT(map()));
        connect(pokemonButtons[i], SIGNAL(pokemonOrderChanged(int,int)), SIGNAL(teamChanged()));
        connect(pokemonButtons[i], SIGNAL(dropEventReceived(int,QDropEvent*)), SIGNAL(dropEvent(int,QDropEvent*)));
    }
    connect(mapper, SIGNAL(mapped(int)), SIGNAL(doubleClicked(int)));
    connect(map2, SIGNAL(mapped(int)), SIGNAL(clicked(int)));
}

PokeButtonsHolder::~PokeButtonsHolder()
{
    delete ui;
}

int PokeButtonsHolder::currentSlot() const
{
    return group->checkedId();
}

void PokeButtonsHolder::setCurrentSlot(int slot)
{
    group->button(slot)->setChecked(true);
}

void PokeButtonsHolder::setTeam(Team &team)
{
    m_team = &team;
    for (int i = 0; i < 6; i++) {
        pokemonButtons[i]->setPokemon(team.poke(i));
    }
}

void PokeButtonsHolder::updatePoke(int slot)
{
    pokemonButtons[slot]->updateAll();
}

#include <QButtonGroup>

#include "../PokemonInfo/pokemonstructs.h"
#include "pokebuttonsholder.h"
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

    QButtonGroup *group = new QButtonGroup(this);

    for (int i = 0; i < 6; i++) {
        pokemonButtons[i]->setNumber(i);
        group->addButton(pokemonButtons[i], i);
    }
}

PokeButtonsHolder::~PokeButtonsHolder()
{
    delete ui;
}

int PokeButtonsHolder::currentSlot() const
{
    return group->checkedId();
}

void PokeButtonsHolder::setTeam(Team &team)
{
    for (int i = 0; i < 6; i++) {
        pokemonButtons[i]->setPokemon(team.poke(i));
    }
}

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

    for (int i = 0; i < 6; i++) {
        pokemonButtons[i]->setNumber(i);
    }
}

PokeButtonsHolder::~PokeButtonsHolder()
{
    delete ui;
}

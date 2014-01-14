#include "pokedexpokeselection.h"
#include "ui_pokedexpokeselection.h"

PokedexPokeSelection::PokedexPokeSelection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PokedexPokeSelection)
{
    ui->setupUi(this);
}

PokedexPokeSelection::~PokedexPokeSelection()
{
    delete ui;
}

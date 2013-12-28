#include "../PokemonInfo/pokemonstructs.h"
#include "smogonsinglepokedialog.h"
#include "ui_smogonsinglepokedialog.h"
#include "pokemontab.h"

SmogonSinglePokeDialog::SmogonSinglePokeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SmogonSinglePokeDialog)
{
    ui->setupUi(this);
}

void SmogonSinglePokeDialog::setPokemon(PokeTeam *p)
{
    ui->scrollArea->setWidget(new PokemonTab(*p));
}

PokemonTab *SmogonSinglePokeDialog::getPokemonTab()
{
    return dynamic_cast<PokemonTab*> (ui->scrollArea->widget());
}

SmogonSinglePokeDialog::~SmogonSinglePokeDialog()
{
    delete ui;
}

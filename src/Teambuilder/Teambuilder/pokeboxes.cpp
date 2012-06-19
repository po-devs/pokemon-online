#include "pokeboxes.h"
#include "ui_pokeboxes.h"

PokeBoxes::PokeBoxes(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PokeBoxes)
{
    ui->setupUi(this);
}

PokeBoxes::~PokeBoxes()
{
    delete ui;
}

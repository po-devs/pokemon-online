#include "pokedex.h"
#include "ui_pokedex.h"

Pokedex::Pokedex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pokedex)
{
    ui->setupUi(this);
}

Pokedex::~Pokedex()
{
    delete ui;
}

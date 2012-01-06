#include "pokebutton.h"
#include "ui_pokebutton.h"

PokeButton::PokeButton(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::PokeButton)
{
    ui->setupUi(this);
}

PokeButton::~PokeButton()
{
    delete ui;
}

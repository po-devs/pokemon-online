#include "pokebutton.h"
#include "ui_pokebutton.h"

PokeButton::PokeButton(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::PokeButton)
{
    ui->setupUi(this);
    ui->number->setBuddy(this);
    layout()->setMargin(2);
}

PokeButton::~PokeButton()
{
    delete ui;
}

void PokeButton::setNumber(int x)
{
    ui->number->setText(tr("#&%1").arg(x));
}

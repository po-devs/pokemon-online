#include "pokebuttonsholder.h"
#include "ui_pokebuttonsholder.h"

PokeButtonsHolder::PokeButtonsHolder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PokeButtonsHolder)
{
    ui->setupUi(this);
}

PokeButtonsHolder::~PokeButtonsHolder()
{
    delete ui;
}

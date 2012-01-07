#include "pokeedit.h"
#include "ui_pokeedit.h"

PokeEdit::PokeEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PokeEdit)
{
    ui->setupUi(this);
}

PokeEdit::~PokeEdit()
{
    delete ui;
}

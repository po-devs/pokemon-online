#include "pokemoneditordialog.h"
#include "ui_pokemoneditordialog.h"

PokemonEditorDialog::PokemonEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PokemonEditorDialog)
{
    ui->setupUi(this);
}

PokemonEditorDialog::~PokemonEditorDialog()
{
    delete ui;
}

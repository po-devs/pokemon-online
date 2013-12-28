#include "TeambuilderLibrary/pokeselection.h"
#include "TeambuilderLibrary/poketablemodel.h"
#include "pokemoneditordialog.h"
#include "ui_pokemoneditordialog.h"

PokemonEditorDialog::PokemonEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PokemonEditorDialog), pokeModel(nullptr)
{
    ui->setupUi(this);
}

PokemonEditorDialog::~PokemonEditorDialog()
{
    delete ui;
}

void PokemonEditorDialog::on_pokemonFrame_clicked()
{
    if (!pokeModel) {
        pokeModel = new PokeTableModel();
        pokeModel->setParent(this);
    }

    PokeSelection *p= new PokeSelection(0, pokeModel);
    p->setParent(this, Qt::Popup);
    QPoint pos = ui->pokemonFrame->mapToGlobal(ui->pokemonFrame->pos());
    p->move(pos.x() + ui->pokemonFrame->width()+10, pos.y()-ui->pokemonFrame->height()/2);
    p->setAttribute(Qt::WA_DeleteOnClose, true);
    p->show();
}

#include "PokemonInfo/pokemoninfo.h"
#include "PokemonInfo/teamholderinterface.h"
#include "TeambuilderLibrary/pokeselection.h"
#include "TeambuilderLibrary/poketablemodel.h"
#include "TeambuilderLibrary/pokemovesmodel.h"
#include "TeambuilderLibrary/theme.h"
#include "Teambuilder/engineinterface.h"
#include "pokemoneditordialog.h"
#include "ui_pokemoneditordialog.h"

PokemonEditorDialog::PokemonEditorDialog(MainEngineInterface *client) :
    ui(new Ui::PokemonEditorDialog), pokeModel(nullptr), movesModel(nullptr)
{
    ui->setupUi(this);

    setPokemon(client->trainerTeam()->team().poke(0).num());
}

PokemonEditorDialog::~PokemonEditorDialog()
{
    delete ui;
}

void PokemonEditorDialog::setPokemon(Pokemon::uniqueId id)
{
    current = id;

    ui->pokemonSprite->setPixmap(PokemonInfo::Picture(current));

    if (!movesModel) {
        movesModel = new PokeMovesModel(id);
        movesModel->setParent(this);

        ui->moveChoice->setModel(movesModel);

        /* Snippet stolen from src/Teambuilder/Teambuilder/pokeedit.cpp */
        ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Name, 125);
        ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Type, Theme::TypePicture(Type::Normal).width()+5);
        ui->moveChoice->setIconSize(Theme::TypePicture(Type::Normal).size());

        ui->moveChoice->sortByColumn(PokeMovesModel::Name, Qt::AscendingOrder);
        /* End snippet */
    } else {
        movesModel->setPokemon(id);
    }
}

void PokemonEditorDialog::on_pokemonFrame_clicked()
{
    if (!pokeModel) {
        pokeModel = new PokeTableModel();
        pokeModel->setParent(this);
    }

    PokeSelection *p= new PokeSelection(current, pokeModel);
    p->setParent(this, Qt::Popup);
    QPoint pos = ui->pokemonFrame->mapToGlobal(ui->pokemonFrame->pos());
    p->move(pos.x() + ui->pokemonFrame->width()+10, pos.y()-ui->pokemonFrame->height()/2);
    p->setAttribute(Qt::WA_DeleteOnClose, true);
    p->show();

    connect(p, SIGNAL(pokemonChosen(Pokemon::uniqueId)), SLOT(setPokemon(Pokemon::uniqueId)));
}

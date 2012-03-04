#include "pokeselection.h"
#include "ui_pokeselection.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "modelenum.h"
#include "theme.h"

PokeSelection::PokeSelection(Pokemon::uniqueId pokemon, QAbstractItemModel *pokemonModel) :
    ui(new Ui::PokeSelection)
{
    ui->setupUi(this);
    ui->pokemonList->setModel(pokemonModel);

    m_num = pokemon;

    ui->pokemonList->setCurrentIndex(pokemonModel->index(pokemon.pokenum, 1));
    ui->pokemonList->scrollTo(ui->pokemonList->currentIndex());

    updateSprite();
    updateTypes();

    connect(ui->shiny, SIGNAL(toggled(bool)), SLOT(updateSprite()));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(updateSprite()));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(updateTypes()));
    connect(ui->pokemonList, SIGNAL(pokemonActivated(Pokemon::uniqueId)), SLOT(finish()));
    connect(ui->done, SIGNAL(clicked()), SLOT(finish()));
}

void PokeSelection::setNum(const Pokemon::uniqueId &num)
{
    m_num = num;
}

void PokeSelection::finish()
{
    emit pokemonChosen(num());
    close();
}

Pokemon::gen PokeSelection::getGen()
{
    return ui->pokemonList->currentIndex().data(CustomModel::PokegenRole).value<Pokemon::gen>();
}

void PokeSelection::updateSprite()
{
    Pokemon::uniqueId _num = num();
    bool shiny = ui->shiny->isChecked();

    ui->pokemonSprite->setPixmap(PokemonInfo::Picture(_num, getGen(), Pokemon::Male, shiny));
}

void PokeSelection::updateTypes()
{
    Pokemon::uniqueId _num = num();
    int type1 = PokemonInfo::Type1(_num, getGen());
    int type2 = PokemonInfo::Type2(_num, getGen());

    ui->type1->setPixmap(Theme::TypePicture(type1));

    if (type2 != Pokemon::Curse) {
        ui->type2->setPixmap(Theme::TypePicture(type2));
        ui->type2->setVisible(true);
    } else {
        ui->type2->setVisible(false);
    }
}

PokeSelection::~PokeSelection()
{
    delete ui;
}

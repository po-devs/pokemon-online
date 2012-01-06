#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"

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
    ui->number->setText(tr("#&%1").arg(x+1));
    setAccessibleName(tr("Pokemon slot %1").arg(x+1));
}

void PokeButton::setPokemon(PokeTeam &poke)
{
    ui->level->setText(tr("Lv. %1").arg(poke.level()));
    ui->item->setPixmap(ItemInfo::Icon(poke.item()));
    ui->species->setText(PokemonInfo::Name(poke.num()));
    ui->nickname->setText(poke.nickname().isEmpty() ? ui->species->text() : poke.nickname());
    ui->sprite->setPixmap(poke.picture());
}

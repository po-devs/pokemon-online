#include "pokeboxitem.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"

PokeBoxItem::PokeBoxItem(PokeTeam *poke, PokeBox *box) : poke(NULL), m_Box(box)
{
    changePoke(poke);
    setFlags(QGraphicsItem::ItemIsMovable);
}

void PokeBoxItem::changePoke(PokeTeam *poke)
{
    delete this->poke;
    this->poke = poke;
    setPixmap(PokemonInfo::Icon(poke->num()));
}

void PokeBoxItem::setBox(PokeBox *newBox)
{
    m_Box = newBox;
}

PokeBoxItem::~PokeBoxItem()
{
    delete poke;
    poke = NULL;
}

void PokeBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    (void) event;
}

void PokeBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    (void) event;
}

void PokeBoxItem::startDrag()
{
}

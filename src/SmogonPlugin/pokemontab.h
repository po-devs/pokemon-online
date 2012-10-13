#ifndef POKEMONTAB_H
#define POKEMONTAB_H

#include "../PokemonInfo/pokemoninfo.h"
#include <QLabel>
#include <QDialog>

class PokemonTab : public QWidget{
public:
    PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent);

};

#endif // POKEMONTAB_H

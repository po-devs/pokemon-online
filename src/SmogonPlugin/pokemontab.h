#ifndef POKEMONTAB_H
#define POKEMONTAB_H

#include "../PokemonInfo/pokemoninfo.h"
#include <QLabel>
#include <QDialog>
#include <QtGui>

class PokemonTab : public QWidget{
private:
    PokeTeam originalPokemon;
    QComboBox* build_chooser;
    QComboBox* item_chooser;
    QComboBox* ability_chooser;
    QComboBox* nature_chooser;
    QComboBox* ev_chooser;
    QComboBox* move1_chooser;
    QComboBox* move2_chooser;
    QComboBox* move3_chooser;
    QComboBox* move4_chooser;
    QLabel* description;
public:
    PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent);
    PokeTeam *getPokeTeam();
    void updateUI();
};

#endif // POKEMONTAB_H

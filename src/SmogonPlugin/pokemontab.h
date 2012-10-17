#ifndef POKEMONTAB_H
#define POKEMONTAB_H

#include <QLabel>
#include <QDialog>
#include <QtGui>
#include "SmogonBuild.h"
#include "../PokemonInfo/pokemoninfo.h"



class PokemonTab : public QWidget{
public:
    PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent);
    void update_ui(SmogonBuild *builds);
private:
    QVBoxLayout *mainLayout;
};

#endif // POKEMONTAB_H

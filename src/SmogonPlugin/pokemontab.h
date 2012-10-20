#ifndef POKEMONTAB_H
#define POKEMONTAB_H

#include <QLabel>
#include <QDialog>
#include <QtGui>
#include "SmogonBuild.h"
#include "../PokemonInfo/pokemoninfo.h"


class PokemonTab : public QWidget{
    Q_OBJECT
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
    QList<SmogonBuild>* allBuilds;
public:
    PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent);
    ~PokemonTab();
    PokeTeam *getPokeTeam();
    void createInitialUi(QList<SmogonBuild>* builds);
public slots:    
    void updateUI();
};

#endif // POKEMONTAB_H

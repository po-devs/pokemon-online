#ifndef POKEMONTEAMTABS_H
#define POKEMONTEAMTABS_H

#include "../PokemonInfo/pokemoninfo.h"
#include <QLabel>
#include <QDialog>
#include <QTabWidget>

class PokemonTeamTabs : public QTabWidget{
public:
    PokemonTeamTabs();
    Team getTeam();
};

#endif // POKEMONTEAMTABS_H

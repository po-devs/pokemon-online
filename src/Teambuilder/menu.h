#ifndef TEAMBUILDER_MENU_H
#define TEAMBUILDER_MENU_H

#include "teambuilder.h"
//for Team struct
#include "../PokemonInfo/pokemoninfo.h"
// :)
#include <QtGui>

class TB_Menu : public QWidget
{
	Q_OBJECT
private:
    Team myteam;

public:
    TB_Menu();

public slots:
    void launchTeambuilder();
    void launchCredits();
    void goOnline();
};

#endif

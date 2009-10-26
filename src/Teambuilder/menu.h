#ifndef TEAMBUILDER_MENU_H
#define TEAMBUILDER_MENU_H

#include "teambuilder.h"
#include "otherwidgets.h"
//for Team struct
#include "../PokemonInfo/pokemoninfo.h"
// :)
#include <QtGui>

class TB_Menu : public QCenteredWidget
{
	Q_OBJECT
private:
    Team m_Team;

    Team * team();

public:
    TB_Menu();

public slots:
    void launchTeambuilder();
    void launchCredits();
    void goOnline();
};

#endif

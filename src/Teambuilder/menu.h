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
    TrainerTeam m_Team;

    Team * team();
    TrainerTeam * trainerTeam();

public:
    TB_Menu();

public slots:
    void launchTeambuilder();
    void launchCredits();
    void goOnline();
};

#endif

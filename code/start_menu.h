#ifndef START_MENU_H_INCLUDED
#define START_MENU_H_INCLUDED

#include "Team_Struct.h"
#include "MF.hh"

class MF_ImHLApplet;

//So simple!!
class StartMenu : public MF_BDirections, public MF_Prio
{
    public:
        Team &equipe;
        StartMenu(Team &equipe);

        MF_ImHLApplet *teambuilder, *battle, *credits, *exit;

        bool recoitMessage(const char * message, MF_Base *fenetre);
        void affiche(Surface &s);
};

#endif // START_MENU_H_INCLUDED

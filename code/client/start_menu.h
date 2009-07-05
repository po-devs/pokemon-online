#ifndef START_MENU_H_INCLUDED
#define START_MENU_H_INCLUDED

#include "Team_Struct.h"
#include "../generic/MF.hh"

class MF_ImHLApplet;

//So simple!!
class StartMenu : public MF_BDirections, public MF_Prio
{
    public:
        Team &team;
        StartMenu(Team &team);

        MF_ImHLApplet *teambuilder, *battle, *credits, *exit;

        bool RecvFromSub(const char * message, MF_Base *fenetre);
        void display(Surface &s);
};

#endif // START_MENU_H_INCLUDED

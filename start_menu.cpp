#include "start_menu.h"
#include "MF_applet.hh"

StartMenu::StartMenu(Team &equipe) :equipe(equipe)
{
    allouer (teambuilder = new MF_ImHLApplet("teambuilder.png", 140, 130));
    allouer (battle = new MF_ImHLApplet("battle online.png", 140, 220));
    allouer (credits = new MF_ImHLApplet("credits.png", 230, 310));
    allouer (exit = new MF_ImHLApplet("exit.png", 250, 400));
}

void StartMenu::affiche(Surface &s)
{
    updated = true;
    s.fill(0, Color(0xFF, 0xFF, 0xFF));
    afficheMF(s);
}

bool StartMenu::recoitMessage(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "clic") == 0)
    {
        if (fenetre == teambuilder)
        {
            envoieMessage("teambuilder");
            return true;
        }
        if (fenetre == exit)
        {
            envoieMessage("exit");
            return true;
        }
        if (fenetre == credits)
        {
            envoieMessage("credits");
            return true;
        }
        if (fenetre == battle)
        {
            envoieMessage("battle online");
            return true;
        }
    }
    return false;
}

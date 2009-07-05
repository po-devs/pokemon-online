#include "start_menu.h"
#include "../generic/MF_applet.hh"

StartMenu::StartMenu(Team &team) :team(team)
{
    allocate (teambuilder = new MF_ImHLApplet("teambuilder.png", 140, 130));
    allocate (battle = new MF_ImHLApplet("battle online.png", 140, 220));
    allocate (credits = new MF_ImHLApplet("credits.png", 230, 310));
    allocate (exit = new MF_ImHLApplet("exit.png", 250, 400));
}

void StartMenu::display(Surface &s)
{
    updated = true;
    s.fill(0, Color(0xFF, 0xFF, 0xFF));
    displayMF(s);
}

bool StartMenu::RecvFromSub(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "clic") == 0)
    {
        if (fenetre == teambuilder)
        {
            sendToBoss("teambuilder");
            return true;
        }
        if (fenetre == exit)
        {
            sendToBoss("exit");
            return true;
        }
        if (fenetre == credits)
        {
            sendToBoss("credits");
            return true;
        }
        if (fenetre == battle)
        {
            sendToBoss("battle online");
            return true;
        }
    }
    return false;
}

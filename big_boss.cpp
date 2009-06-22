#include "big_boss.h"
#include "teambuilder.h"
#include "start_menu.h"
#include "credits.h"
#include "TheInternet.h"

BigBoss::BigBoss()
{
    allouer(new StartMenu(equipe));
}

bool BigBoss::recoitMessage(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "teambuilder") == 0) {
        allouer(new TeamBuilder(equipe));
        goto changed;
    }
    if (strcmp(message, "menu") == 0) {
        allouer(new StartMenu(equipe));
        goto changed;
    }
    if (strcmp(message, "exit") == 0) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
        goto changed;
    }
    if (strcmp(message, "credits") == 0)
    {
        allouer(new Credits());
        goto changed;
    }
    if (strcmp(message, "battle online") == 0)
    {
        allouer(new ServerList());
        goto changed;
    }
    //more will be added

    return false;

changed:
    detruireMF(fenetre);
    return true;
}

void BigBoss::affiche(Window &ecran)
{
    if (!updated)
    {
        updated = true;
        afficheMF(ecran);
    } else if (pDebut->check_updated() == false)
    {
        pDebut->affiche(ecran);
    } else
    {
        return ;
    }
    ecran.flip();
}

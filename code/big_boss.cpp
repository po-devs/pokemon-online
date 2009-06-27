#include "big_boss.h"
#include "teambuilder.h"
#include "start_menu.h"
#include "credits.h"
#include "TheInternet.h"

BigBoss::BigBoss()
{
    allocate(new StartMenu(team));
}

bool BigBoss::RecvFromSub(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "teambuilder") == 0) {
        allocate(new TeamBuilder(team));
        goto changed;
    }
    if (strcmp(message, "menu") == 0) {
        allocate(new StartMenu(team));
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
        allocate(new Credits());
        goto changed;
    }
    if (strcmp(message, "battle online") == 0)
    {
        allocate(new ServerList());
        goto changed;
    }
    //more will be added

    return false;

changed:
    destroyMF(fenetre);
    return true;
}

void BigBoss::display(Window &ecran)
{
    if (!updated)
    {
        updated = true;
        displayMF(ecran);
    } else if (pStart->check_updated() == false)
    {
        pStart->display(ecran);
    } else
    {
        return ;
    }
    ecran.flip();
}

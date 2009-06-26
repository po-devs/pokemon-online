#include <SDL/SDL_ttf.h>
#include "big_boss.h"
#include "MF_text.hh"

using namespace std;

int main(int argc, char **argv) try
{
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    srand(time(NULL));

    //freopen("CON", "w", stdout);

    SDL_WM_SetCaption("TeamBuilder", NULL);
    SDL_WM_SetIcon(SDL_LoadBMP("db/icon.bmp"), NULL);

    SDL_EnableUNICODE(SDL_ENABLE);
    Window screen (32,649,609,SDL_DOUBLEBUF|SDL_HWSURFACE);

    BigBoss *tb;
    tb = new BigBoss();

    while (1)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0)
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    goto end;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_F4 && ALT_ON || event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        goto end;
                    }
                default:
                    tb->gereEvenement(event);
                    break;
            }
        }

        //if(SDL_GetAppState() & SDL_APPACTIVE)
        {
            tb->affiche(screen);
        }
        SDL_Delay(1);
    }

    end:

    delete tb;

    TTF_Quit();
    SDL_Quit();

	exit(0);
}  catch (exception &ex)
{
    cout << "Exception: " << ex.what();
    return 0;
} catch (int &i)
{
    cout << "Exception int: " << i << endl;
    return 0;
} catch (...)
{
    cout << "Exception recue!" << endl;
    return -1;
}

#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include "big_boss.h"

using namespace std;

int main(int argc, char **argv) try
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();
    TTF_Init();
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    srand(time(NULL));

    freopen("client_stdout.txt", "w", stdout);

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
                    tb->deal_w_Event(event);
                    break;
            }
        }

        if(SDL_GetAppState() & SDL_APPACTIVE)
        {
            tb->display(screen);
        }
        SDL_Delay(1);
    }

    end:

    delete tb;
    delete &FontMan;
    delete &ImageMan;

    TTF_Quit();
    SDLNet_Quit();
    SDL_Quit();

	return(0);
}  catch (const exception &ex)
{
    cout << "Exception: " << ex.what();
    return 0;
} catch (const int &i)
{
    cout << "Exception int: " << i << endl;
    return 0;
} catch (const string &s)
{
    cout << "Exception string: " << s << endl;
    return 0;
} catch (const char *a)
{
    cout << "Exception char*: " << a << endl;
    return 0;
} catch (...)
{
    cout << "Exception inconnue!" << endl;
    return 0;
}

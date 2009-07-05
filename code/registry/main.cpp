#include <vector>
#include <string>
#include <map>
#include "serverdata.h"

using namespace std;
using namespace interface;

const uint8_t pcount_limit = 4;
const uint8_t scount_limit = 1;

/* We store the servers here */
ServList valid_list; /* servers which have specified their info  */
ServList invalid_list; /* when servers haven't specified yet their name & desc & num of players */
uint32_t vlist_size = 0;
uint32_t ilist_size = 0;

/* They have a mutex inside activated for Add, Remove and Clear, and
    that you should activate during the whole Wait() / GetSocketReady() loop */
SafeSelector<PlaySocket> *play_select;
SafeSelector<ServSocket> *serv_select;

/* Uint32 is the IP converted to an integer
    count is the number of times the IP is connected */
map<uint32_t, Uint8in> serv_count; //servers
map<uint32_t, Uint8in> play_count; //regular players

/* Threads that terminate and that for some reason can't manage
   to be Waited will be stored here and be Waited the next time
   this function is called (don't worry there is a Mutex) */
void dispose_thread(Thread *t)
{
    static Thread *local_thread = NULL;
    static Mutex local_mutex;

    local_mutex.Lock();
    delete local_thread;
    local_thread = t;
    local_mutex.Unlock();
}

Mutex scount_lock;
Mutex pcount_lock;

void dec_scount(uint32_t key)
{
    scount_lock.Lock();
    if ((--serv_count[key]) == 0)
        serv_count.erase(key);
    scount_lock.Unlock();
}

/* Waiting for a better SFML that lists IP! */
void dec_pcount(uint32_t key)
{
    pcount_lock.Lock();
    if ((--play_count[key]) == 0)
        play_count.erase(key);
    pcount_lock.Unlock();
}

/* Waiting for a better SFML that lists IP! */
uint8_t inc_pcount(uint32_t key)
{
    pcount_lock.Lock();
    uint8_t ret = play_count[key];
    pcount_lock.Unlock();
    return ret;
}
uint8_t inc_scount(uint32_t key)
{
    scount_lock.Lock();
    uint8_t ret = serv_count[key];
    scount_lock.Unlock();
    return ret;
}

uint32_t num_servers()
{
    valid_list.Lock();
    invalid_list.Lock();
    uint32_t ret = vlist_size + ilist_size;
    invalid_list.Unlock();
    valid_list.Unlock();

    return ret;
}

/* TODO: antiflood */

/*This code is produced supposing that
  ONLY THE MAIN THREAD accepts new connections */

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();

 /* Données test */
    ServSocket client;
    valid_list.push_back(new ServerData(client, 508, "Blue Heaven", "Manga rocks"));
    vlist_size += 1;

    {
        SDL_SetVideoMode(600, 600, 16, SDL_SWSURFACE);

        freopen("registry_stdout.txt", "w", stdout);

        //The server socket
        TCPSocket serv_listener;
        TCPSocket play_listener;

        //The selectors
        SafeSelector<PlaySocket> _play_select;
        SafeSelector<ServSocket> _serv_select;

        play_select = &_play_select;
        serv_select = &_serv_select;

        if (serv_listener->Listen(50507) != Socket::Done)
        {
            cout << "Error: impossible to listen on port 50507" << endl;
            goto end;
        }

        cout << "Listening on port 50507" << endl;

        if (play_listener->Listen(50508)  != Socket::Done)
        {
            cout << "Error: impossible to listen on port 50508" << endl;
            goto end;
        }

        cout << "Listening on port 50508" << endl;

        //play_listener->SetBlocking(false);
        serv_listener->SetBlocking(false);

        while (1)
        {
            //Si il n'y a pas d'activité, on économisera le CPU avec sf::Sleep
            bool activity = false;

            PlaySocket pconnected;
            ServSocket sconnected;

            Socket::Status s;

            /** En premier lieu: l'acceptation de sockets **/

            if ( (s=play_listener->Accept(pconnected)) == Socket::Done)
            {
                cout << "Socket joueur acceptee!" << endl;

                activity = true;
                uint32_t ref = (pconnected->GetIp()).ToInteger();

                if (inc_pcount(ref) > pcount_limit)
                {
                    //On disconnecte! : trop de gens avec la même IP
                    pconnected->Close();
                    //On diminue le play_count, du coup
                    dec_pcount(ref);
                } else
                {
                    pconnected.create_var(NULL);
                    pconnected->SetBlocking(false);
                    TRACE(play_select->Add(pconnected));
                }
            }

            if (serv_listener->Accept(sconnected) == Socket::Done)
            {
                cout << "Socket serveur acceptee!" << endl;

                activity = true;
                uint32_t ref = sconnected->GetIp().ToInteger();

                if (inc_scount(ref) > scount_limit)
                {
                    //On disconnecte! : trop de gens avec la même IP
                    sconnected->Close();
                    dec_scount(ref);
                } else
                {
                    sconnected->SetBlocking(false);

                    if (num_servers() >= uint16_t(-1)) {
                        cout << "Trop de serveurs connectés: " << uint16_t(-1) << endl;
                        sconnected->Close();
                        dec_scount(ref);
                    } else {
                        invalid_list.Lock();
                        invalid_list.push_back(new ServerData(sconnected));
                        sconnected.change_var(--invalid_list.end());
                        ilist_size += 1;
                        invalid_list.Unlock();

                        serv_select->Add(sconnected);
                    }
                }
            }

            /** En deuxième lieu: gérer les sockets existantes **/

            /* côté joueur */
            play_select->Lock();

            size_t num = play_select->Wait(0);

            if (num) {
                activity = true;
            }

            /* Get the ready socks :) */
            while (num > 0)
            {
                num -= 1;
                PlaySocket s = play_select->GetSocketReady(num);
                if (*s.get_var() == NULL) //si elle n'est pas déjà threadée
                {
                    s.change_var(new Thread(play_io, &s));
                }
            }
            play_select->Unlock();

            /*côté serveur */
            serv_select->Lock();

            num = serv_select->Wait(0);
            /* Get the ready socks :) */
            while (num > 0)
            {
                num -= 1;
                ServSocket s = serv_select->GetSocketReady(num);
                ServerData *sdata = **s.get_var();
                sdata->create_thread(serv_io); // gère automatiquement même si il y a déjà un thread
            }
            serv_select->Unlock();

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                    goto end;
            }

            if (!activity) {
                SDL_Delay(10);
            }
        }
    }
end:
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}

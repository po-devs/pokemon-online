#include "TheInternet.h"
#include <iostream>
#include <string>
#include "508socket.h"
#include "utilities.hh"

using namespace std;
using namespace _508::sock;

ConnectToRegistry::ConnectToRegistry()
                  : super(NULL)
{
    ;
}

ConnectToRegistry::~ConnectToRegistry()
{
    if (super)
    {
        finish_off = true;
        SDL_WaitThread(super, NULL);
    }
}

void ConnectToRegistry::start_everything()
{
    super = SDL_CreateThread(the_real_thing, this);
}

int ConnectToRegistry::the_real_thing(void *data)
{
    //classe de base
    ConnectToRegistry &base = *(static_cast<ConnectToRegistry*>(data));

    //on prend l'addresse de la registry
    ifstream in("db/registry.txt");

    string regip;
    getline(in, regip);
    in.close();

    sockboss nosy(1);
    socket thesock;

    if (!nosy.getsock(nosy.createsocket(regip.c_str(), 6508), thesock))
    {
        base.envoieMessage("error: Error -- give the programmer the code C01 for help");
        return 1;
    }

    if (!(*thesock))
    {
        base.envoieMessage(("error: Impossible to connect to the registry(" + regip +"):\n \n"+nosy.error).c_str());
        return 1;
    }

    if (base.finish_off)
    {
        return 0;
    }

    string msg = "__servlist";
    msg[0] = 1;
    msg[1] = msg.length()-2;

    thesock->send(msg);

    if (base.finish_off)
    {
        return 0;
    }

    string recv;

    while (!base.finish_off && *thesock)
    {
        nosy.checksockets();
        if (thesock->ready())
        {
            //First we need to know the length of the length of the message
            thesock->recv(recv, 1);
            break;
        } else
        {
            SDL_Delay(10);
        }
    }

    if (base.finish_off || !*thesock)
    {
        return 0;
    }

    // Now we need to know the length of the message
    Uint8 l_l = recv[0];
    recv = "";

    while (!base.finish_off && recv.length() < l_l && *thesock)
    {
        nosy.checksockets();
        if (thesock->ready())
        {
            //First we need to know the length of the length of the message
            thesock->recv(recv, recv.length()-l_l);
            break;
        } else
        {
            SDL_Delay(10);
        }
    }

    if (base.finish_off || !*thesock)
    {
        return 0;
    }

    //And now we need the whole message
    size_t l = calc_length(recv.data(), l_l);
    //And now, download time :D

    recv= "";
    while (!base.finish_off && recv.length() < l && *thesock)
    {
        nosy.checksockets();
        if (thesock->ready())
        {
            //First we need to know the length of the length of the message
            thesock->recv(recv, recv.length()-l);
            break;
        } else
        {
            SDL_Delay(10);
        }
    }

    if (base.finish_off || !*thesock)
    {
        return 0;
    }

    //Now everything is in the right string!
    //time for heavy weaponry
    MegaDeserializer pile(recv);
    return 0;
}

size_t ConnectToRegistry::calc_length(const char *data, Uint8 l_l)
{
    size_t res = 0;
    while (l_l > 0)
    {
        res *= 256;
        res += (Uint8)*data;
        --l_l;
        ++data;
    }
    return res;
}

ServerList::ServerList()
           :MF_Applet(462, 521, Color(84, 109, 142))
{
    allouer(a = new MF_Button("a2.png", "a_pressed.png"));
    allouer(b = new MF_Button("b.png", "b_pressed.png"));
    allouer(start = new MF_Button("start.png", "start_pressed.png"));
    allouer(left = new MF_Button("left.png", "left_pressed.png"));
    allouer(right = new MF_Button("right.png", "right_pressed.png"));
    allouer(down = new MF_Button("down.png", "down_pressed.png"));
    allouer(up = new MF_Button("up.png", "up_pressed.png"));
    allouer(servers = new MF_BarManager(387,300,35,28,"verdana.ttf",11));

    //POSITIONNEMENT
    a->move(259,377);
    b->move(369,379);
    start->move(191,411);
    left->move(22,397);
    right->move(102,397);
    up->move(62, 357);
    down->move(62,437);

    move((SDL_GetVideoSurface()->w-dims.w)/2, (SDL_GetVideoSurface()->h-dims.h)/2);

    police = servers->police;
    size_t ls = police.line_skip();

    //Initialisation
    servers->add_set<MF_BigASet>();
    servers->add_type<MF_TextBar>(105,ls+2);
    servers->add_type<MF_TextBar>(205,ls+2);
    servers->add_type<MF_TextBar>(50,ls+2);

    //Special connection
    c.pSup = this;
    c.start_everything();
}

void ServerList::affiche(Surface &ecran)
{
    if (!updated)
    {
        updated = true;
        self_affiche(ecran);
        afficheMF(ecran);
    } else if (pDebut->check_updated() == false)
    {
        pDebut->affiche(ecran);
    }
}

void ServerList::self_affiche(Surface &ecran)
{
    ecran.fill(0, Color(255,255,255));
    MF_Applet::affiche(ecran);
}

bool ServerList::recoitMessage(const char *message, MF_Base *fenetre)
{
    if (fenetre == b and strcmp(message, "release") == 0)
    {
        envoieMessage("menu");
        return true;
    }

    if (fenetre == &c)
    {
        if (strncmp(message, "error: ", strlen("error: ")) == 0)
        {
            allouer ( new MF_Alert(this, police, message + strlen("error: "), bgColor) );
            return true;
        }
    }
    return false;
}

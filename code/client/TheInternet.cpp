#include <iostream>
#include <string>
#include <vector>
#include "../generic/intersock.hpp"
#include "../generic/internet_utilities.hh"
#include "TheInternet.h"

using namespace std;
using namespace interface;
using _508::recv_body;
using _508::send_body;

/* Simple uh? */
bool deserialize(MegaDeserializer &des, ServerData &s)
{
    char scheme[] = {'#', '0'+32,'#', '0'+10, '$', '5', '$', '9', 0};
    return des.mega_pop(scheme, &s.refip, &s.num_players, &s.name, &s.description);
}

void express_data(const char *data, size_t len)
{
    for (int i = 0; i < len; i++)
        cout << int (data[i]) << " ";

    cout << endl;
}

ConnectToRegistry::ConnectToRegistry()
                  :finish_off(false)
{
    ;
}

ConnectToRegistry::~ConnectToRegistry()
{
    finish_off = true;
    thread.Wait();
}

void ConnectToRegistry::start_everything()
{
    thread.Launch(&ConnectToRegistry::the_real_thing, this);
}


int ConnectToRegistry::the_real_thing(void *data)
{
    //classe de base
    ConnectToRegistry &base = *(static_cast<ConnectToRegistry*>(data));
    TCPSocket Client;
    string ex; //exception

    //on prend l'addresse de la registry
    string regip = *find_line("db/registry.txt", 0);

    Client->SetBlocking(false);

    string msg,recv;
    Socket::Status s;
    MegaDeserializer pile;

    IPAddress ip (regip.c_str(), 50508);

    if (!ip) {
        ex = "error: impossible to resolve ip " + regip + ".\n \n" + SDL_GetError();
    }

    while ( (s=Client->Connect(ip)) == Socket::NotReady && !base.finish_off) {
        ;
    }

    if (base.finish_off)
        goto end;

    if (s != Socket::Done)
    {
        ex = "error: Error while connecting to registry (" + regip + ").\n \n" + SDL_GetError();
        goto ex;
    }

    s = send_body(Client, string("servlist"), base.finish_off);

    if (s == Socket::Error) {
        ex = string() + "error: Error sending request registry for server list.\n \n" + SDL_GetError();
        goto ex;
    }

    //receives the body of the answer from registry
    s = recv_body(Client, recv, base.finish_off);

    if (s == Socket::NotReady)
        goto end;

    if (s == Socket::Error) {
        ex = string() + "error: Error when receiving server list.\n \n" + SDL_GetError();
        goto ex;
    }


    {
        cout << "taille reçue: " << recv.length() << endl;
        /** Now everything is in the right string!
        time for heavy weaponry **/
        pile.init(recv);

        size_t numservers;
        if (!pile.pop_l(numservers, 16)) {
            ex = "error: Invalid or corrupted communication";
            goto ex;
        } else {
            vector<ServerData> v;
            v.reserve(numservers);

            cout << "number of servers: " << numservers << endl;

            while (numservers > 0) {
                ServerData s;
                if (!deserialize(pile, s)) {
                    ex = "error: Invalid or corrupted communication \n \n\
Either registry is a bad one, there is a bug, \nyou are not up to date or someone is hacking you";
                    goto ex;
                }
                v.push_back(s);
                cout << "numplayers: " << s.num_players << "..." << endl;
                numservers -= 1;
            }

            /* Alright, now lets push that up */
            ServerList * sl = dynamic_cast<ServerList*> (base.pSup);
            sl->set_serverlist(v);
        }
    }


end:
    Client->Close();
    return 0;
ex:
    Client->Close();
    base.sendToBoss(ex.c_str());
    return 0;
}

ServerList::ServerList()
           :MF_Applet(462, 521, Color(84, 109, 142))
{
    allocate(a = new MF_Button("a2.png", "a_pressed.png"));
    allocate(b = new MF_Button("b.png", "b_pressed.png"));
    allocate(start = new MF_Button("start.png", "start_pressed.png"));
    allocate(left = new MF_Button("left.png", "left_pressed.png"));
    allocate(right = new MF_Button("right.png", "right_pressed.png"));
    allocate(down = new MF_Button("down.png", "down_pressed.png"));
    allocate(up = new MF_Button("up.png", "up_pressed.png"));
    allocate(servers = new MF_BarManager(387,300,35,28,"verdana.ttf",11));

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
    servers->add_type<MF_TextBar>(105,ls+2); //name
    servers->add_type<MF_TextBar>(205,ls+2); //admin
    servers->add_type<MF_TextBar>(50,ls+2);  //players

    //Special connection
    c.pSup = this;
    c.start_everything();
}

void ServerList::display(Surface &ecran)
{
    if (!updated)
    {
        updated = true;
        self_display(ecran);
        displayMF(ecran);
    } else if (pStart->check_updated() == false)
    {
        pStart->display(ecran);
    }
}

void ServerList::self_display(Surface &ecran)
{
    ecran.fill(0, Color(255,255,255));
    MF_Applet::display(ecran);
}

bool ServerList::RecvFromSub(const char *message, MF_Base *fenetre)
{
    if (fenetre == b and strcmp(message, "release") == 0)
    {
        sendToBoss("menu");
        return true;
    }

    if (fenetre == &c)
    {
        if (strncmp(message, "error: ", strlen("error: ")) == 0)
        {
            allocate ( new MF_Alert(this, police, message + strlen("error: "), bgColor) );
            return true;
        }
    }

    if (fenetre == up and strcmp(message, "clic") == 0)
    {
        servers->montecurseur(1);
        return true;
    }

    if (fenetre == down and strcmp(message, "clic") == 0)
    {
        servers->descendcurseur(1);
        return true;
    }

    return false;
}

void ServerList::set_serverlist(const vector<ServerData> &list)
{
    servers->clear_options();
    servers->add_option(0, "Name", "Description", "Players");

    vector<ServerData>::const_iterator it;
    int i = 1;

    for (it = list.begin(); it != list.end(); ++it, ++i) {
        servers->add_option(i, it->name.c_str(), it->description.c_str(), toString(it->num_players).c_str());
    }
}


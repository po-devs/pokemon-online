#include "TheInternet.h"
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include "utilities.hh"

using namespace std;
using sf::Socket::Error;
using sf::Socket::Disconnected;
using sf::Socket::NotReady;
using sf::Socket::Done;
using sf::Socket::Status;


/*  Receive a message from &sock and append it to recv, stopping if finished is true.
    sock should be set to NonBlocking beforehand.

    Return:
        - sf::Socket::NotReady if was interrupted by finished
        - sf::Socket::Error or sf::Socket::Disconnected if such was the error
        - sf::Socket::Done if everything went well
*/
sf::Socket::Status recv_body(sf::SocketTCP &sock, string &recv, const bool &finished)
{
    /* three steps: getting the length of the length of the message,
       then the length of the message,
       then the message
    */

    char buffer[500];

    size_t received;

    //Step one : length of length
    while (1)
    {
        if (finished)
            return NotReady;

        Status s = sock.Receive(buffer, 1, received);

        if (s == Error || s == Disconnected)
            return s;

        if (s == Done)
            break;

        SDL_Delay(10);
    }

    if (received != 1)
        return Error;

    //length of length :)
    Uint8 l_l = buffer[0];

    // cursor, where are we located in the buffer
    int i=0;

    if (l_l >= sizeof(buffer))
            return Error;

    //Step two : length
    while (1)
    {
        if (finished)
            return NotReady;

        Status s = sock.Receive(buffer+i, l_l - i, received);

        if (s == Error || s == Disconnected)
            return s;

        if (s == Done)
        {
            i += received; //moving cursor
            if (i == l_l)
                break;
            else
                continue;
        }

        SDL_Delay(10);
    }

    //length of the message
    size_t length = ConnectToRegistry::calc_length(recv.data(), l_l);

    //Step three: Yata!
    while (1)
    {
        if (finished)
            return NotReady;

        Status s = sock.Receive(buffer, min(sizeof(buffer), length), received);

        if (s == Error || s == Disconnected)
            return s;

        if (s == Done)
        {
            recv.append(buffer, received);
            length -= received; //remaining length to go

            if (length == 0)
                break;
            else
                continue;
        }

        SDL_Delay(10);
    }

    return Done;
}





ConnectToRegistry::ConnectToRegistry()
                  :thread(&ConnectToRegistry::the_real_thing, this)
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
    thread.Launch();
}

///Doesn't work for some reason (not my fault though)
//void ConnectToRegistry::the_real_thing(void *data)
//{
//    //classe de base
//    ConnectToRegistry &base = *(static_cast<ConnectToRegistry*>(data));
//    sf::SocketTCP Client;
//
//    try
//    {
//        //on prend l'addresse de la registry
//        string regip = *find_line("db/registry.txt", 0);
//
//        Client.SetBlocking(false);
//
//        if (Client.Connect(50508, regip.c_str()) != sf::Socket::Done)
//            throw "error: Error while connecting to registry (" + regip + ").";
//
//        if (base.finish_off)
//            goto end;
//
//        string msg = "__servlist";
//        msg[0] = 1;
//        msg[1] = msg.length()-2;
//
//        while (1)
//        {
//            if (base.finish_off)
//                goto end;
//
//            Status s = Client.Send(msg.c_str(), msg.length());
//
//            if (s == Disconnected || s == Error)
//                throw "error: Error when asking registry for server list.";
//
//            if (s == Done)
//                break;
//
//            SDL_Delay(10);
//        }
//
//        //getting registry's answer
//        string recv;
//
//        Status s = recv_body(Client, recv, base.finish_off);
//
//        if (s == NotReady)
//            goto end;
//
//        if (s == Error || s == Disconnected)
//            throw "error: Error when receiving server list";
//
//        //Now everything is in the right string!
//        //time for heavy weaponry
//        MegaDeserializer pile(recv);
//
//        //TODO: deserialize the data ;)
//    } catch (const string &ex)
//    {
//        cout << "gotcha" << endl;
//        base.envoieMessage(ex.c_str());
//    } catch (const char *a)
//    {
//        cout << "gotcha" << endl;
//        base.envoieMessage(a);
//    }
//
//    end:
//    Client.Close();
//    return;
//}

void ConnectToRegistry::the_real_thing(void *data)
{
    //classe de base
    ConnectToRegistry &base = *(static_cast<ConnectToRegistry*>(data));
    sf::SocketTCP Client;
    string ex; //exception

    //on prend l'addresse de la registry
    string regip = *find_line("db/registry.txt", 0);

    Client.SetBlocking(false);

    string msg,recv;
    Status s;
    MegaDeserializer pile;

    if (Client.Connect(50508, regip.c_str()) != sf::Socket::Done) {
        ex = "error: Error while connecting to registry (" + regip + ").";
        goto ex;
    }

    if (base.finish_off)
        goto end;

    msg = "__servlist";
    msg[0] = 1;
    msg[1] = msg.length()-2;

    while (1)
    {
        if (base.finish_off)
            goto end;

        s = Client.Send(msg.c_str(), msg.length());

        if (s == Disconnected || s == Error) {
            ex = "error: Error when asking registry for server list.";
            goto ex;
        }

        if (s == Done)
            break;

        SDL_Delay(10);
    }

    //receives the body of the answer from registry
    s = recv_body(Client, recv, base.finish_off);

    if (s == NotReady)
        goto end;

    if (s == Error || s == Disconnected) {
        ex = "error: Error when receiving server list";
        goto ex;
    }

    //Now everything is in the right string!
    //time for heavy weaponry
    pile.init(recv);

    //TODO: deserialize the data ;)
end:
    Client.Close();
    return;
ex:
    Client.Close();
    base.envoieMessage(ex.c_str());
    return;
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

#ifndef THEINTERNET_H_INCLUDED
#define THEINTERNET_H_INCLUDED

#include "MF_applet.hh"
#include <SFML/System.hpp>

class ConnectToRegistry : public MF_Base
{
    public: /* private */
        /** @name  the_real_thig
            @brief la fonction threadée
        **/
        static void the_real_thing(void *data);
        /** @name  calc_length
            @brief calcule la longueur du message à recevoir
            @param data  le début du message
            @param l_l  la longueur en nombre de caractères de la longueur du message
        **/
        static size_t calc_length(const char *data, Uint8 l_l);

    public:
        /* Si mis à true, on doit arrêter ce qu'on fait */
        bool finish_off;
        sf::Thread thread;

        ConnectToRegistry();
        ~ConnectToRegistry();

        void start_everything();
};

class ServerList : public MF_BDirections, public MF_Applet
{
    public:
        MF_Button *a,*b,*start, *up, *right, *left, *down;
        MF_BarManager *servers; //35,28,387,300
        MF_MLine *description;
        ConnectToRegistry c;
        Font police;

        ServerList();

        void display(Surface &ecran);
        void self_display(Surface &ecran);

        bool RecvFromSub(const char *message, MF_Base *fenetre);
};



#endif // THEINTERNET_H_INCLUDED

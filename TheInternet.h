#include "MF_applet.hh"

#ifndef THEINTERNET_H_INCLUDED
#define THEINTERNET_H_INCLUDED

class ConnectToRegistry : public MF_Base
{
    public: /* private */
        static int the_real_thing(void *data);
        static size_t calc_length(const char *data, Uint8 l_l);

    public:
        //a flag to kill
        bool finish_off;
        SDL_Thread *super;

        ConnectToRegistry();
        ~ConnectToRegistry();

        void start_everything();
};

class ServerList : public MF_BDirections, public MF_Applet
{
    public:
        MF_Button *a,*b,*start, *up, *right, *left, *down;
        MF_BarManager *servers; //35,28,387,300
        MF_MLigne *description;
        ConnectToRegistry c;
        Font police;

        ServerList();

        void affiche(Surface &ecran);
        void self_affiche(Surface &ecran);

        bool recoitMessage(const char *message, MF_Base *fenetre);
};



#endif // THEINTERNET_H_INCLUDED

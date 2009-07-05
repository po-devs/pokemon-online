#ifndef THEINTERNET_H_INCLUDED
#define THEINTERNET_H_INCLUDED

#include "../generic/MF_applet.hh"
#include "../generic/interthread.hpp"
#include <vector>

/* The data of a server is like this */
struct ServerData
{
    uint32_t refip;
    uint16_t num_players; //10 bits
    std::string name; //len: 5bits
    std::string description; //len: 9 bits
};

class ConnectToRegistry : public MF_Base
{
    public: /* private */
        /** @name  the_real_thing
            @brief la fonction threadée
        **/
        static int the_real_thing(void *data);

    public:
        /* Si mis à true, on doit arrêter ce qu'on fait */
        bool finish_off;
        interface::Thread thread;

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

        void set_serverlist(const std::vector<ServerData> &list);

        bool RecvFromSub(const char *message, MF_Base *fenetre);
};



#endif // THEINTERNET_H_INCLUDED

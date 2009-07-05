#include "serverdata.h"
#include "../generic/internet_utilities.hh"

using namespace std;
using _508::recv_body;
using _508::send_body;
using namespace interface;

extern Selector<PlaySocket> *play_select;
extern Selector<ServSocket> *serv_select;
extern ServList valid_list;
extern ServList invalid_list;
extern uint32_t vlist_size;
extern uint32_t ilist_size;
extern void dispose_thread(Thread *t);

extern void dec_scount(uint32_t key);
extern uint8_t inc_scount(uint32_t key);
extern void dec_pcount(uint32_t key);
extern uint8_t inc_pcount(uint32_t key);

void StopAllThreads()
{
    /* Calling those function with a NULL argument
       set their static flag so that threads terminate */
    serv_io(NULL);
    play_io(NULL);
}

/* On suppose que seule cette fonction modifie les infos d'un serveur déjà créé
   -> pas besoin de mutex pour la lecture */
int serv_io(void *data)
{
    static bool terminate = false;

    if (!data)
    {
        //terminate all the threads calling that function
        terminate = true;
        return 0;
    }

    ServerData &serv = * (static_cast<ServerData *> (data));
    ServSocket &sock = serv.client;

    /* A quelle liste appartient le serveur? */
    bool valid = (serv.name != "");

    /* On reçoit les infos du serveur */
    string recv;
    Socket::Status s = recv_body(sock, recv, terminate);

error:
    if (s != Socket::Done)
    {
        /* il y a une erreur! */
        ServList &liste = (valid? valid_list : invalid_list);
        uint32_t &size  = (valid? vlist_size : ilist_size);

        liste.Lock();
        liste.erase(*sock.get_var());
        size -= 1;
        liste.Unlock();
        serv_select->Remove(sock);
        dec_scount(serv.client->GetIp().ToInteger());
        delete &serv;

        return 0;
    }

    MegaDeserializer des(recv);

    //deserialize possède un mutex donc tout va bien
    if (!deserialize(des, serv) || serv.name == "") {
        /* il y a une erreur de format (ou une tentative de hack ^^)
            ou un serveur qui ne donne même pas son nom donc on enlève le serveur! */
        s = Socket::Error;
        goto error;
    }

    /* Si la liste est déjà dans valid_list, rien à changer, sinon
        on la met dans valid_list */
    if (!valid)
    {
        invalid_list.Lock();
        valid_list.Lock();

        valid_list.splice(valid_list.end(), invalid_list, *sock.get_var());

        vlist_size += 1;
        ilist_size -= 1;

        valid_list.Unlock();
        invalid_list.Unlock();
    }

    //fin du thread
    serv.threaded = false;
    return 0;
}


int play_io(void *data)
{
    static bool terminate = false;

    if (!data)
    {
        //terminate all the threads calling that function
        terminate = true;
        return 0;
    }

    PlaySocket &sock =  * (static_cast<PlaySocket *> (data));

    //réception
    TRACE(string recv;)
    TRACE(Socket::Status s = recv_body(sock, recv, terminate);)

    //si déconnexion ou terminate
    TRACE(if (s != Socket::Done))
    {
        TRACE(goto error;)
    }

    //On étudie enfin le contenu du message!
    TRACE(if (recv == "servlist"))
    {
        /* envoyer la liste des serveurs */

        /* sérialisation de server_list */
        TRACE(MegaSerializer ser;)
        valid_list.Lock();
        ser.push_l(vlist_size, 16); //on limite quand même le nombre de serveurs à 65535
        ServList::iterator it;
        for (it = valid_list.begin(); it != valid_list.end(); ++it) {
            (*it)->Lock();
            serialize(ser, **it);
            (*it)->Unlock();
        }
        valid_list.Unlock();
        /* fin de la sérialisation */

        /*envoi*/
        size_t bitslen;
        /* détermination de la longueur de la chaine + obtention de la chaine */
        const char *cc = ser.get_ccdata(&bitslen);
        if (bitslen % 8 != 0)
            bitslen = (bitslen >> 3) + 1;
        else
            bitslen = bitslen >> 3;
        /* fin de la détermination et de l'obtention */
        s = send_body(sock, cc, bitslen, terminate);
        if (s != Socket::Done)
        {
            goto error;
        }
    }

    /* on quitte et se met en attente pour être détruit */
    TRACE(dispose_thread(*sock.get_var());)
    TRACE(sock.change_var(NULL);) /* pour indiquer qu'il n'y a plus de threads s'occupant de socks */
    return 0;

error:
    TRACE(play_select->Remove(sock);)
    TRACE(dispose_thread(*sock.get_var());)
    TRACE(dec_pcount(sock->GetIp().ToInteger());)

    return 0;
}

ServerData::ServerData(const ServSocket &client, uint16_t num_players, const string &name, const string &description)
        : client(client), num_players(num_players), name(name), description(description)
            , thread(NULL), threaded(false)
{
    ;
}

ServerData::~ServerData()
{
    /* we don't wait explicitly for the thread,
                as the very thread called by this class may as well delete it */
    dispose_thread(thread);
}

bool ServerData::create_thread(Thread::FuncType Function)
{
    if (threaded) {
        return false;
    }
    delete thread;
    thread = new Thread(Function, this);
    threaded = true;
    return true;
}


void serialize(MegaSerializer &ser, const ServerData& s)
{
    char scheme[] = {'#', '0'+32, '#', '0'+10, '$', '5', '$', '9', 0};
    ser.mega_push(scheme, s.client->GetIp().ToInteger(), s.num_players, &s.name, &s.description);
}

bool deserialize(MegaDeserializer &des, ServerData &s)
{
    char scheme[] = {'#', '0'+10, '$', '5', '$', '9', 0};
    s.Lock();
    bool ret = des.mega_pop(scheme, &s.num_players, &s.name, &s.description);
    s.Unlock();

    return ret;
}

/**THIS FILE WILL NOT BE USED ANYMORE IN THIS PROJECT,
instead I'm going to use SFML's Network module **/


#ifndef _508SOCKET_H_INCLUDED
#define _508SOCKET_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <list>
#include <utility>
#include <string>

#include <iostream>
#include <fstream>
using namespace std;

/* SM for Safe Mode */
#define _508sockSM
#ifdef I_KNOW_WHAT_I_AM_DOING //if you know what you're doing, then I allow you to play with fire
 #define _508private public
#else
 #define _508private private
#endif

namespace _508
{

namespace sock
{

typedef void * thread ;
typedef bool * block ;

class sockboss;
class socket;

/* _socket SHOULD NOT EVEN BE USED BY YOU DIRECTLY, USE socket INSTEAD, IT MANAGES IT VERY WELL */

/* This thing use a SDL TCP socket ..
   It's a bit like fstream, i mean you convert it to bool to see if
   something wrong happened and you use clear() to clear the err flag.

   it got its own protections you can use to protect it against multithreading:
     active() checks if its already used
     reserve() return true if it's not used and also prevents others from using it, or false otherwise
     release() remove any protection.

   Note that the protection thing is only active() returning true when sock is reserved.

   Oh, it has also a ref count system, it's used very well by the class socket, and functions like
   _socket::share are part of this.
*/
class _socket
{
    friend class socket;
    public:
        TCPsocket sock;
        bool fail; // if last thing done failed
        bool connected; //if currently connected to something
        int lastcode;
        size_t length;
        void *buffer;
        size_t id; //its id in the sockboss class
        mutable size_t refcount; //refcount system :D
        sockboss *boss; //its sockboss
        SDL_Thread *thread;
        bool *threaded;

        explicit _socket (const _socket& s) {
            #ifdef _508sockSM
            throw "_508::sock::_socket copy constructor forbidden -- use socket instead of _socket ffs";
            #endif
        }
        _socket & operator = (const _socket &s) {
            #ifdef _508sockSM
            throw "_508::sock::_socket operator = forbidden -- use socket instead of _socket ffs";
            #endif
        }

        ~_socket ();

        operator bool () const {
            return !fail;
        }
        operator int () const {
            return lastcode;
        }
        bool active() const {
            return _active;
        }
        void clear() {
            fail = false;
        }
        bool reserve();

        void release() {
            _active = false;
        }
        void init(sockboss *b, size_t id) ;

        _socket *share() const {
            refcount++;
            return const_cast<_socket *> (this);
        }

        /* to use only if SDLNet_CheckSockets has been called on a set including this one, recently please */
        bool ready() const {
            return !_active && SDLNet_SocketReady(sock);
        }

        /* Don't use two of them at the same time -- use reserve() and release() to avoid that kind of misunderstanding */
        _socket &open(const IPaddress &ip);
        _socket &open(TCPsocket sock);
        _socket &open(const char *host, Uint16 port);
        _socket &close();
        /* If the boolean threaded is not NULL, then the action is threaded and the boolean threaded will be
            put to false when the action is finished */
        _socket &send(const std::string &msg, bool *threaded = NULL);
        _socket &send(const std::string &msg, size_t length, bool *threaded = NULL);
        _socket &send(const char * msg, size_t length, bool *threaded = NULL);
        _socket &recv(std::string &recv_string, size_t length, bool *threaded = NULL);
        _socket &recv(char *recv_string, size_t length, bool *threaded = NULL);
        _socket &accept(TCPsocket *recv_sock, bool *threaded = NULL);

    _508private:
        bool _active; /** use active()!!! **/
        SDL_mutex * _mutex; /** for _active **/

        _socket (sockboss *b = NULL, size_t id = ~0);
        _socket (const IPaddress &ip, sockboss *b = NULL, size_t id = ~0);
        _socket (const TCPsocket sock, sockboss *b = NULL, size_t id = ~0);
        _socket (const char *host, Uint16 port, sockboss *b = NULL, size_t id = ~0);

        /* All those four functions use the void* buffer member, don't mess! */
        _socket &recv(); //the addr of the receiving char is stored in buffer
        _socket &strrecv(); //the addr of the receiving string to send is stored in buffer
        _socket &send(); //the addr of the thing to send is stored in buffer
        _socket &accept(); //the addr of the sock is stored in buffer

        static int CreateThread(void *data);
        _socket& createthread(_socket& (_socket::*ptr)() ) {
            SDL_WaitThread(thread, NULL);
            std::pair<_socket*, _socket& (_socket::*)()> p (this, ptr);
            thread = SDL_CreateThread(CreateThread, &p);
            return *this;
        }
        _socket &threadornot(_socket& (_socket::*ptr)(), bool *threaded) {
                if (!threaded) {
                    SDL_WaitThread(thread, NULL);
                    return (this->*ptr)();
                } else {
                    this->threaded = threaded;
                    return createthread(ptr);
                }
        }
};

/* Wrapper for the _socket type
   the _socket is accessible by using .s or its members with ->*/
class socket
{
    public:
        _socket *s; //use with caution!
    public:
        const _socket& operator * () const { return *s; }
        _socket& operator * () { return *s; }
        const _socket* operator->() const { return &(operator*()); }
        _socket* operator->() { return &(operator*()); }
//        operator bool () const { return bool(*s); }
//        operator int () const { return int(*s); }

        socket (sockboss *b = NULL, size_t id = ~0) { s = new _socket(b, id); }
        socket (const IPaddress &ip, sockboss *b = NULL, size_t id = ~0) { s = new _socket(ip, b, id); }
        socket (const TCPsocket sock, sockboss *b = NULL, size_t id = ~0) { s = new _socket(sock, b, id); }
        socket (const char *host, Uint16 port, sockboss *b = NULL, size_t id = ~0) { s = new _socket(host, port, b, id); }
        socket (const socket &sock) { s = sock->share(); }
        socket& operator = (const socket &sock) {if (s) s->~_socket(); s = sock->share(); return *this;}
        ~socket() { if (s) s->~_socket(); s = NULL;}
};

/*
  Source of error? Should put id = NULL in the destructor
*/
class sockboss
{
    public:
        size_t maxsockets;
        /* Stocke toutes les sockets */
        /* Contains the various sockets, its size is maxsockets */
        socket **id;
        size_t numsockets;
        /* We need a SDLNet set to check if sockets are ready */
        SDLNet_SocketSet set;
        const char *error;

        sockboss(size_t msocks = 512);
        ~sockboss();

        /* Same as SDLNet_Checksockets */
        int checksockets();
        /* All call and return addsocket */
        int createsocket(const TCPsocket sock);
        int createsocket(const char *host, Uint16 port);
        int createsocket(const IPaddress &ip);
        int createsocket();
        /* Returns -2 if too many socks already,
            if socket is not empty (sock->sock!= NULL) tries to add socket->sock to set (-1 on errors),
            at last gets place in the tab and returns its ID */
        int addsocket(socket sock);
        /* only if sock was empty before
           Returns -1 on errors or the ID */
        int addtoset(socket &sock);
        /* Returns the error num */
        int delsocket(socket &sock);
        bool getsock(size_t id, socket &receiver);
        int numinset();
        /* internally, when creating a socket gets a unused ID */
        size_t freeid();
};

//obvious memory leak with assoc, don't use yet
template <class T>
class asockboss : public sockboss
{
    public:
        T **assoc;

        asockboss(size_t msocks = 512);
        ~asockboss();

        int addsocket(socket sock);
        int delsocket(socket &sock);

        bool getassoc(size_t id, T &receiver);

        template <class U>
        void map(void (U::*ptr) (T *el));
};

template <class T>
inline asockboss<T>::asockboss(size_t msocks) : sockboss(msocks), assoc(new T* [msocks])
{
    for (size_t i = 0; i < maxsockets; i++)
    {
        assoc[i] = NULL;
    }
}

template <class T>
inline asockboss<T>::~asockboss()
{
    for (size_t i = 0; i < maxsockets; i++)
    {
        delete assoc[i];
    }
    delete [] assoc;
}

template <class T>
inline int asockboss<T>::addsocket(socket sock)
{
    int no = sockboss::addsocket(sock);
    if (no < 0)
        return no;

    assoc[no] = new T(*id[no]);
    return no;
}

template <class T>
inline int asockboss<T>::delsocket(socket &sock)
{
    delete assoc[sock->id], assoc[sock->id] = NULL;
    return sockboss::delsocket(sock);
}

inline bool _socket::reserve()
{
    SDL_mutexP(_mutex);
    bool result = !_active;
    if (result) _active = true;
    SDL_mutexV(_mutex);
    return result;
}

inline void _socket::init(sockboss *b, size_t id)
{
    _active = false;
    length = 0;
    buffer = NULL;
    fail = false;
    lastcode = 0;
    connected = false;
    thread = NULL;
    refcount = 1;
    threaded = NULL;
    sock = NULL;
    boss = b;
    this->id = id;
    _mutex = SDL_CreateMutex();
}

inline _socket::_socket (sockboss *b, size_t id)
                : sock(NULL)
{
    init(b, id);
}

inline _socket::_socket (const IPaddress &ip, sockboss *b, size_t id)
{
    init(b, id);
    open(ip);
}

inline _socket::_socket (const TCPsocket sock, sockboss *b, size_t id)
                : sock(sock)
{
    init(b, id);
}

inline _socket::_socket (const char *host, Uint16 port, sockboss *b, size_t id)
{
    init(b, id);
    open(host, port);
}

inline _socket::~_socket ()
{
    /* Reference counting system :D */
    if (! --refcount)
    {
        SDL_WaitThread(thread, NULL);
        SDL_DestroyMutex(_mutex);
        close();

        delete this;
    }
}

inline int _socket::CreateThread(void *data)
{
    std::pair < _socket*, _socket& (_socket::*)() > p = * (static_cast< std::pair<_socket*, _socket& (_socket::*)()> * > (data));
    ((*p.first).*p.second)();
    if (p.first->threaded) {
        *(p.first->threaded) = false;
    }
    p.first->thread = NULL;

    return 0;
}

inline _socket& _socket::open(const IPaddress &ip)
{
    close();
    sock = SDLNet_TCP_Open(const_cast<IPaddress*> (&ip));
    connected = !(fail = sock == NULL);
    return *this;
}

inline _socket& _socket::open(TCPsocket sock)
{
    close();
    this->sock = sock;
    connected = sock != NULL;
    return *this;
}

inline _socket& _socket::open(const char *host, Uint16 port)
{
    close();
    IPaddress ip;
    if ( (fail =(lastcode = SDLNet_ResolveHost(&ip, host, port)) < 0)) {
        return *this;
    }
    return open(ip);
}

inline _socket& _socket::close()
{
    SDL_WaitThread(thread, NULL);
    if (sock) {
        SDLNet_TCP_Close(sock);
    }
    sock = NULL;
    connected = false;
    return *this;
}

inline _socket & _socket::send(const std::string &msg, bool *threaded)
{
    return send(msg, msg.length(), threaded);
}

inline _socket & _socket::send(const std::string &msg, size_t length, bool *threaded)
{
    return send (msg.data(), length, threaded);
}

inline _socket & _socket::send(const char * msg, size_t length, bool *threaded)
{
    buffer = const_cast <char*> (msg);
    this->length = length;

    return threadornot(& _socket::send, threaded);
}

//basic send function
inline _socket &_socket::send()
{
    fail = ((lastcode = SDLNet_TCP_Send(sock, buffer, length)) < (int)length);
    return *this;
}

inline _socket & _socket::recv(std::string &buf, size_t length, bool *threaded)
{
    buffer = &buf;
    this->length = length;

    return threadornot(&_socket::strrecv, threaded);
}

inline _socket &_socket::recv(char *buf, size_t length, bool *threaded)
{
    buffer = buf;
    this->length = length;

    return threadornot(&_socket::recv, threaded);
}

//basic recv(string) function
inline _socket & _socket::strrecv()
{
    char buf[length];
    fail = ((lastcode = SDLNet_TCP_Recv(sock, buf, length)) < 0);
    if (!fail) {
        std::string *ptr = static_cast <std::string *> (buffer);
        ptr->append(buf, lastcode);
    }
    return *this;
}

//basic recv(char) function
inline _socket &_socket::recv()
{
    fail = ((lastcode = SDLNet_TCP_Recv(sock, buffer, length)) <= 0);
    return *this;
}

inline _socket &_socket::accept(TCPsocket *s, bool *threaded)
{
    buffer = s;
    return threadornot(&_socket::accept, threaded);
}

//basic accept function
inline _socket &_socket::accept()
{
    *((TCPsocket*)buffer) = SDLNet_TCP_Accept(sock);
    return *this;
}

inline sockboss::sockboss (size_t msocks)
    : maxsockets (msocks), id (new socket * [msocks]), numsockets(0), set(SDLNet_AllocSocketSet(msocks)), error("")
{
    for (size_t i = 0; i < maxsockets; i++)
    {
        id[i] = NULL;
    }
}

inline sockboss::~sockboss ()
{
    for (size_t i = 0; i < maxsockets; i++)
    {
        delete id[i];
    }
    delete [] id;
    SDLNet_FreeSocketSet(set);
}

/* numsocket must be AT LEAST the number of sockets to be (included the ones just added) minus 1 */
inline size_t sockboss::freeid()
{
    size_t i;
    for (i = 0; i < numsockets; i++)
    {
        if (id[i] == NULL)
            break;
    }
    return i;
}

inline int sockboss::createsocket(TCPsocket sock)
{
    return addsocket(socket(sock));
}

inline int sockboss::createsocket(const char *host, Uint16 port)
{
    return addsocket(socket(host, port));
}

inline int sockboss::createsocket(const IPaddress &ip)
{
    return addsocket(socket(ip));
}

inline int sockboss::createsocket()
{
    return addsocket(socket());
}

inline int sockboss::addsocket(socket sock)
{
    if (numsockets >= maxsockets) {
        return -2;
    }

    #ifdef _508sockSM
    if (sock->boss != NULL) {
        throw "Réassignation d'une socket sans passer par le boss?? oO";
    }
    #endif

    if (sock->sock != NULL) {
        if ( SDLNet_TCP_AddSocket(set, sock->sock) <= 0) {
            return -1;
        }
    }

    sock->boss = this;
    sock->id = freeid();
    id[sock->id] = new socket(sock);

    numsockets += 1;

    return sock->id;
}

inline int sockboss::numinset()
{
    return SDLNet_TCP_AddSocket(set, NULL);
}

inline int sockboss::addtoset(socket &sock)
{
    #ifdef _508sockSM
    if (sock.s && sock->boss != this) {
        throw "On essaie de me voler? :'(";
    }
    SDLNet_TCP_DelSocket(set, sock->sock);
    #endif

    return SDLNet_TCP_AddSocket(set, sock->sock);
}

inline int sockboss::delsocket (socket &sock)
{
    #ifdef _508sockSM
    if (sock->boss != this) {
        throw "Crime aggravé: destruction de la propriété d'autrui";
    }
    #endif

    size_t sockid = sock->id;
    TCPsocket s = sock->sock;

    //  if not owned, invalid, not owned, not owned
    if (sockid == (size_t)~0 || sockid > maxsockets || id[sockid] == NULL || id[sockid]->s != sock.s) {
        return -1;
    }

    sock->boss = NULL;

    //removing before destroying its contents..
    int ret = SDLNet_TCP_DelSocket(set, s);

    delete id[sockid];
    id[sockid] = NULL;

    numsockets -= 1;

    return ret;
}

inline bool sockboss::getsock(size_t id, socket &receiver)
{
    if (id >= maxsockets) {
        return false;
    }

    socket * it = this->id[id];

    if (it == NULL) {
        return false;
    }

    receiver = *it;

    return true;
}

inline int sockboss::checksockets ()
{
    return SDLNet_CheckSockets(set, 0);
}

}; //namespace _508::sock

}; //namespace _508

#undef _508private

#endif // _508SOCKET_H_INCLUDED

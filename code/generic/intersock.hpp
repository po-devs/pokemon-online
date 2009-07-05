/*
    Wrapper for TCPsocket in an SFML fashion.
    (see www.sfml-dev.org)

    coyotte508@hotmail.com
*/

#include <string>
#include <list>
#include <set>
#include <SDL/SDL_net.h>
#include <iostream>

#include "utilities.hh"
#include "intermutex.hpp"

#ifndef INTERSOCK_HPP_INCLUDED
#define INTERSOCK_HPP_INCLUDED



namespace interface
{

class IPAddress
{
private:
    friend class _TCPSocket;
    IPaddress myIP;

    IPAddress(const IPaddress& ip);
public:

    enum
    {
        Invalid = 0xFFFFFFFF
    };

    /* invalid by default */
    IPAddress();
    /* Try to get that IP address */
    IPAddress(const char *host, uint16_t port);

    bool Resolve(const char *host, uint16_t port);
    std::string ToString();
    uint32_t ToInteger() const;
    uint16_t Port() const;
    bool IsValid() const;

    operator bool () const {
        return IsValid();
    }
};

struct Socket
{
    enum Status
    {
        Done,
        NotReady,
        Error
    };
};

class _BaseSelector;
class _TCPSocket;
/* Magic of the templates...
    go look utilities .hh for a definition of wrapper<> */

/**  @name TCPSocket

    A copyable class.
    All copies share the same socket.

    Functions:
        void SetBlocking(bool blocking);
        bool IsBlocking() const;

        Socket::Status Receive (char *data, size_t maxsize, size_t &recvd_size);
        Socket::Status Send (const char *data, size_t size);
        Socket::Status Listen (uint16_t port);
        Socket::Status Connect (const char *host, uint32_t port);
        Socket::Status Connect (const IPAddress &ip);
        IPAddress GetIp () const;
        void Close();

        ##Is the socket somehow valid?##
        operator bool ();

    Access the functions using '->' instead of '.'
    The socket is automatically closed in case of error,
    or in case of the destruction of the last copy,
    or when needed.

    So you may not have to use Close()...
*/
typedef wrapper <_TCPSocket> TCPSocket;

template <typename> class Selector;

/* A socket that is non copyable and should only be used internally */
class _TCPSocket : public NonCopyable
{
public: /* private!! */
    TCPsocket sock;
    SDLNet_SocketSet set;

    typedef std::list< _BaseSelector * > BossList;
    BossList assoc; //the selectors that contain that socket

    /* caution: before each add you need one remove */
    void remove_from_assoc(); //remove the current sock from the assoc' socket set.
    void add_to_assoc(); //add the current sock to the assoc' socket set
    bool check_ready() const;
public:
    _TCPSocket(TCPsocket s = NULL);
    ~_TCPSocket();

    void SetBlocking(bool blocking);
    bool IsBlocking() const;

    Socket::Status Receive (char *data, size_t maxsize, size_t &recvd_size);
    // Doesn't manage the non-blocking yet (wait for SDL_net to do so!!)
    Socket::Status Send (const char *data, size_t size);
    Socket::Status Listen (uint16_t port);
    Socket::Status Accept (TCPSocket &other);
    Socket::Status Connect (const char *host, uint32_t port);
    Socket::Status Connect (IPAddress &ip);
    IPAddress GetIp () const;
    void Close();

    /* Is the socket somehow valid? */
    operator bool ();
};



/* used for the friend declaration
    eventually you will create a derived class of TCP socket,
    and use a Selector <> of that class, and then there'll
    be problems

    There will be another Add (T) and Remove (T) function in the derived classes,
    but no reason to overload this Add and this Remove.
*/
class _BaseSelector
{
protected:
    friend class _TCPSocket;

    SDLNet_SocketSet mySet;
    size_t maxsize;
    size_t numinset; /* the number of sockets there are in the SocketSet */

    /* Do not use this function in any other member function as it's virtual and can be changed
    (I especially think to the disaster it would cause if used in SafeSelector)

    Those functions are only meant as friend functions of _TCPSocket. */
    virtual void _Add(_TCPSocket *ptr);
    virtual void _Remove(_TCPSocket *ptr);

    /* num of sockets stored */
    virtual size_t size() const = 0;

    /* Constructor and destructor.. */
    _BaseSelector(size_t maxsockets = 256);
    virtual ~_BaseSelector();
};

struct Select
{
    enum Status
    {
        Done, //ok
        Duplicate, //there is already that socket in the set
        TooMany, //there are too many sockets in the set
        NotFound //when removing, removing a non-existing socket...
    };
};

/* I made this class non-copyable, cause it'd be a true hassle to copy a Selector
  (you'd have to create another selector with the needed memory, then copy the sets
  AND the socket sets, and add any socket in the socket set using the right function
  so you give them a pointer of this class in their "assoc" member...
  It's just so much easier to use a wrapper... ( typedef wrapper<Selector<T> > CopyableSelector ...) */
template <class T>
class Selector : public _BaseSelector, public NonCopyable
{
protected:
    typedef T type;

    std::set<type, std::less<type> > sockets;
    std::vector<type> readysockets;

    typedef typename std::set<type, std::less<type> >::iterator set_iterator;
    typedef typename std::vector<type>::iterator vector_iterator;
public:
    Selector (size_t maxsockets = 256);
    ~Selector ();

    Select::Status Add(T newel);
    Select::Status Remove(T &el);

    /* Contrary to SFML Selectors, a timeout of 0 actually means "immediately".
        if you want an infinite timeout (49 days) then use ~0 .
        Return the number of ready sockets with a max timeout of timeout milliseconds
        (returns as soon as there is a ready socket, wait only if  there's none) */
    size_t Wait(size_t timeout = 0);

    virtual size_t size() const;
    T GetSocketReady(unsigned int index); //after a call to Wait...
};

/* You need to lock this class during the whole Wait() / GetSocketReady thing */
template <class T>
class SafeSelector : public Selector<T>, public Mutex
{
protected:
    virtual void _Add(_TCPSocket *ptr) { Lock(); _BaseSelector::_Add(ptr); Unlock(); }
    virtual void _Remove(_TCPSocket *ptr) { Lock(); _BaseSelector::_Remove(ptr); Unlock(); }
public:
    enum Status
    {
        Done, //ok
        Duplicate, //there is already that socket in the set
        TooMany, //there are too many sockets in the set
        NotFound //when removing, removing a non-existing socket...
    };

    SafeSelector (size_t maxsockets = 256) : Selector<T> (maxsockets) {;}

    Select::Status Add(T newel) { Lock(); Select::Status s = Selector<T>::Add(newel); Unlock(); return s; }
    Select::Status Remove(T &el) { Lock(); Select::Status s = Selector<T>::Remove(el); Unlock(); return s; }
};

//////////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////////// ///
///                         TPP FILE                               ///
/// ////////////////////////////////////////////////////////////// ///
//////////////////////////////////////////////////////////////////////

template <class T>
Selector <T> :: Selector(size_t maxsockets) : _BaseSelector (maxsockets)
{
    readysockets.reserve(maxsockets);
}

template <class T>
Selector <T> :: ~Selector()
{
    /* We need to remove ourselves from all of our underling sockets */
    set_iterator it;

    for (it = sockets.begin(); it != sockets.end(); ++it)
    {
        _TCPSocket::BossList list = (*it)->assoc;
        _TCPSocket::BossList::iterator it2 = std::find(list.begin(),
                list.end(), this);
        /* normally it should be there, no need to test */
        list.erase(it2);
    }

    /* Then clear... */
    sockets.clear();
    readysockets.clear();
}

template <class T>
Select::Status Selector <T> :: Add (T newel)
{
    if (sockets.find(newel) != sockets.end())
    {
        return Select::Duplicate;
    }
    if (sockets.size() >= maxsize)
    {
        return Select::TooMany;
    }
    if (newel->sock) {
        SDLNet_TCP_AddSocket(mySet, newel->sock);
        numinset++;
    }
    newel->assoc.push_back(this); //We keep info on the selectors in the socket itself

    sockets.insert(newel);

    return Select::Done;
}

template <class T>
Select::Status Selector<T> :: Remove (T &el)
{
    set_iterator it;
    vector_iterator it2;

    if ( (it = sockets.find(el)) == sockets.end())
    {
        return Select::NotFound;
    }
    if ( (it2 = std::find(readysockets.begin(), readysockets.end(), el)) != readysockets.end() )
    {
        *it2 = T(); //removes the socket from readysockets while keeping readysockets valid
    }
    _TCPSocket::BossList::iterator it3 = std::find(el->assoc.begin(),
                el->assoc.end(), this);
    /* normally it should be there, no need to test */
    el->assoc.erase(it3);

    if (el->sock) {
        SDLNet_TCP_DelSocket(mySet, el->sock);
        numinset--;
    }

    return Select::Done;
}

template <class T>
size_t Selector<T>::Wait(size_t timeout)
{
    if (numinset == 0) {
        return 0;
    }

    if(SDLNet_CheckSockets(mySet, timeout) == -1) {
        return 0;
    }

    readysockets.clear();

    set_iterator it;

    for (it = sockets.begin(); it != sockets.end(); ++it)
    {
        if (SDLNet_SocketReady ((*it)->sock))
            readysockets.push_back(*it);
    }

    return readysockets.size(); //We never know, it may not be the same amount as given by SDLNet_CheckSockets...
}

template <class T>
size_t Selector<T>::size() const
{
    return sockets.size();
}

template <class T>
T Selector<T>::GetSocketReady (size_t index)
{
    return readysockets[index];
}


} //namespace interface



#endif // INTERSOCK_HPP_INCLUDED

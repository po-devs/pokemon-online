#ifndef SERVERDATA_H_INCLUDED
#define SERVERDATA_H_INCLUDED

#include <string>
#include <list>
#include "../generic/intersock.hpp"
#include "../generic/interthread.hpp"
#include "../generic/utilities.hh"

void StopAllThreads();
/* thread functions */
int serv_io(void *data);
int play_io(void *data);

struct ServerData;

template <class T>
class SafeList : public std::list <T>, public interface::Mutex
{
};

typedef SafeList<ServerData *> ServList;

/* A SocketTCP with another variable stored in it,
    shared by references by all the copies of this class */
template <class T>
class SocketTCP_suppl : public interface::TCPSocket
{
public:
    typedef T stored_type;

    /* true if there is a var stored, else false */
    bool occupied() const;
    /* get var, return default if none */
    const stored_type * get_var() const;
    stored_type * get_var();
    /* set a whole new var */
    const stored_type & create_var(const stored_type & newvar);
    /* change the var's content */
    const stored_type & change_var(const stored_type & newval);
    /* A shared var */
    void share_var(const SocketTCP_suppl<stored_type> &ref);
    /* remove the var and manages refcount */
    void remove_var();

private:
    smart_ptr <stored_type> myVar;
};

typedef SocketTCP_suppl<bool> SocketTCPb;
typedef SocketTCP_suppl<size_t> SocketTCPid;
/* the server sockets will have in memory the iterator containing the ServerData they are affiliated to */
typedef SocketTCP_suppl<ServList::iterator> ServSocket;
/* the player sockets will have in memory the thread they are affiliated to! */
typedef SocketTCP_suppl<interface::Thread *> PlaySocket;

struct ServerData : public interface::Mutex
{
    ServSocket client;

    //Player data
    uint16_t num_players; //10 bits
    std::string name; //len: 5bits
    std::string description; //len: 9 bits

    ServerData(const ServSocket &client , uint16_t num_players = 0, const std::string &name="", const std::string &description="");
    ~ServerData();

    /* should be only called by the main thread, or at least you should make sure two threads
        can't call it at the same time

        Return:
            -true if success
            -false if another thread is already running
    */
    bool create_thread(interface::Thread::FuncType Function);

    /* This should be quite private from here on */
    interface::Thread *thread; //so the thread can be closed properly when finished
    bool threaded; //false if the thread is ended
};

void serialize(MegaSerializer &ser, const ServerData& s);
bool deserialize(MegaDeserializer &des, ServerData &s);

class Uint8in //Auto init to 0
{
public:
    Uint8in(uint16_t i = 0) : i (i) {}

    operator uint8_t () const
      { return i; }
    Uint8in & operator = (const uint8_t &ref)
      { i = ref; return *this; }
    Uint8in & operator ++ ()
      { ++i; return * this; }
    Uint8in & operator -- ()
      { --i; return * this; }

private:
    uint8_t i;
};

//////////////////////////////////////////////////////////////////
/// ////////////////////////////////////////////////////////// ///
///                    TPP FILE                                ///
/// ////////////////////////////////////////////////////////// ///
//////////////////////////////////////////////////////////////////


template <class T>
bool SocketTCP_suppl<T>::occupied () const
{
    return myVar;
}

template <class T>
const T * SocketTCP_suppl<T>::get_var () const
{
    return myVar;
}

template <class T>
T * SocketTCP_suppl<T>::get_var ()
{
    return myVar;
}

template <class T>
const T & SocketTCP_suppl<T>::create_var(const T &newval)
{
    return *(myVar = new T(newval));
}

template <class T>
const T & SocketTCP_suppl<T>::change_var(const T &newval)
{
    return (*myVar = newval);
}

template <class T>
void SocketTCP_suppl<T>::share_var(const SocketTCP_suppl<T> &ref)
{
    return *(myVar = ref.myVar);
}

template <class T>
void SocketTCP_suppl<T>::remove_var()
{
    myVar.destroy();
}

#endif // SERVERDATA_H_INCLUDED

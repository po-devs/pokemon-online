#include "intersock.hpp"
#include "intermutex.hpp"

namespace interface
{

Mutex IpToString_Lock; //As the SDL function SDLNet_ResolveIP isn't thread-safe

IPAddress::IPAddress()
{
    myIP.host = Invalid;
}

IPAddress::IPAddress(const IPaddress &ip) : myIP (ip)
{
    ;
}

IPAddress::IPAddress(const char *host, uint16_t port)
{
    SDLNet_ResolveHost(&myIP, host, port);
    if (!myIP.host) { //if the host is non-specified, we set it to Invalid
        myIP.host = Invalid;
    }
}

bool IPAddress::IsValid() const
{
    return myIP.host != Invalid;
}

uint16_t IPAddress::Port() const
{
    return myIP.port;
}

uint32_t IPAddress::ToInteger() const
{
    return myIP.host;
}

bool IPAddress::Resolve(const char *host, uint16_t port)
{
    SDLNet_ResolveHost(&myIP, host, port);
    if (!myIP.host) { //if the host is non-specified, we set it to Invalid
        myIP.host = Invalid;
    }

    return IsValid();
}

std::string IPAddress::ToString()
{
    IpToString_Lock.Lock();
    std::string host_name = SDLNet_ResolveIP(&myIP);
    IpToString_Lock.Unlock();

    return host_name;
}

_TCPSocket::_TCPSocket(TCPsocket s) : sock(s), set(NULL)
{
    ;
}

_TCPSocket::~_TCPSocket()
{
    if (set) {
        SDLNet_FreeSocketSet(set), set = NULL;
    }
    if (sock) {
        SDLNet_TCP_Close(sock), sock = NULL;
    }
    /* No need to call remove_from_assoc as the
        destructor wouldn't be called if this socket
        was in any assoc */
}

void _TCPSocket::remove_from_assoc()
{
    BossList::iterator it;

    for (it = assoc.begin(); it != assoc.end(); ++it)
    {
        (*it)->_Remove(this);
    }
    if (set)
        SDLNet_TCP_DelSocket(set, sock);
}

void _TCPSocket::add_to_assoc()
{
    BossList::iterator it;

    for (it = assoc.begin(); it != assoc.end(); ++it)
    {
        (*it)->_Add(this);
    }
    if (set)
        SDLNet_TCP_AddSocket(set, sock);
}

bool _TCPSocket::check_ready() const
{
    if (IsBlocking())
        return true;
    /*
    shouldn't occur:
    if (sock == NULL)
        return true;
    */
    SDLNet_CheckSockets(set,0);
    return SDLNet_SocketReady(sock);
}


void _TCPSocket::Close()
{
    if (sock != NULL)
    {
        remove_from_assoc();
        SDLNet_TCP_Close(sock);
        sock = NULL;
    }
}

void _TCPSocket::SetBlocking(bool blocking)
{
    if (blocking == IsBlocking())
        return;

    if (!blocking) {
        set = SDLNet_AllocSocketSet(1);
        if (sock)
            SDLNet_TCP_AddSocket(set, sock);
    } else {
        SDLNet_FreeSocketSet(set);
        set = NULL;
    }
}

bool _TCPSocket::IsBlocking() const
{
    return !set;
}

Socket::Status _TCPSocket::Receive (char *data, size_t maxsize, size_t &recvd_size)
{
    if (!sock)
        return Socket::Error;
    if (!check_ready())
        return Socket::NotReady;

    int status = SDLNet_TCP_Recv(sock, data, maxsize);

    if (status <= 0) {
        recvd_size = 0;
        Close();
        return Socket::Error;
    }

    recvd_size = status;

    return Socket::Done;
}

Socket::Status _TCPSocket::Send (const char *data, size_t size)
{
    if (!sock)
        return Socket::Error;

    int status = SDLNet_TCP_Send (sock, data, size);

    if (status < signed(size)) {
        Close();
        return Socket::Error;
    }

    return Socket::Done;
}

Socket::Status _TCPSocket::Listen (uint16_t port)
{
    Close();

    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, NULL, port) != 0)
        return Socket::Error;

    sock = SDLNet_TCP_Open(&ip);

    if (!sock) {
        return Socket::Error;
    }

    add_to_assoc();

    return Socket::Done;
}

Socket::Status _TCPSocket::Accept(TCPSocket &other)
{
    if (!sock)
        return Socket::Error;
    if (!check_ready())
        return Socket::NotReady;

    other = TCPSocket(SDLNet_TCP_Accept(sock));

    if (!other)
        return Socket::Error;

    return Socket::Done;
}

Socket::Status _TCPSocket::Connect(const char *host, uint32_t port)
{
    IPAddress ip(host, port);
    return Connect(ip);
}

Socket::Status _TCPSocket::Connect(IPAddress &ip)
{
    Close();

    sock = SDLNet_TCP_Open(&ip.myIP);

    if (!sock)
        return Socket::Error;

    add_to_assoc();

    return Socket::Done;
}

_TCPSocket::operator bool()
{
    return sock;
}

IPAddress _TCPSocket::GetIp() const
{
    if (!sock) {
        return IPAddress();
    }

    IPaddress *ip =  SDLNet_TCP_GetPeerAddress(const_cast<TCPsocket> (sock) );

    if (!ip) {
        return IPAddress();
    } else {
        return IPAddress(*ip);
    }
}

//ToDo: out of memory...
_BaseSelector::_BaseSelector(size_t maxsockets) : mySet(SDLNet_AllocSocketSet(maxsockets)), maxsize(maxsockets), numinset(0)
{
    ;
}

_BaseSelector::~_BaseSelector()
{
    SDLNet_FreeSocketSet(mySet);
}

/* Do not use this function in any other member function as it's virtual and can be changed
    (I especially think to the disaster it would cause if used in SafeSelector)

   Those functions are only meant as friend functions of _TCPSocket. */
void _BaseSelector::_Add(_TCPSocket *ptr)
{
    SDLNet_TCP_AddSocket(mySet, ptr->sock);
    numinset++;
}

void _BaseSelector::_Remove(_TCPSocket *ptr)
{
    SDLNet_TCP_DelSocket(mySet, ptr->sock);
    numinset--;
}


} //interface

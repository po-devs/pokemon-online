#ifdef BOOST_SOCKETS

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "asiosocket.h"

/* The deleteObjectLater destructor ensures the objects are destroyed in
   the main thread instead of the local thread to the SocketManager.

   This prevents possible calls to SocketSQ's slots to have multithreading problems */
struct deleteObjectLater {
    void operator () (QObject *q) {
        qDebug() << "Deleting object later " << q;
        q->QObject::deleteLater();
    }
};


using boost::asio::ip::tcp;
using namespace boost::asio;

SocketManager::SocketManager() {
    finished = false;
}

SocketManager::~SocketManager() {
    finished = true;

    /* Wait till the thread finished */
    while (finished) {
    }
}

void SocketManager::run() {
    while (!finished) {
        io_service.reset();

        boost::system::error_code ec;

        io_service.run_one(ec);
    }

    finished = false;
}

SocketSQ::pointer SocketManager::createSocket() {
    return SocketSQ::pointer(new SocketSQ(this, new tcp::socket(io_service)), deleteObjectLater());
}

SocketSQ::pointer SocketManager::createServerSocket() {
    return SocketSQ::pointer(new SocketSQ(this, new tcp::acceptor(io_service)), deleteObjectLater());
}

SocketSQ::SocketSQ(SocketManager *manager, tcp::socket *s) : mysock(s), manager(manager), m(QMutex::Recursive), bufCounter(0), notifiedDced(false)
{
    isServer = false;
    incoming = NULL;
    freeConnection = true;
}

SocketSQ::SocketSQ(SocketManager *manager, tcp::acceptor *s) : myserver(s), manager(manager), bufCounter(0), notifiedDced(false)
{
    isServer = true;
    incoming = NULL;
    freeConnection = true;

    incoming = new tcp::socket(manager->io_service);
}

SocketSQ::~SocketSQ() {
    qDebug() << "Starting to delete SocketSQ " << this;
    if (isServer) {
        delete myserver;
        delete incoming;
    } else {
        delete mysock;
    }
    qDebug() << "Deleted SocketSQ " << this;
}

bool SocketSQ::listen(quint16 port, char *ip)
{
    server().open(tcp::v4());
    try {
        if (ip) {
            server().bind(tcp::endpoint(ip::address::from_string("127.0.0.1"), port));
        } else {
            server().bind(tcp::endpoint(tcp::v4(), port));
        }
    }
    catch(std::exception &e) {
        qDebug() << "Socket listen error: " << e.what();
        return false;
    }
    boost::system::error_code ec;

    bool ret;

    ret = !server().listen(boost::asio::socket_base::max_connections, ec);
    server().async_accept(*incoming, boost::bind(&SocketSQ::acceptHandler, this, boost::asio::placeholders::error));

    return ret;
}

/* The function is necessary because it's overloading the base deleteLater, which we don't want
   called until all the handlers are finished (they may not be finished even after a disconnect.

   The SocketSQ is always handled with SocketSQ::pointer, so it's deleted with the shared ptr system */
void SocketSQ::deleteLater()
{
}

SocketSQ::pointer SocketSQ::nextPendingConnection()
{
    if (!isServer)
        return SocketSQ::pointer();
    if (freeConnection)
        return SocketSQ::pointer();
    SocketSQ::pointer ret = pointer(new SocketSQ(manager, incoming), deleteObjectLater());
    boost::system::error_code ec;
    ret->myip = QString::fromStdString(incoming->remote_endpoint(ec).address().to_string());

    incoming = new tcp::socket(manager->io_service);
    freeConnection = true;
    server().async_accept(*incoming, boost::bind(&SocketSQ::acceptHandler, this, boost::asio::placeholders::error));

    /* Avoids to start dead sockets */
    if (ec) {
        return SocketSQ::pointer();
    }

    ret->start();    
    return ret;
}

void SocketSQ::start() {
    /* Starts the receiving loop */
    readHandler(boost::system::error_code(), 0);
}

void SocketSQ::disconnectFromHost()
{
    sock().close();
}

tcp::socket &SocketSQ::sock()
{
    return* mysock;
}

const tcp::socket &SocketSQ::sock() const
{
    return *mysock;
}

tcp::acceptor &SocketSQ::server()
{
    return *myserver;
}

const tcp::acceptor &SocketSQ::server() const
{
    return *myserver;
}

int SocketSQ::bytesAvailable()
{
    QMutexLocker l(&m);

    if (notifiedDced)
        return 0;

    return buffer.size()-bufCounter;
}

void SocketSQ::readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec) {
        if (!notifiedDced) {
            notifiedDced = true;
            emit disconnected();
        }
        return;
    }

    if (bytes_transferred > 0) {
        m.lock();
        buffer.append(innerBuffer, bytes_transferred);
        m.unlock();

        emit active();
    }

    sock().async_read_some(boost::asio::buffer(innerBuffer, 10000),
                      boost::bind(&SocketSQ::readHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void SocketSQ::writeHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec) {
        if (!notifiedDced) {
            notifiedDced = true;
            emit disconnected();
        }
        return;
    }

    QMutexLocker l(&m);

    sending = sending.right(sending.size() - bytes_transferred) + toSend;
    toSend.clear();

    if (sending.size() == 0)
        return;

    sock().async_send(boost::asio::buffer(sending.data(), sending.size()),
                      boost::bind(&SocketSQ::writeHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void SocketSQ::acceptHandler(const boost::system::error_code& ec)
{
    if (ec) {
        boost::asio::detail::throw_error(ec);
        server().async_accept(*incoming, boost::bind(&SocketSQ::acceptHandler, this, boost::asio::placeholders::error));
        return;
    }

    freeConnection = false;
    emit active();
}

void SocketSQ::getChar(char *ch) {
    QMutexLocker l(&m);

    if (buffer.size() - bufCounter <= 0)
        return;

    if (ch)
        *ch = buffer[bufCounter];
    bufCounter += 1;
}

QByteArray SocketSQ::read(int length)
{
    QMutexLocker l(&m);

    QByteArray ret;
    if (bufCounter == 0)
        ret = buffer;
    else {
        ret = buffer.mid(bufCounter, length);
        buffer.remove(0, bufCounter+length);
        bufCounter = 0;
    }
    return ret;
}

void SocketSQ::putChar(char c)
{
    QMutexLocker l(&m);
    toSend.append(c);
}

void SocketSQ::setLowDelay(bool lowDelay)
{
    boost::asio::ip::tcp::no_delay option(lowDelay);
    sock().set_option(option);
}

void SocketSQ::write(const QByteArray &b)
{
    QMutexLocker l(&m);
    if (toSend.length() == 0)
        toSend = b;
    else
        toSend.append(b);

    sendData();
}

QString SocketSQ::ip()
{
    return myip;
}

void SocketSQ::sendData()
{
    QMutexLocker l(&m);

    if (sending.size() == 0 && toSend.size() > 0) {
        writeHandler(boost::system::error_code(), 0);
        return;
    }
}

#endif

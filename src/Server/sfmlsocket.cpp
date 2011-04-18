#include <boost/bind.hpp>
#include "sfmlsocket.h"

using boost::asio::ip::tcp;

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
        boost::system::error_code ec;
        io_service.run_one(ec);
    }

    finished = false;
}

SocketSQ* SocketManager::createSocket() {
    return new SocketSQ(this, new tcp::socket(io_service));
}

SocketSQ* SocketManager::createServerSocket() {
    return new SocketSQ(this, new tcp::acceptor(io_service));
}

SocketSQ::SocketSQ(SocketManager *manager, tcp::socket *s) : mysock(s), manager(manager), m(QMutex::Recursive), bufCounter(0), notifiedDced(false)
{
    isServer = false;
    incoming = NULL;
    freeConnection = true;

    /* Starts the receiving loop */
    readHandler(boost::system::error_code(), 0);
}

SocketSQ::SocketSQ(SocketManager *manager, tcp::acceptor *s) : myserver(s), manager(manager), bufCounter(0), notifiedDced(false)
{
    isServer = true;
    incoming = NULL;
    freeConnection = true;

    incoming = new tcp::socket(manager->io_service);
    server().async_accept(*incoming, endpoint,
                          boost::bind(&SocketSQ::acceptHandler, this, boost::asio::placeholders::error));
}

SocketSQ::~SocketSQ() {
    if (isServer) {
        delete myserver;
        delete incoming;
    } else {
        delete mysock;
    }
}

bool SocketSQ::listen(quint16 port)
{
    server().bind(tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    boost::system::error_code ec;

    return server().listen(boost::asio::socket_base::max_connections, ec);
}

void SocketSQ::deleteLater()
{
    if (!notifiedDced) {
        notifiedDced = true;
        emit disconnected();
    }

    QObject::deleteLater();
}

SocketSQ* SocketSQ::nextPendingConnection()
{
    if (!isServer)
        return NULL;
    if (freeConnection)
        return NULL;
    SocketSQ *ret = new SocketSQ(manager, incoming);
    incoming = new tcp::socket(manager->io_service);
    server().async_accept(*incoming, endpoint,
                          boost::bind(&SocketSQ::acceptHandler, this, boost::asio::placeholders::error));

    ret->myip = QString::fromStdString(endpoint.address().to_string());

    freeConnection = true;

    return ret;
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

void SocketSQ::delayDeath()
{
    QObject::deleteLater();
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
        deleteLater();
        return;
    }

    m.lock();
    buffer.append(innerBuffer, bytes_transferred);
    m.unlock();

    emit active();

    sock().async_read_some(boost::asio::buffer(innerBuffer, 10000),
                      boost::bind(&SocketSQ::readHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void SocketSQ::writeHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec) {
        deleteLater();
        return;
    }

    (void) bytes_transferred;

    QMutexLocker l(&m);
    sending = toSend;
    toSend.clear();

    if (sending.size() == 0)
        return;

    boost::asio::async_write(sock(), boost::asio::buffer(sending.data(), sending.size()),
                      boost::bind(&SocketSQ::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void SocketSQ::acceptHandler(const boost::system::error_code& ec)
{
    (void) ec;

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
    toSend.append(c);
}

void SocketSQ::write(const QByteArray &b)
{
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

#include "sfmlsocket.h"

QString stt(int status) {
    if (status == sf::Socket::Disconnected) {
        return "Disconnected";
    } else if (status == sf::Socket::Done) {
        return "Done";
    } else if (status == sf::Socket::Error) {
        return "Error";
    } else if (status == sf::Socket::NotReady) {
        return "Not Ready";
    } else {
        return QString("Unexpected Status: %1").arg(status);
    }
}

QString btt(const QByteArray &b) {
    QStringList bytes;

    int byte = -1;
    int count = 0;

    foreach(char c, b) {
        if (byte == -1) {
            byte = c;
            count = 1;
        } else {
            if (byte == c) {
                count ++;
            } else {
                bytes.push_back(QString("%1 times the byte %2").arg(count).arg(byte));
                byte = c;
                count = 1;
            }
        }
    }
    bytes.push_back(QString("%1 times the byte %2").arg(count).arg(byte));

    return bytes.join(", ");
}

SocketManager::SocketManager() {
    finished = false;
}

SocketManager::~SocketManager() {
    finished = true;

    /* Wait till the thread finished */
    while (finished) {
    }

    foreach (SocketSQ *s, heap) {
        delete s;
    }
}

void SocketManager::Run() {
    while (!finished) {
        sf::Sleep(0.010);

        foreach(SocketSQ *s, socketsToSend) {
            s->sendData();
        }

        foreach(SocketSQ *s, heap) {
            s->fill();
        }

        lock.Lock();
        if (socketsToAdd.size() > 0) {
            foreach(SocketSQ *s, socketsToAdd) {
                heap.insert(s);
            }
            socketsToAdd.clear();
        }
        if (socketsToRemove.size() > 0) {
            foreach(SocketSQ *s, socketsToRemove) {
                if (!existSocket(s))
                    continue;
                heap.remove(s);
                s->delayDeath();
            }
            socketsToRemove.clear();
        }
        socketsToSend = waitingList;
        waitingList.clear();
        lock.Unlock();
    }

    finished = false;
}

SocketSQ* SocketManager::createSocket() {
    return new SocketSQ(this);
}

void SocketManager::addSocket(SocketSQ *s) {
    sf::Lock l(lock);

    socketsToAdd.append(s);
}

void SocketManager::deleteSocket(SocketSQ *s) {
    sf::Lock l(lock);

    socketsToRemove.append(s);
}

bool SocketManager::existSocket(SocketSQ* s) const {
    return heap.contains(s);
}

void SocketManager::addSendingSocket(SocketSQ *s) {
    sf::Lock l(lock);

    if(existSocket(s)) {
        waitingList.append(s);
    }
}

SocketSQ::SocketSQ(SocketManager *manager, sf::SocketTCP s) : mysock(s), manager(manager), bufCounter(0), notifiedDced(false)
{
    isServer = false;
    incoming = NULL;
    freeConnection = true;

    sock().SetBlocking(false);
    manager->addSocket(this);
}

bool SocketSQ::listen(quint16 port)
{
    isServer = true;
    return sock().Listen(port);
}

void SocketSQ::deleteLater()
{
    if (!notifiedDced) {
        notifiedDced = true;
        emit disconnected();
    }

    manager->deleteSocket(this);
}

SocketSQ* SocketSQ::nextPendingConnection()
{
    if (!isServer)
        return NULL;
    if (freeConnection)
        return NULL;
    SocketSQ *ret = incoming;

    freeConnection = true;

    return ret;
}

void SocketSQ::disconnectFromHost()
{
    sock().Close();
}

sf::SocketTCP &SocketSQ::sock()
{
    return mysock;
}

const sf::SocketTCP &SocketSQ::sock() const
{
    return mysock;
}

void SocketSQ::delayDeath()
{
    QObject::deleteLater();
}

int SocketSQ::bytesAvailable()
{
    sf::Lock l(m);

    if (notifiedDced)
        return 0;

    return buffer.size()-bufCounter;
}

void SocketSQ::fill()
{
    if (!isServer) {
        m.Lock();

        char buf [1024];
        std::size_t length;
        sf::Socket::Status st = sf::Socket::NotReady;
        bool filled = false;
        while (buffer.size() <= 100000 && ((st = sock().Receive (buf,1024, length)) == sf::Socket::Done)) {
            qDebug() << "Received " << btt(QByteArray(buf, length));
            filled = true;
            buffer.append(buf, length);
        }

        buffer.clear();
        qDebug() << "status: " << stt(st);

        m.Unlock();

        if (!sock().IsValid() || st == sf::Socket::Disconnected) {
            if (!notifiedDced) {
                notifiedDced = true;
                emit disconnected();
            }
            exit(0);
        } else {
            if (filled){
                emit active();
            }
        }
    } else {
        if (freeConnection) {
            sf::SocketTCP newsock;
            sf::IPAddress ip;

            if (sock().Accept(newsock,&ip) == sf::Socket::Done) {
                incoming = new SocketSQ(manager, newsock);

                incoming->myip = QString::fromStdString(ip.ToString());
                freeConnection = false;

                emit active();
            }
        }
    }
}

void SocketSQ::getChar(char *ch) {
    sf::Lock l(m);

    if (buffer.size() - bufCounter <= 0)
        return;

    if (ch)
        *ch = buffer[bufCounter];
    bufCounter += 1;
}

QByteArray SocketSQ::read(int length)
{
    sf::Lock l(m);

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
    int remaining = toSend.size();
    sf::Socket::Status st = sf::Socket::Done;

    for (int counter = 0; counter < toSend.size(); counter += 1024) {
        qDebug() << "Sending " + btt(QByteArray(toSend.data()+counter, std::min(remaining, 1024)));
        st = sock().Send(toSend.data()+counter, std::min(remaining, 1024));

        if (st != sf::Socket::Done) {
            qDebug() << "status: " << stt(st);
            break;
        }

        remaining = std::max(remaining - 1024, 0);
    }

    if (remaining == 0) {
        toSend.clear();
        exit(0);
    }
    else
        toSend.remove(0, toSend.size() - remaining);

    if (!sock().IsValid() || st == sf::Socket::Disconnected) {
        notifiedDced = true;
        emit disconnected();
    } else if (remaining > 0) {
        manager->addSendingSocket(this);
    }
}

bool SocketSQ::connectTo(quint16 port, const std::string &ip)
{
    bool success;

    sock().SetBlocking(true);
    success = !sock().Connect(port, sf::IPAddress(ip));
    sock().SetBlocking(false);

    return success;
}

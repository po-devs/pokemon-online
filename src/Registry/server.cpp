#include "../Utilities/otherwidgets.h"
#include "analyze.h"
#include "server.h"

Server::Server(int _id, QTcpSocket *s)
{
    id() = _id;
    ip() = s->peerAddress().toString();
    listed() = false;

    m_relay = new Analyzer(s, id());
    m_relay->setParent(this);

    m_validator = new QNickValidator(NULL);
    m_validator->setParent(this);

    connect(m_relay, SIGNAL(loggedIn(QString,QString,quint16)), SLOT(login(QString,QString,quint16)));
    connect(m_relay, SIGNAL(nameChange(QString)), SLOT(nameChanged(QString)));
    connect(m_relay, SIGNAL(numChange(quint16)), SLOT(numChanged(quint16)));
    connect(m_relay, SIGNAL(descChange(QString)), SLOT(descChanged(QString)));
    connect(m_relay, SIGNAL(maxChange(quint16)), SLOT(maxChanged(quint16)));
    connect(m_relay, SIGNAL(disconnected()), SLOT(disconnected()));
}

void Server::login(const QString &name, const QString &desc, quint16 num, quint16 max)
{
    descChanged(desc);
    numChanged(num);
    nameChanged(name);
    maxChanged(max);
}

void Server::login(const QString &name, const QString &desc, quint16 num)
{
    descChanged(desc);
    numChanged(num);
    nameChanged(name);
}

void Server::descChanged(const QString &desc)
{
    this->desc() = desc.left(400).trimmed();
}

void Server::numChanged(quint16 num)
{
    players() = num;
}

void Server::nameChanged(const QString &name)
{
    if (m_validator->validate(name) != QValidator::Acceptable) {
        m_relay->sendInvalidName();
        return;
    }

    if (name == this->name())
        return;

    emit nameChangedReq(id(), name);
}

void Server::maxChanged(const quint16 max)
{
    this->maxPlayers() = max;
}

void Server::refuseIP()
{
    m_relay->notify(NetworkReg::Logout);
}

void Server::refuseName()
{
    m_relay->sendNameTaken();
}

void Server::accept()
{
    m_relay->sendAccept();
    listed() = true;
}

void Server::disconnected()
{
    emit disconnection(id());
}

void Server::kick()
{
    m_relay->close();
}

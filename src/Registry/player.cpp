#include "server.h"
#include "player.h"
#include "analyze.h"

Player::Player(int _id, QTcpSocket *s)
{
    id() = _id;
    ip() = s->peerAddress().toString();

    m_relay = new Analyzer(s, id());
    m_relay->setParent(this);

    connect(m_relay, SIGNAL(disconnected()), SLOT(disconnected()));
}

void Player::disconnected()
{
    emit disconnection(id());
}

void Player::kick()
{
    m_relay->close();
}

void Player::sendRegistryAnnouncement(const QString &announcement) {
    m_relay->sendRegistryAnnouncement(announcement);
}

void Player::sendServer(const Server &s)
{
    if(s.port() == 0)
        m_relay->sendServer(s.name(), s.desc(), s.players(), s.ip(),s.maxPlayers(),5080, s.passwordProtected());
    else
        m_relay->sendServer(s.name(), s.desc(), s.players(), s.ip(),s.maxPlayers(),s.port(), s.passwordProtected());
}

void Player::sendServerListEnd()
{
  m_relay->sendServerListEnd();
}

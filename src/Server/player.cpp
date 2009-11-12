#include "player.h"

Player::Player(QTcpSocket *sock) : myrelay(sock)
{
    m_isLoggedIn = false;
    m_isChallenged = false;

    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(TeamInfo)), this, SLOT(loggedIn(TeamInfo)));
    connect(&relay(), SIGNAL(messageReceived(QString)), this, SLOT(recvMessage(QString)));
    connect(&relay(), SIGNAL(teamReceived(TeamInfo)), this, SLOT(recvTeam(TeamInfo)));
    connect(&relay(), SIGNAL(challengeReceived(int)), this, SLOT(challengeReceived(int)));
    connect(&relay(), SIGNAL(challengeRefused(int)), this, SLOT(challengeRefused(int)));
    connect(&relay(), SIGNAL(challengeAccepted(int)), this, SLOT(challengeAccepted(int)));
    connect(&relay(), SIGNAL(busyForChallenge(int)), this, SLOT(busyForChallenge(int)));
}

Player::~Player()
{
    cancelChallenges();
}

bool Player::connected() const
{
    return relay().isConnected();
}

bool Player::isChallenged() const
{
    return m_isChallenged;
}

bool Player::hasChallenged() const
{
    return !m_challenged.empty();
}

int Player::challengedBy() const
{
    return m_challengedby;
}

bool Player::isLoggedIn() const
{
    return m_isLoggedIn;
}

void Player::setLoggedIn(bool logged)
{
    m_isLoggedIn = logged;
}

void Player::disconnected()
{
    emit disconnected(id());
}

void Player::sendChallengeCancel(int id)
{
    relay().sendCancelChallenge(id);
}

BasicInfo Player::basicInfo() const
{
    BasicInfo ret = {team().name, team().info};
    return ret;
}

void Player::recvMessage(const QString &mess)
{
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), mess);
}

Analyzer & Player::relay()
{
    return myrelay;
}

const Analyzer & Player::relay() const
{
    return myrelay;
}

void Player::loggedIn(const TeamInfo &_team)
{
    team() = _team;

    emit loggedIn(id(), _team.name);
}

QString Player::name() const
{
    return team().name;
}

void Player::setId(int id)
{
    myid = id;
}

void Player::recvTeam(const TeamInfo &team)
{
    this->team() = team;

    emit recvTeam(id());
}

int Player::id() const
{
    return myid;
}

void Player::challengeReceived(int id)
{
    if (!isLoggedIn() || id == this->id()) {
	return;
    }

    emit challengeFromTo(this->id(), id);
}

void Player::challengeIssued(int id)
{
    m_challenged.insert(id);
}

bool Player::challenge(int idto)
{
    if (isChallenged())
	return false;

    relay().sendChallenge(idto);
    m_isChallenged = true;
    m_challengedby = idto;

    return true;
}

void Player::busyForChallenge(int id)
{
    if (!isLoggedIn() || id == this->id() || !isChallenged() || challengedBy() != id) {
	return;
    }

    emit busyForChallenge(this->id(), id);
}

void Player::challengeAccepted(int id)
{
    if (!isLoggedIn() || id == this->id()) {
	return;
    }
    if (!isChallenged()) {
	sendMessage(tr("You are not challenged by anyone"));
    }
    if (challengedBy() != id) {
	sendMessage(tr("You are not challenged by that player"));
    }
    emit challengeAcc(this->id(), id);

    m_isChallenged = false;
}

void Player::challengeRefused(int id)
{
    if (!isLoggedIn() || id == this->id() || !isChallenged() || challengedBy() != id) {
	return;
    }

    emit challengeRef(this->id(), id);

    m_isChallenged = false;
}

void Player::removeChallenge(int id)
{
    m_challenged.remove(id);
}

void Player::sendBusyForChallenge(int id)
{
    relay().sendBusyForChallenge(id);
    removeChallenge(id);
}

void Player::sendMessage(const QString &mess)
{
    relay().sendMessage(mess);
}

void Player::startBattle(int id)
{
    relay().sendMessage(tr("Fake battle started with player %1").arg(id));

    if (isChallenged() && challengedBy() != id) {
	emit busyForChallenge(this->id(), id);
    }
    m_isChallenged = false;
    cancelChallenges();
}

void Player::cancelChallenges()
{
    foreach(int id, m_challenged)
	emit challengeCanceled(this->id(), id);
    m_challenged.clear();
}

void Player::sendChallengeRefusal(int id)
{
    relay().sendRefuseChallenge(id);
    removeChallenge(id);
}

TeamInfo & Player::team()
{
    return myteam;
}

const TeamInfo & Player::team() const
{
    return myteam;
}


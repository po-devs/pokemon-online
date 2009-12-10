#include "player.h"
#include "../PokemonInfo/battlestructs.h"

Player::Player(QTcpSocket *sock) : myrelay(sock)
{
    m_state = NotLoggedIn;
    m_challengedby = -1;

    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(TeamInfo)), this, SLOT(loggedIn(TeamInfo)));
    connect(&relay(), SIGNAL(messageReceived(QString)), this, SLOT(recvMessage(QString)));
    connect(&relay(), SIGNAL(teamReceived(TeamInfo)), this, SLOT(recvTeam(TeamInfo)));
    connect(&relay(), SIGNAL(challengeStuff(int,int)), this, SLOT(challengeStuff(int,int)));
    connect(&relay(), SIGNAL(forfeitBattle()), SLOT(battleForfeited()));
    connect(&relay(), SIGNAL(battleMessage(BattleChoice)), SLOT(battleMessage(BattleChoice)));
    connect(&relay(), SIGNAL(battleChat(QString)), SLOT(battleChat(QString)));
}

Player::~Player()
{
    cancelChallenges();
    if (battling())
	battleForfeited();
}

void Player::changeState(int newstate)
{
    m_state = newstate;

    if (m_state == Battling)
    {
	cancelChallenges();
    }
}

void Player::disconnected()
{
    emit disconnected(id());
}

void Player::battleChat(const QString &s)
{
    emit battleChat(id(), s);
}

void Player::battleMessage(const BattleChoice &b)
{
    emit battleMessage(id(), b);
}

void Player::recvMessage(const QString &mess)
{
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), mess);
}

bool Player::challenge(int idto)
{
    if (state() != LoggedIn)
	return false;

    relay().sendChallengeStuff(Sent, idto);

    changeState(Challenged);
    m_challengedby = idto;

    return true;
}

void Player::battleForfeited()
{
    if (!battling()) {
	return;
    }

    changeState(LoggedIn);

    emit battleFinished(Forfeit, opponent(), id());
}

void Player::battleResult(int result)
{
    relay().sendBattleResult(result);
    changeState(LoggedIn);
}

int Player::opponent() const
{
    return m_opponent;
}

void Player::challengeStuff(int desc, int id)
{
    if (!isLoggedIn() || id == this->id()) {
	// INVALID BEHAVIOR
	return;
    }

    if (desc != Sent && desc != Canceled)
    {
	if (!isChallenged()) {
	    // INVALID BEHAVIOR
	    return;
	}
	if (challengedBy() != id) {
	    // INVALID BEHAVIOR
	    return;
	}
    } else if (desc == Sent) {
	if (battling()) {
	    // INVALID BEHAVIOR
	    return;
	}
    } else if (desc == Canceled) {
	if (!hasChallenged(id)) {
	    // INVALID BEHAVIOR
	    return;
	}
    }

    if (desc == Sent) {
	addChallenge(id);
    } else if (desc != Accepted) {
	changeState(LoggedIn);
    }

    emit challengeStuff(desc, this->id(), id);
}

void Player::sendChallengeStuff(int stuff, int other)
{
    /* This is either Canceled, Refused, or Busied */
    if (stuff == Canceled) {
	changeState(LoggedIn);
    } else {
	removeChallenge(other);
    }
    relay().sendChallengeStuff(stuff, other);
}

void Player::startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    relay().engageBattle(id, team, conf);

    m_opponent = id;

    if (isChallenged() && challengedBy() == id) {
	m_challengedby = -1;
    }

    changeState(Battling);

    removeChallenge(id);
}

void Player::cancelChallenges()
{
    foreach(int id, m_challenged)
	emit challengeStuff(Canceled, this->id(), id);
    m_challenged.clear();
    if (isChallenged()) {
	emit challengeStuff(Busy, this->id(), opponent());
    }
}

bool Player::hasChallenged(int id) const
{
    return m_challenged.contains(id);
}

void Player::addChallenge(int id)
{
    m_challenged.insert(id);
}

void Player::removeChallenge(int id)
{
    m_challenged.remove(id);
}

TeamInfo & Player::team()
{
    return myteam;
}

const TeamInfo & Player::team() const
{
    return myteam;
}

Analyzer & Player::relay()
{
    return myrelay;
}

const Analyzer & Player::relay() const
{
    return myrelay;
}

bool Player::battling() const
{
    return state() == Battling;
}

int Player::state() const
{
    return m_state;
}

bool Player::connected() const
{
    return relay().isConnected();
}

bool Player::isChallenged() const
{
    return m_state == Challenged;
}

int Player::challengedBy() const
{
    return m_challengedby;
}

bool Player::isLoggedIn() const
{
    return m_state != NotLoggedIn;
}

int Player::id() const
{
    return myid;
}

BasicInfo Player::basicInfo() const
{
    BasicInfo ret = {team().name, team().info};
    return ret;
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
    cancelChallenges();

    this->team() = team;

    emit recvTeam(id());
}

void Player::sendMessage(const QString &mess)
{
    relay().sendMessage(mess);
}

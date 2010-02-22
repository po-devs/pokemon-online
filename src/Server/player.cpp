#include "player.h"
#include "security.h"
#include "challenge.h"
#include "../PokemonInfo/battlestructs.h"

Player::Player(QTcpSocket *sock, int id) : myrelay(sock, id), myid(id)
{
    battle = NULL;
    challengedBy = NULL;
    myip = relay().ip();

    m_state = NotLoggedIn;
    myauth = 0;

    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(TeamInfo)), SLOT(loggedIn(TeamInfo)));
    connect(&relay(), SIGNAL(messageReceived(QString)), SLOT(recvMessage(QString)));
    connect(&relay(), SIGNAL(teamReceived(TeamInfo)), SLOT(recvTeam(TeamInfo)));
    connect(&relay(), SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(&relay(), SIGNAL(forfeitBattle()), SLOT(battleForfeited()));
    connect(&relay(), SIGNAL(battleMessage(BattleChoice)), SLOT(battleMessage(BattleChoice)));
    connect(&relay(), SIGNAL(battleChat(QString)), SLOT(battleChat(QString)));
    connect(&relay(), SIGNAL(sentHash(QString)), SLOT(hashReceived(QString)));
    connect(&relay(), SIGNAL(wannaRegister()), SLOT(registerRequest()));
    connect(&relay(), SIGNAL(kick(int)), SLOT(playerKick(int)));
    connect(&relay(), SIGNAL(ban(int)), SLOT(playerBan(int)));
    connect(&relay(), SIGNAL(banRequested(QString)), SLOT(CPBan(QString)));
    connect(&relay(), SIGNAL(unbanRequested(QString)), SLOT(CPUnban(QString)));
    connect(&relay(), SIGNAL(PMsent(int,QString)), SLOT(receivePM(int,QString)));
    connect(&relay(), SIGNAL(getUserInfo(QString)), SLOT(userInfoAsked(QString)));
    connect(&relay(), SIGNAL(banListRequested()), SLOT(giveBanList()));
    connect(&relay(), SIGNAL(awayChange(bool)), SLOT(awayChange(bool)));
    connect(&relay(), SIGNAL(battleSpectateRequested(int)), SLOT(spectatingRequested(int)));
    connect(&relay(), SIGNAL(battleSpectateEnded(int)), SLOT(quitSpectating(int)));
    connect(&relay(), SIGNAL(battleSpectateChat(int,QString)), SLOT(spectatingChat(int,QString)));
}

Player::~Player()
{
}

void Player::doWhenDC()
{
    cancelChallenges();
    if (battling())
        battleForfeited();
    foreach(int id, battlesSpectated) {
        quitSpectating(id);
    }
}

void Player::quitSpectating(int battleId)
{
    if (battlesSpectated.contains(battleId)) {
        battlesSpectated.remove(battleId);
        emit spectatingStopped(this->id(), battleId);
    }
}

void Player::spectateBattle(const QString &name0, const QString &name1, int battleId)
{
    battlesSpectated.insert(battleId);
    relay().notify(NetworkServ::SpectateBattle, name0, name1, qint32(battleId));
}

void Player::cancelChallenges()
{
    if (challengedBy != NULL) {
        challengedBy->cancel(this);
    }
    while (challenged.size() != 0) {
        (*challenged.begin())->cancel(this);
    }
}

void Player::removeChallenge(Challenge *c)
{
    if (challengedBy == c) {
        challengedBy = NULL;
    } else {
        challenged.remove(c);
    }
}

void Player::addChallenge(Challenge *c, bool youarechallenged)
{
    if (youarechallenged) {
        challengedBy = c;
    } else {
        challenged.insert(c);
    }
}

bool Player::okForChallenge(int src) const
{
    if (!isLoggedIn() || battling() || away())
        return false;

    /* If already challenged by someone */
    if (challengedBy != NULL) {
        return false;
    }
    /* If already challenged that same person */
    foreach(Challenge *c, challenged) {
        if (c->challenged() == src) {
            return false;
        }
    }

    return true;
}

bool Player::okForBattle() const
{
    return isLoggedIn() && !battling();
}

void Player::awayChange(bool away)
{
    if (away == this->away()) {
        return;
    }

    if (!isLoggedIn() || battling()) {
        return;
    }

    changeState(Away, away);
    emit awayChange(id(), away);
}

void Player::changeState(int newstate, bool on)
{
    if (on) {
        m_state |= newstate;
    } else {
        m_state &= 0xFF ^ newstate;
    }
}

int Player::auth() const {
    return myauth;
}

void Player::setAuth(int auth)  {
    myauth = auth;
}

void Player::setName(const QString &newname)  {
    team().name = newname;
}

void Player::kick() {
    relay().close();
}

void Player::disconnected()
{
    emit disconnected(id());
}

void Player::battleChat(const QString &s)
{
    if (!isLoggedIn() || !battling())
        return; //INVALID BEHAVIOR
    emit battleChat(id(), s);
}

void Player::spectatingChat(int id, const QString &chat)
{
    if (!battlesSpectated.contains(id)) {
        return; //INVALID BEHAVIOR
    }
    emit spectatingChat(this->id(), id, chat);
}

void Player::battleMessage(const BattleChoice &b)
{
    if (!isLoggedIn())
        return; //INVALID BEHAVIOR
    emit battleMessage(id(), b);
}

void Player::recvMessage(const QString &mess)
{
    if (!isLoggedIn())
        return; //INVALID BEHAVIOR
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), mess);
}

void Player::battleForfeited()
{
    if (!battling()) {
        return; //INVALID BEHAVIOR
    }

    changeState(LoggedIn, true);

    emit battleFinished(Forfeit, opponent(), id());
}

void Player::battleResult(int result, int winner, int loser)
{
    relay().sendBattleResult(result, winner, loser);

    if ((winner == id() || loser == id()) && (result == Forfeit || result == Close))
        changeState(Battling, false);
}

void Player::receivePM(int id, const QString &pm)
{
    if (!isLoggedIn()) {
        //INVALID BEHAVIOR
        return;
    }

    QString str = pm.trimmed();

    if (str.length() == 0) {
        //INVALID BEHAVIOR
        return;
    }

    emit PMReceived(this->id(), id, str);
}

void Player::playerBan(int p) {
    if (!isLoggedIn()) {
        emit info(id(), "Tried to ban while not logged in");
        kick();
        return;
    }

    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }

    emit playerBan(id(),p);
}

void Player::CPBan(const QString &name)
{
    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }
    SecurityManager::ban(name);
    emit info(id(), "Banned player " + name + " with CP.");
}

void Player::CPUnban(const QString &name)
{
    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }
    SecurityManager::unban(name);
    emit info(id(), "Unbanned player " + name + " with CP.");
}

void Player::playerKick(int p) {
    if (!isLoggedIn()) {
        emit info(id(), "Tried to kick while not logged in");
        kick();
        return;
    }

    if (auth() < 1) {
        return; //INVALID BEHAVIOR
    }

    emit playerKick(id(),p);
}

int Player::opponent() const
{
    return m_opponent;
}

void Player::challengeStuff(const ChallengeInfo &c)
{
    qDebug() << "Challenge received with desc " << c.desc() << " from " << id() << " to " << c.opponent();

    if (battling()) {
        return; // INVALID BEHAVIOR
    }

    int id = c.opponent();

    if (!isLoggedIn() || id == this->id()) {
        // INVALID BEHAVIOR
        return;
    }

    if (team().invalid()) {
        sendMessage("Your team is invalid, you can't challenge!");
        return;
    }

    int desc = c.desc();

    if (desc < ChallengeInfo::Sent || desc  >= ChallengeInfo::ChallengeDescLast) {
        // INVALID BEHAVIOR
        return;
    }

    if (desc == ChallengeInfo::Sent)
    {
        emit sendChallenge(this->id(), id, c);
    } else {
        if (desc == ChallengeInfo::Accepted && !okForBattle()) {
            return;
        }

        if(challengedBy && challengedBy->challenger() == id) {
            challengedBy->manageStuff(this, c);
        } else {
            foreach (Challenge *_c, challenged) {
                if (_c->challenged() == id) {
                    _c->manageStuff(this, c);
                    return;
                }
            }
        }
    }
}

void Player::sendChallengeStuff(const ChallengeInfo &c)
{
    relay().sendChallengeStuff(c);
}

void Player::startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    relay().engageBattle(this->id(), id, team, conf);

    m_opponent = id;

    changeState(Battling, true);

    cancelChallenges();
}

void Player::giveBanList()
{
    if (myauth == 0) {
        return; //INVALID BEHAVIOR
    }
    QSet<QString> bannedMembers = SecurityManager::banList();
    foreach(QString s, bannedMembers) {
        relay().notify(NetworkServ::GetBanList, s, SecurityManager::ip(s));
    }
}

TeamBattle & Player::team()
{
    return myteam;
}

const TeamBattle & Player::team() const
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
    return state() & Battling;
}

bool Player::away() const
{
    return state() & Away;
}

int Player::state() const
{
    return m_state;
}

bool Player::connected() const
{
    return relay().isConnected();
}

PlayerInfo Player::bundle() const
{
    PlayerInfo p;
    p.auth = myauth;
    p.flags = state();
    p.id = id();
    p.team = basicInfo();

    return p;
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
    if (isLoggedIn())
        return; //INVALID BEHAVIOR


    AuthentificationState st = testAuthentification(_team);

    if (st == Invalid) {
        kick();
        return;
    }

    if (st == Success) {
        team() = _team;
        emit loggedIn(id(), _team.name);
        return;
    }

    /* st == Partial */
    team() = _team;
    return;
}

Player::AuthentificationState Player::testAuthentification(const TeamInfo &team)
{
    if (!SecurityManager::isValid(team.name)) {
        emit info(id(), "invalid name: \"" + team.name + "\"");
        sendMessage("Invalid name. Change your name.");
        return Invalid;
    }

    if (SecurityManager::exist(team.name)) {
        SecurityManager::Member m = SecurityManager::member(team.name);
        if (m.isBanned()) {
            sendMessage("You are banned!");
            return Invalid;
        }
        if (m.isProtected()) {
            relay().notify(NetworkServ::AskForPass, m.salt);
            waiting_name = team.name;
            return Partial;
        }

        myauth = m.authority();

        m.modifyIP(relay().ip());
        m.modifyDate(QDate::currentDate().toString(Qt::ISODate));
        SecurityManager::updateMember(m);
        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        return Success;
    } else {
        myauth = 0;

        SecurityManager::create(SecurityManager::Member(team.name.toLower(), QDate::currentDate().toString(Qt::ISODate), "000", "", "", relay().ip()));
        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        return Success;
    }
}

void Player::registerRequest() {
    /* If not logged in or in the middle of an authentification, we quit */
    if (!isLoggedIn() || waiting_name.length() > 0)
        return; //INVALID BEHAVIOR
    SecurityManager::Member m = SecurityManager::member(name());

    if (m.isProtected())
        return; //INVALID BEHAVIOR

    for (int i = 0; i < SecurityManager::Member::saltLength; i++) {
        m.salt[i] = (true_rand() % (122-49)) + 49;
    }

    SecurityManager::updateMemory(m);
    relay().notify(NetworkServ::AskForPass, m.salt);
}

void Player::userInfoAsked(const QString &name)
{
    if (myauth == 0) {
        return; //INVALID BEHAVIOR
    }

    if (!SecurityManager::exist(name)) {
        relay().sendUserInfo(UserInfo(name, UserInfo::NonExistant));
        return;
    }

    SecurityManager::Member m = SecurityManager::member(name);

    UserInfo ret(name, m.isBanned() ? UserInfo::Banned : 0, m.authority(), m.ip.trimmed());
    relay().sendUserInfo(ret);

    QList<QString> aliases = SecurityManager::membersForIp(m.ip.trimmed());

    foreach(QString alias, aliases) {
        relay().notify(NetworkServ::GetUserAlias, alias);
    }
}

void Player::hashReceived(const QString &_hash) {
    QString hash = md5_hash(_hash);
    if (waiting_name.length() > 0) {
        if (hash == SecurityManager::member(waiting_name).hash) {
            SecurityManager::Member m = SecurityManager::member(waiting_name);

            m.modifyIP(relay().ip());
            m.modifyDate(QDate::currentDate().toString(Qt::ISODate));
            m.hash = hash;
            myauth = m.authority();
            SecurityManager::updateMember(m);

            QString temp = waiting_name;
            waiting_name.clear();
            emit loggedIn(id(), temp);
        } else {
            emit info(id(), tr("authentification failed for %1").arg(waiting_name));
            kick();
            return;
        }
    } else {
        SecurityManager::Member m = SecurityManager::member(name());
        if (m.isProtected()) {
            return; //Invalid behavior
        }

        m.hash = hash;
        SecurityManager::updateMember(m);
        emit info(id(), tr("%1 registered.").arg(name()));
    }
}

QString Player::name() const
{
    return team().name;
}

QString Player::ip() const
{
    return myip;
}

void Player::recvTeam(const TeamInfo &team)
{
    cancelChallenges();

    if (team.name == this->team().name) {
        /* No authentification required... */
        this->team() = team;
        emit recvTeam(id(), team.name); // no check needed, going directly there...
        return;
    }

    AuthentificationState s = testAuthentification(team);

    /* just keeping the old name while not logged in */
    QString name = this->team().name;
    this->team() = team;
    this->team().name = name;

    if (s == Success) {
        emit loggedIn(id(), team.name); //checks needed
        return;
    }

    if (s == Invalid) {
        kick();
        return;
    }

    // Partial authentification
    /*
      .
      .
      .
      */
}

void Player::spectatingRequested(int id)
{
    if (!isLoggedIn()) {
        return; //INVALID BEHAVIOR
    }
    if (id == this->id()) {
        return; //INVALID BEHAVIOR
    }
    if (battlesSpectated.size() >= 2) {
        sendMessage(tr("You're already watching %1 battles!").arg(battlesSpectated.size()));
        return;
    }
    emit spectatingRequested(this->id(), id);
}

void Player::sendMessage(const QString &mess)
{
    relay().sendMessage(mess);
}

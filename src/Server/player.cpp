#include "player.h"
#include "security.h"
#include "challenge.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "battle.h"
#include "tier.h"

Player::Player(QTcpSocket *sock, int id) : myrelay(sock, id), myid(id)
{
    battle = NULL;
    challengedBy = NULL;
    battleSearch() = false;
    battleId() = -1;
    myip = relay().ip();
    rating() = -1;

    m_state = NotLoggedIn;
    myauth = 0;

    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(TeamInfo,bool,bool,QColor)), SLOT(loggedIn(TeamInfo,bool,bool,QColor)));
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
    connect(&relay(), SIGNAL(ladderChange(bool)), SLOT(ladderChange(bool)));
    connect(&relay(), SIGNAL(showTeamChange(bool)), SLOT(showTeamChange(bool)));
    connect(&relay(), SIGNAL(tierChanged(QString)), SLOT(changeTier(QString)));
    connect(&relay(), SIGNAL(findBattle(FindBattleData)), SLOT(findBattle(FindBattleData)));
    connect(&relay(), SIGNAL(showRankings(QString,int)), SLOT(getRankingsByPage(QString, int)));
    connect(&relay(), SIGNAL(showRankings(QString,QString)), SLOT(getRankingsByName(QString, QString)));
}

Player::~Player()
{
}

void Player::ladderChange(bool n)
{
    if (!isLoggedIn())
        return;//INV BEHAV
    ladder() = n;
    emit updated(id());
}

void Player::showTeamChange(bool n)
{
    if (!isLoggedIn())
        return; //INV BEHAV
    showteam() = n;
    emit updated(id());
}

void Player::cancelBattleSearch()
{
    if (!inSearchForBattle())
        return;
    emit battleSearchCancelled(id());
}

void Player::changeTier(const QString &newtier)
{
    if (!TierMachine::obj()->exists(newtier)) {
        sendMessage(tr("That tier doesn't exist!"));
        return;
    }
    if (!TierMachine::obj()->isValid(team(), newtier)) {
        QString pokeList = "";
        for(int i = 0; i < 6; i++) {
            if (TierMachine::obj()->isBanned(team().poke(i),newtier)) {
                pokeList += PokemonInfo::Name(team().poke(i).num()) + ", ";
            }
        }
        if (pokeList.length() >= 2)
            pokeList.resize(pokeList.size()-2);

        sendMessage(tr("The following pokemons are banned in %1, hence you can't choose that tier: %2.").arg(newtier,pokeList));
        return;
    }
    tier() = newtier;
    rating() = TierMachine::obj()->rating(name(), tier());
    cancelChallenges();
    emit updated(id());
}

void Player::doWhenDC()
{
    relay().stopReceiving();
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

void Player::spectateBattle(const QString &name0, const QString &name1, int battleId, bool doubles)
{
    battlesSpectated.insert(battleId);
    relay().notify(NetworkServ::SpectateBattle, name0, name1, qint32(battleId), doubles);
}

void Player::cancelChallenges()
{
    if (challengedBy != NULL) {
        challengedBy->cancel(this);
    }
    while (challenged.size() != 0) {
        (*challenged.begin())->cancel(this);
    }
    cancelBattleSearch();
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

    emit battleFinished(Forfeit, opponent(), id(), battle->rated(), battle->tier());
}

void Player::battleResult(int result, int winner, int loser)
{
    relay().sendBattleResult(result, winner, loser);

    if ((winner == id() || loser == id()) && (result == Forfeit || result == Close))
        changeState(Battling, false);
}

void Player::getRankingsByPage(const QString &tier, int page)
{
    qDebug() << "tier: " << tier << ", page: " << page;
    if (!TierMachine::obj()->exists(tier))
        return;
    /* A page is 40 players */
    int startingRank = (page-1) * 40 + 1;

    relay().startRankings(page, startingRank, (TierMachine::obj()->count(tier)-1)/40 + 1);

    const RankingTree<QString> *rt = TierMachine::obj()->getRankingTree(tier);

    RankingTree<QString>::iterator it = rt->getByRanking(startingRank);

    int i = 0;
    while (i < 40 && it.p != NULL)
    {
        i++;
        relay().sendRanking(it->data, it->key);
        --it;
    }
}

void Player::getRankingsByName(const QString &tier, const QString &name)
{
    if (!TierMachine::obj()->exists(tier))
        return;
    if (!TierMachine::obj()->existsPlayer(tier,name))
        getRankingsByPage(tier, 1);
    else {
        int page = (TierMachine::obj()->ranking(name, tier)-1)/40 + 1;
        getRankingsByPage(tier, page);
    }
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
    if (battling()) {
        return; // INVALID BEHAVIOR
    }

    int id = c.opponent();

    if (id == 0) {
        /* Find Battle Cancel */
        if (inSearchForBattle())
            cancelBattleSearch();
        return;
    }

    if (!isLoggedIn() || id == this->id()) {
        // INVALID BEHAVIOR
        return;
    }

    int desc = c.desc();

    if (desc < ChallengeInfo::Sent || desc  >= ChallengeInfo::ChallengeDescLast) {
        // INVALID BEHAVIOR
        return;
    }

    if (desc == ChallengeInfo::Sent)
    {
        if (team().invalid() && ! (c.clauses & ChallengeInfo::ChallengeCup)) {
            sendMessage("Your team is invalid, you can't challenge except for Challenge Cup!");
            return;
        }
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

void Player::findBattle(const FindBattleData& f)
{
    if (battling()) {
        // INVALID BEHAVIOR
        return;
    }
    if (!isLoggedIn()) {
        // INVALID BEHAVIOR
        return;
    }
    if (team().invalid())
    {
        sendMessage("Your team is invalid, you can't find battles!");
        return;
    }

    cancelBattleSearch();

   emit findBattle(id(),f);
}

void Player::sendChallengeStuff(const ChallengeInfo &c)
{
    relay().sendChallengeStuff(c);
}

void Player::startBattle(int id, const TeamBattle &team, const BattleConfiguration &conf, bool doubles)
{
    relay().engageBattle(this->id(), id, team, conf, doubles);

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
    p.rating = ladder() ? rating() : -1;
    p.tier = tier();
    p.avatar = avatar();
    p.color = color();

    if (showteam()) {
        for(int i = 0; i < 6; i++) {
            p.pokes[i] = team().poke(i).num();
        }
    } else {
        for(int i = 0; i < 6; i++) {
            p.pokes[i] = 0;
        }
    }

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

void Player::loggedIn(const TeamInfo &_team,bool ladder, bool showteam, QColor c)
{
    if (isLoggedIn())
        return; //INVALID BEHAVIOR

    avatar() = _team.avatar;
    this->ladder() = ladder;
    this->showteam() = showteam;

    if (c.lightness() <= 140 && c.green() <= 180)
        color() = c;

    AuthentificationState st = testAuthentification(_team);

    if (st == Invalid) {
        kick();
        return;
    }

    qDebug() << "Assigning team";
    team() = _team;
    winningMessage() = _team.win;
    losingMessage() = _team.lose;
    tier() = TierMachine::obj()->findTier(team());
    rating() = TierMachine::obj()->rating(name(), tier());

    if (st == Success) {
        emit loggedIn(id(), _team.name);
    }
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
        rating() = 1000;

        SecurityManager::create(SecurityManager::Member(team.name, QDate::currentDate().toString(Qt::ISODate), "000", "", "", relay().ip()));
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

    if (SecurityManager::maxAuth(m.ip.trimmed()) > auth()) {
        relay().notify(NetworkServ::GetUserAlias, m.name);
        return;
    }

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

    avatar() = team.avatar;

    if (team.name.toLower() == this->team().name.toLower()) {
        /* No authentification required... */
        this->team() = team;
        winningMessage() = team.win;
        losingMessage() = team.lose;
        tier() = TierMachine::obj()->findTier(this->team());
        rating() = TierMachine::obj()->rating(name(), tier());

        emit recvTeam(id(), team.name); // no check needed, going directly there...
        return;
    }

    AuthentificationState s = testAuthentification(team);

    /* just keeping the old name while not logged in */
    QString name = this->team().name;
    this->team() = team;
    tier() = TierMachine::obj()->findTier(this->team());
    rating() = TierMachine::obj()->rating(this->name(), tier());
    winningMessage() = team.win;
    losingMessage() = team.lose;
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

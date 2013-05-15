#include "relaymanager.h"
#include "../Shared/config.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "player.h"
#include "security.h"
#include "challenge.h"
#include "tiermachine.h"
#include "tier.h"
#include "waitingobject.h"
#include "server.h"
#include "analyze.h"
#include <algorithm>

Player::Player(const GenericSocket &sock, int id)
{
    loginInfo() = NULL;
    m_bundle.id = id;

    myrelay = new Analyzer(sock, id);
    lockCount = 0;
    battleSearch() = false;
    myip = relay().ip();
    server_pass_sent = false;
    needToUpdate = false;


    m_bundle.auth = 0;

    doConnections();

    /* Version control, whatever happens, because the problem could be because of an old version */
    relay().notify(NetworkServ::VersionControl_, ProtocolVersion(), Flags(), ProtocolVersion(1,1), ProtocolVersion(0,0), ProtocolVersion(0,0), Server::serverIns->servName());

    /* Autokick after 3 minutes if still not logged in */
    QTimer::singleShot(1000*180, this, SLOT(firstAutoKick()));
}

Player::~Player()
{
    removeRelay();

    if (loginInfo()) {
        delete loginInfo();
    }
}

void Player::doConnections()
{
    /* The reason for Queued Connection is that for example a disconnect signal could be received when
      a script sends a message to the client.

      If that happens we want the disconnect signal to happen after the script function*/
    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()), Qt::QueuedConnection);
    connect(&relay(), SIGNAL(loggedIn(LoginInfo*)), SLOT(loggedIn(LoginInfo*)));
    connect(&relay(), SIGNAL(logout()), SLOT(logout()));
    connect(&relay(), SIGNAL(serverPasswordSent(const QByteArray&)), SLOT(serverPasswordSent(const QByteArray&)));
    connect(&relay(), SIGNAL(messageReceived(int, QString)), SLOT(recvMessage(int, QString)));
    connect(&relay(), SIGNAL(playerDataRequested(int)), SLOT(recvPlayerDataRequest(int)));
    connect(&relay(), SIGNAL(teamChanged(const ChangeTeamInfo&)), SLOT(recvTeam(const ChangeTeamInfo&)));
    connect(&relay(), SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(&relay(), SIGNAL(forfeitBattle(int)), SLOT(battleForfeited(int)));
    connect(&relay(), SIGNAL(battleMessage(int,BattleChoice)), SLOT(battleMessage(int,BattleChoice)));
    connect(&relay(), SIGNAL(battleChat(int,QString)), SLOT(battleChat(int,QString)));
    connect(&relay(), SIGNAL(sentHash(QByteArray)), SLOT(hashReceived(QByteArray)));
    connect(&relay(), SIGNAL(wannaRegister()), SLOT(registerRequest()));
    connect(&relay(), SIGNAL(kick(int)), SLOT(playerKick(int)));
    connect(&relay(), SIGNAL(ban(int)), SLOT(playerBan(int)));
    connect(&relay(), SIGNAL(tempBan(int,int)), SLOT(playerTempBan(int,int)));
    connect(&relay(), SIGNAL(banRequested(QString)), SLOT(CPBan(QString)));
    connect(&relay(), SIGNAL(tempBanRequested(QString,int)), SLOT(CPTBan(QString,int)));
    connect(&relay(), SIGNAL(unbanRequested(QString)), SLOT(CPUnban(QString)));
    connect(&relay(), SIGNAL(PMsent(int,QString)), SLOT(receivePM(int,QString)));
    connect(&relay(), SIGNAL(getUserInfo(QString)), SLOT(userInfoAsked(QString)));
    connect(&relay(), SIGNAL(banListRequested()), SLOT(giveBanList()));
    connect(&relay(), SIGNAL(awayChange(bool)), SLOT(awayChange(bool)));
    connect(&relay(), SIGNAL(battleSpectateRequested(int)), SLOT(spectatingRequested(int)));
    connect(&relay(), SIGNAL(battleSpectateEnded(int)), SLOT(quitSpectating(int)));
    connect(&relay(), SIGNAL(battleSpectateChat(int,QString)), SLOT(spectatingChat(int,QString)));
    connect(&relay(), SIGNAL(ladderChange(bool)), SLOT(ladderChange(bool)));
    connect(&relay(), SIGNAL(tierChanged(quint8,QString)), SLOT(changeTier(quint8,QString)));
    connect(&relay(), SIGNAL(findBattle(FindBattleData)), SLOT(findBattle(FindBattleData)));
    connect(&relay(), SIGNAL(showRankings(QString,int)), SLOT(getRankingsByPage(QString, int)));
    connect(&relay(), SIGNAL(showRankings(QString,QString)), SLOT(getRankingsByName(QString, QString)));
    connect(&relay(), SIGNAL(joinRequested(QString)), SLOT(joinRequested(QString)));
    connect(&relay(), SIGNAL(leaveChannel(int)), SLOT(leaveRequested(int)));
    connect(&relay(), SIGNAL(ipChangeRequested(QString)), SLOT(ipChangeRequested(QString)));
    connect(&relay(), SIGNAL(endCommand()), SLOT(sendUpdatedIfNeeded()));
    connect(&relay(), SIGNAL(reconnect(int,QByteArray)), SLOT(onReconnect(int,QByteArray)));

    /* To avoid threading / simulateneous calls problems, it's queued */
    connect(this, SIGNAL(unlocked()), &relay(), SLOT(undelay()),Qt::QueuedConnection);
}

void Player::autoKick()
{
    if (!isLoggedIn()) {
        blockSignals(false); // In case we autokick an alt that was already disconnected
        disconnected();
    }
}

/* This is the auto kick that kicks you if you don't log in in time.
  So we check if we are just not disconnected.

  Because we wouldn't want to kick a disconnected player waiting for a battle */
void Player::firstAutoKick()
{
    if (state()[WaitingReconnect]) {
        return;
    }

    autoKick();
}

void Player::ladderChange(bool n)
{
    if (!isLoggedIn())
        return;//INV BEHAV
    if (state()[LadderEnabled] == n) {
        return;
    }
    state().setFlag(LadderEnabled, n);
    relay().notifyOptionsChange(id(), away(), n);
}

void Player::cancelBattleSearch()
{
    if (!inSearchForBattle())
        return;
    emit battleSearchCancelled(id());
}

void Player::lock()
{
    lockCount += 1;
    relay().delay();
}

void Player::unlock()
{
    lockCount -= 1;
    if (lockCount >= 0)
        emit unlocked();
    else
        lockCount = 0;
}

void Player::setNeedToBeUpdated(bool onlyIfInCommand)
{
    needToUpdate = true;

    if (onlyIfInCommand && !isInCommand()) {
        sendUpdatedIfNeeded();
    }
}

bool Player::isInCommand() const
{
    return relay().isInCommand();
}

void Player::sendUpdatedIfNeeded()
{
    if (needToUpdate) {
        needToUpdate = false;
        emit updated(id());
    }
}

void Player::changeTier(quint8 teamNum, const QString &newtier)
{
    if (!isLoggedIn()) {
        return;
    }

    if (teamNum >= teamCount()) {
        return;
    }
    if (team(teamNum).tier == newtier)
        return;
    if (battling()) {
        sendMessage(tr("You can't change tiers while battling."));
        return;
    }
    if (!TierMachine::obj()->exists(newtier)) {
        sendMessage(tr("The tier %1 doesn't exist!").arg(newtier));
        return;
    }
    if (!TierMachine::obj()->isValid(team(teamNum), newtier)) {
        Tier *tier = &TierMachine::obj()->tier(newtier);

        if (!tier->allowGen(team(teamNum).gen)) {
            sendMessage(tr("The generation of your team (%1) is invalid for the tier %2 which is in generation %3.").arg(GenInfo::Version(team(teamNum).gen), tier->name(), GenInfo::Version(tier->gen())));
            return;
        }

        QList<int> indexList;
        for(int i = 0; i < 6; i++) {
            if (tier->isBanned(team(teamNum).poke(i))) {
                indexList.append(i);
            }
        }

        if (indexList.size() > 0) {
            foreach(int i, indexList) {
                sendMessage(tr("The Pokemon '%1' is banned on tier '%2' for the following reasons: %3").arg(PokemonInfo::Name(team(teamNum).poke(i).num()), newtier,
                                                                                                            tier->bannedReason(team(teamNum).poke(i))));
            }
            return;
        } else {
            sendMessage(tr("You have too many restricted pokemons, or simply too many pokemons for the tier %1.").arg(newtier));
            return;
        }
    }
    if (Server::serverIns->beforeChangeTier(id(), teamNum, team(teamNum).tier, newtier)) {
        QString oldtier = team(teamNum).tier;
        executeTierChange(teamNum, newtier);
        Server::serverIns->afterChangeTier(id(), teamNum, oldtier, team(teamNum).tier);
    }
}

void Player::executeTierChange(int num, const QString &newtier)
{
    bool oldT(false), newT(false);
    QString oldTier = team(num).tier;

    for (int i = 0; i < teamCount(); i++) {
        if (i != num) {
            if (team(i).tier == oldTier) {
                oldT = true;
            }
            if (team(i).tier == newtier) {
                newT = true;
            }
        }
    }

    cancelChallenges();

    team(num).tier = newtier;
    if (newT && oldT) {
        //The list of tiers didn't change overall, no need to recalculate anything
    } else {
        syncTiers(oldTier);
        findRating(team(num).tier);
    }
}

void Player::removeRelay()
{
    if (myrelay) {
        RelayManager::obj()->addTrash(myrelay, this);
        myrelay = NULL;
    }
}

void Player::doWhenDC()
{
    removeRelay();

    cancelChallenges();
    cancelBattleSearch();

    foreach(int id, battles) {
        quitSpectating(id);
    }
    foreach(int id, battlesSpectated) {
        quitSpectating(id);
    }
    foreach(Player *p, knowledge) {
        if (p->isLoggedIn()) {
            p->relay().sendLogout(this->id());
        }
    }
}

void Player::doWhenRC(bool wasLoggedIn)
{
    changeState(Player::WaitingReconnect, false);

    if (!wasLoggedIn)
    {
        /* make acquaintances again! */
        foreach(Player *p, knowledge) {
            if (p->isLoggedIn()) {
                p->relay().notify(NetworkServ::PlayersList, bundle());
                relay().notify(NetworkServ::PlayersList, p->bundle());
            }
        }

        QSet<int> channelsCopy = channels;
        channels.clear();

        foreach(int channelId, channelsCopy) {
            emit joinRequested(id(), channelId);
            /* In case a script kicked us */
            if (!isLoggedIn()) {
                return;
            }
        }

        if (channels.empty()) {
            emit joinRequested(id(), 0);

            if (!isLoggedIn()) {
                return;
            }
        }
    } else {
        QSet<int> copy = channels;
        channels.clear();
        foreach(int channelId, copy) {
            emit needChannelData(id(), channelId);
        }
    }

    foreach(int battleid, battles)
    {
        emit resendBattleInfos(this->id(), battleid);
    }
}

void Player::doWhenDQ()
{
    removeRelay();

    cancelChallenges();
    cancelBattleSearch();

    foreach(int id, battles) {
        battleForfeited(id);
    }
    foreach(int id, battlesSpectated) {
        quitSpectating(id);
    }
    foreach(Player *p, knowledge) {
        if (isLoggedIn() && p->isLoggedIn()) {
            p->relay().sendLogout(this->id());
        }
        p->knowledge.remove(this);
    }
    knowledge.clear();
}

void Player::quitSpectating(int battleId)
{
    if (battlesSpectated.contains(battleId)) {
        battlesSpectated.remove(battleId);
        emit spectatingStopped(this->id(), battleId);
    } else if (battles.contains(battleId)) {
        emit spectatingStopped(this->id(), battleId);
    }
}

void Player::onReconnect(int id, const QByteArray &hash)
{
    if (state()[LoginAttempt]) {
        return; //INVALID BEHAVIOR
    }

    state().setFlag(LoginAttempt, true);

    emit reconnect(this->id(), id, hash);
}

void Player::joinRequested(const QString &name)
{
    if (!isLoggedIn()) {
        return;
    }
    /* Too many channels */
    if (auth() == 0 && channels.size() >= 12) {
        sendMessage("You can't join more than 12 channels");
        return;
    }

    emit joinRequested(id(), name);
}

bool Player::inChannel(int chan) const
{
    return channels.contains(chan);
}

void Player::sendPacket(const QByteArray &packet)
{
    relay().sendPacket(packet);
}

void Player::leaveRequested(int slotid)
{
    if (!isLoggedIn()) {
        return;
    }

    if (!channels.contains(slotid)) {
        return;
    }

    /* Not allowing the player to have less than 1 channel open ! */
    if (channels.size() <= 1) {
        return;
    }

    emit leaveRequested(id(), slotid);
}

void Player::ipChangeRequested(const QString& ip)
{
    if (isLoggedIn())
        return; // only allowed before logging in

    if (proxyip.size() > 0)
        return; // Can't change twice

    if (!Server::serverIns->isLegalProxyServer(myip))
        return;

    proxyip = myip;
    myip = ip;

    /* So the anti-dos can now work correctly on the player */
    relay().changeIP(ip);

    emit ipChangeRequested(id(), ip);
}

void Player::spectateBattle(int battleId, const BattleConfiguration &battle)
{
    battlesSpectated.insert(battleId);
    relay().spectateBattle(battleId, battle);
}

void Player::cancelChallenges()
{
    foreach(Challenge *c, challengedBy) {
        c->cancel(this);
    }
    while (challenged.size() != 0) {
        (*challenged.begin())->cancel(this);
    }
}

void Player::removeChallenge(Challenge *c)
{
    challengedBy.remove(c);
    challenged.remove(c);
}

void Player::addChallenge(Challenge *c, bool youarechallenged)
{
    if (youarechallenged) {
        challengedBy.insert(c);
    } else {
        challenged.insert(c);
    }
}

bool Player::okForChallenge(int) const
{
    if (!isLoggedIn() || battling() || away() || challengedBy.size() >= 3)
        return false;

    return true;
}

bool Player::okForBattle() const
{
    return isLoggedIn();
}

void Player::awayChange(bool away)
{
    if (away == this->away()) {
        return;
    }

    if (!isLoggedIn() || battling()) {
        return;
    }

    if (Server::serverIns->beforePlayerAway(id(), away)) {
        executeAwayChange(away);
        Server::serverIns->afterPlayerAway(id(), away);
    }
}

void Player::executeAwayChange(bool away)
{
    changeState(Away, away);
    emit awayChange(id(), away);
}

void Player::changeState(int newstate, bool on)
{
    state().setFlag(newstate, on);
}

int Player::auth() const {
    return m_bundle.auth;
}

void Player::setAuth(int auth)  {
    m_bundle.auth = auth;
}

void Player::setName(const QString &newname)  {
    m_bundle.name = newname;
}

void Player::setInfo(const QString &newInfo)  {
    m_bundle.info = newInfo;
}

void Player::kick() {
    emit logout(id());
}

void Player::disconnected()
{
    removeRelay();
    emit disconnected(id());
}

int Player::firstBattleId()
{
    return *battles.begin();
}

void Player::battleChat(int bid, const QString &s)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleChat(id(), bid, s);
}

void Player::spectatingChat(int id, const QString &chat)
{
    if (!battlesSpectated.contains(id)) {
        return; //INVALID BEHAVIOR
    }
    emit spectatingChat(this->id(), id, chat);
}

void Player::battleMessage(int bid, const BattleChoice &b)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleMessage(id(), bid, b);
}

void Player::recvMessage(int chan, const QString &mess)
{
    if (!isLoggedIn())
        return; //INVALID BEHAVIOR
    if (!channels.contains(chan))
        return;
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), chan, mess);
}

/**
  Sends the player info of the requested id to the player
  */
void Player::recvPlayerDataRequest(int pid)
{
    if (!isLoggedIn()) {
        return;
    }
    if (Server::serverIns->playerLoggedIn(pid)) {
        relay().sendPlayer(Server::serverIns->player(pid)->bundle());
    }
}

void Player::battleForfeited(int bid)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleFinished(bid, Forfeit, 0, id());
}

void Player::battleResult(int battleid, int result, int battlemode, int winner, int loser)
{
    relay().sendBattleResult(battleid, result, battlemode, winner, loser);
}

void Player::addBattle(int battleid)
{
    battles.insert(battleid);
}

void Player::removeBattle(int battleid)
{
    battles.remove(battleid);
}

void Player::getRankingsByName(const QString &tier, const QString &name)
{
    if (!TierMachine::obj()->exists(tier))
        return;
    TierMachine::obj()->fetchRankings(tier, name, this, SLOT(displayRankings()));
}

void Player::getRankingsByPage(const QString &tier, int page)
{
    if (!TierMachine::obj()->exists(tier))
        return;
    /* A page is 40 players */
    TierMachine::obj()->fetchRankings(tier, page, this, SLOT(displayRankings()));
}

void Player::displayRankings()
{
    WaitingObject *src = (WaitingObject*) (sender());

    int page = src->data["rankingpage"].toInt();
    int startingRank = (page-1) * TierMachine::playersByPage + 1;

    int count = TierMachine::obj()->count(src->data.value("tier").toString());
    relay().startRankings(page, startingRank, (count-1) / TierMachine::playersByPage + 1);

    typedef QPair<QString, int> p;
    QVector<p> results = src->data["rankingdata"].value<QVector<p> >();

    /* Removing the properties to clear memory */
    src->data.clear();

    foreach(p pair, results) {
        relay().sendRanking(pair.first, pair.second);
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

void Player::playerTempBan(int player, int time)
{
    if(!isLoggedIn()) {
        emit info(id(), "Tried to temp ban while not logged in");
        kick();
        return;
    }
    if(auth() < 1) {
        return;
    }
    emit playerTempBan(id(), player, time);
}

void Player::CPBan(const QString &name)
{
    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }
    int maxAuth = SecurityManager::maxAuth(SecurityManager::ip(name));
    if (maxAuth >= auth()) {
        sendMessage(name + " has authority equal or superior to yours under another nick.");
        return;
    }
    SecurityManager::setBanExpireTime(name, 0);
    SecurityManager::ban(name);
    emit info(id(), "Banned player " + name + " with CP.");

    QFile out("bans.txt");
    out.open(QIODevice::Append);
    out.write((this->name() + " CP banned " + name + ".\n").toUtf8());
}

void Player::CPUnban(const QString &name)
{
    if (auth() < 1) {
        return; //INVALID BEHAVIOR
    }

    if (auth() < 2) {
        int MAX_MOD_UNBAN = 1440*60; // One full day for mods
        SecurityManager::Member member = SecurityManager::member(name);
		if (member.ban_expire_time > MAX_MOD_UNBAN + QDateTime::currentDateTimeUtc().toTime_t()) {
            return; //INVALID BEHAVIOR
		}
    }

    SecurityManager::unban(name);
    emit info(id(), "Unbanned player " + name + " with CP.");

    QFile out("bans.txt");
    out.open(QIODevice::Append);
    out.write((this->name() + " unbanned " + name + ".\n").toUtf8());
}

void Player::CPTBan(const QString &name, int time)
{
    if (auth() < 1) {
        return; //INVALID BEHAVIOR
    }
    int maxAuth = SecurityManager::maxAuth(SecurityManager::ip(name));
    if (maxAuth >= auth()) {
        sendMessage(name + " has authority " + maxAuth + " under another nick.");
        return;
    }
    /* Checking the time boundaries */
    if (auth() < 2) {
        time = std::max(1, std::min(time, 1440));
    }
    SecurityManager::setBanExpireTime(name, QDateTime::currentDateTimeUtc().toTime_t() + time*60);
    SecurityManager::ban(name);
    emit info(id(), "Temporarily Banned player " + name + " with CP for " + int(time) + " minutes.");
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

void Player::challengeStuff(const ChallengeInfo &c)
{
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

    if (c.team >= teamCount()) {
        sendMessage("You must select a correct team");
        return;
    }

    if (desc == ChallengeInfo::Sent)
    {
        if (team().invalid() && ! (c.clauses & ChallengeInfo::ChallengeCup)) {
            sendMessage("Your team is invalid, you can't challenge except for Challenge Cup! Try giving moves to your pokemon.");
            return;
        }
        if (challenged.size() >= 10) {
            sendMessage("You already have challenged 10 people, you can't challenge more!");
            return;
        }
        if (c.mode > ChallengeInfo::Rotation) {
            sendMessage("You must challenge to a correct mode");
            return;
        }

        emit sendChallenge(this->id(), id, c);
    } else {
        if (desc == ChallengeInfo::Accepted && !okForBattle()) {
            return;
        }

        foreach (Challenge *_c, challengedBy) {
            if (_c->challenger() == id && _c->tier() == c.desttier && _c->gen() == c.gen) {
                _c->manageStuff(this, c);
                return;
            }
        }
        foreach (Challenge *_c, challenged) {
            if (_c->challenged() == id) {
                _c->manageStuff(this, c);
                return;
            }
        }
    }
}

bool Player::hasSentCommand(int commandid) const
{
    if (commandid != lastcommand) {
        lastcommand = commandid;
        return false;
    }
    return true;
}

void Player::findBattle(const FindBattleData& f)
{
    if (!isLoggedIn()) {
        // INVALID BEHAVIOR
        return;
    }

    if (battles.size() >= 3) {
        // INVALID BEHAVIOR
        return;
    }

    cancelBattleSearch();

    if (Server::serverIns->beforeFindBattle(id())) {
        emit findBattle(id(),f);
        Server::serverIns->afterFindBattle(id());
    }
}

void Player::sendChallengeStuff(const ChallengeInfo &c)
{
    relay().sendChallengeStuff(c);
}

void Player::startBattle(int battleid, int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    relay().engageBattle(battleid, this->id(), id, team, conf);

    cancelChallenges();
    cancelBattleSearch();
}

void Player::giveBanList()
{
    if (auth() == 0) {
        return; //INVALID BEHAVIOR
    }
    QHash<QString, std::pair<QString, int> > bannedMembers = SecurityManager::banList();
    QHashIterator<QString, std::pair<QString, int> > it(bannedMembers);

    while (it.hasNext()) {
        it.next();
        relay().notify(NetworkServ::GetBanList, it.key(), it.value().first, it.value().second);
    }
}

TeamBattle & Player::team(int i)
{
    return m_teams.team(i);
}

const TeamBattle & Player::team(int i) const
{
    return m_teams.team(i);
}

Pokemon::gen Player::gen(int team) const
{
    return this->team(team).gen;
}

int Player::teamCount() const
{
    return m_teams.count();
}

int Player::rating(const QString &tier)
{
    if (ratings().contains(tier)) {
        return ratings()[tier];
    } else {
        return TierMachine::obj()->rating(name(), tier);
    }
}

QHash<QString, quint16> &Player::ratings()
{
    return m_bundle.ratings;
}

Analyzer & Player::relay()
{
    if (!myrelay) {
        return *RelayManager::obj()->dummyRelay();
    }
    return *myrelay;
}

const Analyzer & Player::relay() const
{
    if (!myrelay) {
        return *RelayManager::obj()->dummyRelay();
    }
    return *myrelay;
}

bool Player::battling() const
{
    return battles.size() > 0;
}

bool Player::supportsZip() const
{
    return spec()[SupportsZipCompression];
}

bool Player::hasTier(const QString &tier) const
{
    return tiers.contains(tier);
}

bool Player::hasBattle(int battleId) const
{
    return battles.contains(battleId);
}

bool Player::away() const
{
    return state()[Away];
}

bool Player::waitingForReconnect() const
{
    return state()[WaitingReconnect] && !isLoggedIn();
}

bool Player::discarded() const
{
    return state()[DiscardedId];
}

bool Player::connected() const
{
    return relay().isConnected();
}

const QString & Player::name() const
{
    return m_bundle.name;
}

QString & Player::name()
{
    return m_bundle.name;
}

const QString & Player::info() const
{
    return m_bundle.info;
}

QString & Player::info()
{
    return m_bundle.info;
}

const PlayerInfo& Player::bundle() const
{
    //Todo: update those in real time
    m_bundle.flags.setFlag(PlayerInfo::Away, state()[Away]);
    m_bundle.flags.setFlag(PlayerInfo::LadderEnabled, state()[LadderEnabled]);

    return m_bundle;
}

bool Player::isLoggedIn() const
{
    return state()[LoggedIn];
}

int Player::id() const
{
    return m_bundle.id;
}

bool Player::ladder() const
{
    return state()[LadderEnabled];
}

void Player::logout()
{
    emit logout(id());
}

bool Player::hasReconnectPass() const
{
    return (isLoggedIn() && waiting_pass.length() > 0) || state()[WaitingReconnect];
}

bool Player::testReconnectData(Player *other, const QByteArray &hash)
{
    //test ip first
    QHostAddress own(ip()), otherHost(other->ip());

    if (reconnectBits() != 0) {
        if (!own.isInSubnet(otherHost, reconnectBits())) {
            other->relay().notify(NetworkServ::Reconnect, false, quint8(PlayerFlags::IPMismatch));
            other->kick();
            return false;
        }
    }

    if (hash != waiting_pass) {
        other->relay().notify(NetworkServ::Reconnect, false, quint8(PlayerFlags::WrongHash));
        other->kick();
        return false;
    }

    return true;
}

void Player::associateWith(Player *other)
{
    removeRelay();
    std::swap(myrelay, other->myrelay);
    relay().setId(id());
    relay().disconnect(other);
    other->disconnect(&relay());

    std::swap(myip, other->myip);
    std::swap(proxyip, other->proxyip);

    lockCount = 0;

    doConnections();

    blockSignals(false);

    /* Updates IP in case it changed */
    SecurityManager::Member m = SecurityManager::member(name());
    m.ip = ip().toLatin1();
    SecurityManager::updateMember(m);

    if (!isLoggedIn()) {
        other->changeState(DiscardedId, true);
    }
    other->logout();
}

void Player::loggedIn(LoginInfo *info)
{
    if (loginInfo()) {
        delete loginInfo();
    }
    loginInfo() = info;

    if (state()[LoginAttempt])
        return;

    state().setFlag(LoginAttempt, true);

    if (!testNameValidity(info->trainerName)) {
        return;
    }

    spec().setFlag(SupportsZipCompression, info->data[PlayerFlags::SupportsZipCompression]);
    spec().setFlag(IdsWithMessage, info->data[PlayerFlags::IdsWithMessage]);
    spec().setFlag(ReconnectEnabled, info->network[NetworkServ::LoginCommand::HasReconnect]);
    state().setFlag(LadderEnabled, info->data[PlayerFlags::LadderEnabled]);
    state().setFlag(Away, info->data[PlayerFlags::Idle]);
    reconnectBits() = info->reconnectBits;

    assignNewColor(info->trainerColor);
    os() = info->clientType;

    if (info->trainerInfo) {
        assignTrainerInfo(*info->trainerInfo);
    }

    if (info->teams) {
        m_teams.init(*info->teams);
    } else {
        m_teams.init();
    }

    name() = info->trainerName;

    // If the server is password protected, the login cannot continue until the server password is supplied
    if (Server::serverIns->isPasswordProtected()) {
        // hack, uses waiting name to store the salt
        waiting_pass.resize(SecurityManager::Member::saltLength);
        for (int i = 0; i < SecurityManager::Member::saltLength; i++) {
            waiting_pass[i] = uchar((true_rand() % (90-49)) + 49);
        }

        relay().notify(NetworkServ::ServerPass, waiting_pass);
        waiting_name = info->trainerName;
        return;
    } else {
        server_pass_sent = true;
    }

    testAuthentification(info->trainerName);
}

void Player::serverPasswordSent(const QByteArray &_hash)
{
    if (Server::serverIns->correctPass(_hash, waiting_pass)) {
        server_pass_sent = true;
        waiting_pass.clear();
        testAuthentification(waiting_name);
    } else {
        // Retry the password prompt
        // XXX: maybe make a counter of 3 or something in retry attempts?
        relay().notify(NetworkServ::ServerPass, waiting_pass);
    }
}

void Player::loginSuccess()
{
    ontologin = true;
    findTierAndRating(true);
}

void Player::testAuthentification(const QString &name)
{
    lock();

    waiting_name = name;
    SecurityManager::loadMemberInMemory(name, this, SLOT(testAuthentificationLoaded()));
}

void Player::testAuthentificationLoaded()
{
    unlock();
    QString name = waiting_name;

    if (SecurityManager::exist(name)) {
        SecurityManager::Member m = SecurityManager::member(name);
        if (m.isBanned()) {
            sendMessage("You are banned!");
            kick();
            return;
        }

        if (m.isProtected()) {
            relay().notify(NetworkServ::AskForPass, m.salt);
            return;
        }

        setAuth(m.authority());

        m.modifyIP(ip());
        m.modifyDate(QDateTime::currentDateTime().toString(Qt::ISODate));
        SecurityManager::updateMember(m);

        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        loginSuccess();
    } else {
        setAuth(0);

        SecurityManager::create(name, QDateTime::currentDateTime().toString(Qt::ISODate), ip());
        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        loginSuccess();
    }
}

void Player::findTierAndRating(bool force)
{
    tiers.clear();
    for (int i = 0; i < m_teams.count(); i++) {
        findTier(i);
        tiers.insert(team(i).tier);
    }
    findRatings(force);
}

void Player::findTier(int slot)
{
    team(slot).tier = TierMachine::obj()->findTier(team(slot));
}

bool Player::isInSameChannel(const Player *other) const {
    foreach(int chanid, channels) {
        if (other->channels.contains(chanid)) {
            return true;
        }
    }
    return false;
}

bool Player::hasKnowledgeOf(Player *other) const
{
    return knowledge.contains(other);
}

void Player::acquireKnowledgeOf(Player *other) {
    if (!isInSameChannel(other)) {
        relay().sendPlayer(other->bundle());
        other->relay().sendPlayer(bundle());
    }
    knowledge.insert(other);
    other->knowledge.insert(this);
}

/* Only rough knowledge, meaning updated infos don't matter */
void Player::acquireRoughKnowledgeOf(Player *other) {
    if (knowledge.contains(other))
        return;
    acquireKnowledgeOf(other);
}

void Player::findRatings(bool force)
{
    if (force) {
        ratings().clear();
    }

    QString name = waiting_name.length()>0 ? waiting_name : this->name();

    bool one = false;
    foreach(QString tier, tiers) {
        if (!ratings().contains(name)) {
            one = true;
            findRating(tier);
        }
    }

    if (!one) {
        ratingsFound();
    }
}

const quint16 &Player::avatar() const
{
    return m_bundle.avatar;
}

quint16 &Player::avatar()
{
    return m_bundle.avatar;
}

const QString &Player::description() const
{
    return m_bundle.info;
}

QString &Player::description()
{
    return m_bundle.info;
}

const QColor &Player::color() const
{
    return m_bundle.color;
}

QColor &Player::color()
{
    return m_bundle.color;
}

void Player::findRating(const QString &tier)
{
    lock();
    TierMachine::obj()->loadMemberInMemory(waiting_name.length()>0 ? waiting_name : this->name(), tier, this, SLOT(ratingLoaded()));
}

void Player::addChannel(int chanid)
{
    channels.insert(chanid);
}

void Player::removeChannel(int chanid)
{
    channels.remove(chanid);
}

void Player::ratingLoaded()
{
    unlock();
    QString tier = sender()->property("tier").toString();
    ratings().insert(tier, TierMachine::obj()->rating(waiting_name.length() > 0 ? waiting_name : name(), tier));

    if (tiers.count() <= ratings().count() && ratings().keys().toSet().contains(tiers)) {
        ratingsFound();
    }
}

void Player::ratingsFound()
{
    if (ontologin) {
        ontologin = false;
        if (waiting_name.length() > 0 && (waiting_name != name() || !isLoggedIn()))
            emit loggedIn(id(), waiting_name);
        else
            emit recvTeam(id(), name());
        waiting_name.clear();
    } else {
        setNeedToBeUpdated(true);
        /* Sends the team tiers to the players.
          Note: in case of cached tiers for a player, we can end up sending several time
          that in succession if he changes multiple tiers at the same time */
        relay().sendTeam(NULL, getTierList());
    }
}

void Player::sendLoginInfo()
{
    if (spec()[ReconnectEnabled]) {
        if (waiting_pass.length() == 0) {
            generateReconnectPass();
        }
    }
    relay().sendLogin(bundle(), getTierList(), waiting_pass);
}

static uchar random_character()
{
    return rand();
}

void Player::generateReconnectPass()
{
    waiting_pass.resize(8);
    std::generate(waiting_pass.begin(), waiting_pass.end(), &random_character);
}

QStringList Player::getTierList() const
{
    QStringList ret;
    for (int i = 0; i < teamCount(); i++) {
        ret.push_back(team(i).tier);
    }
    return ret;
}

void Player::assignNewColor(const QColor &c)
{
    if (c.lightness() <= 140 && c.green() <= 180)
        color() = c;
}

void Player::assignTrainerInfo(const TrainerInfo &info)
{
    avatar() = info.avatar;
    winningMessage() = info.winning;
    losingMessage() = info.losing;
    description() = info.info;
}

bool Player::isLocked() const
{
    return lockCount > 0;
}

bool Player::testNameValidity(const QString &name)
{
    if(!SecurityManager::isValid(name)) {
        emit info(id(), "invalid name: \"" + name + "\"");
        sendMessage("Invalid name. Change your name.");
        kick();
        return false;
    }
    return true;
}

void Player::registerRequest() {
    /* If not logged in or in the middle of an authentification, we quit */
    if (!isLoggedIn() || waiting_name.length() > 0)
        return; //INVALID BEHAVIOR
    SecurityManager::Member m = SecurityManager::member(name());

    if (m.isProtected())
        return; //INVALID BEHAVIOR

    for (int i = 0; i < SecurityManager::Member::saltLength; i++) {
        m.salt[i] = uchar((true_rand() % (90-49)) + 49);
    }

    SecurityManager::updateMember(m);
    relay().notify(NetworkServ::AskForPass, QString(m.salt));
}

void Player::userInfoAsked(const QString &name)
{
    if (auth() == 0) {
        return; //INVALID BEHAVIOR
    }

    if (!SecurityManager::exist(name)) {
        relay().sendUserInfo(UserInfo(name, UserInfo::NonExistant));
        return;
    }

    SecurityManager::Member m = SecurityManager::member(name);

    UserInfo ret(name, m.isBanned() ? UserInfo::Banned : 0, m.authority(), m.ip, m.date);
    relay().sendUserInfo(ret);

    if (SecurityManager::maxAuth(m.ip) > auth()) {
        relay().notify(NetworkServ::GetUserAlias, m.name);
        return;
    }

    QList<QString> aliases = SecurityManager::membersForIp(m.ip);

    foreach(QString alias, aliases) {
        relay().notify(NetworkServ::GetUserAlias, alias);
    }
}

void Player::hashReceived(const QByteArray &_hash) {
    if (!server_pass_sent) return; // Don't accept this if we haven't logged in

    QByteArray hash = md5_hash(_hash.toHex());
    if (waiting_name.length() > 0) {
        if (battling()) {
            sendMessage("You can't change teams while battling.");
            return;
        }
        if (hash == SecurityManager::member(waiting_name).hash) {
            SecurityManager::Member m = SecurityManager::member(waiting_name);

            m.modifyIP(ip().toLatin1());
            m.modifyDate(QDateTime::currentDateTime().toString(Qt::ISODate).toLatin1());
            m.hash = hash;
            setAuth(m.authority());
            SecurityManager::updateMember(m);

            loginSuccess();
        } else {
            emit info(id(), tr("authentication failed for %1").arg(waiting_name));
            sendMessage("Wrong password for this name. If you don't know the password, please change your name.");
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

QString Player::ip() const
{
    return myip;
}

QString Player::proxyIp() const
{
    return proxyip;
}

void Player::recvTeam(const ChangeTeamInfo &cinfo)
{
    /* If the guy is not logged in, obvious. If he is battling, he could make it so the points lost are on his other team */
    if (!isLoggedIn())
        return;

    QString oldName = name();

    if (cinfo.name && *cinfo.name != oldName && battling()) {
        sendMessage("You can't change names while battling.");
        return;
    }

    if (cinfo.name || cinfo.teams || cinfo.team) {
        cancelChallenges();
        cancelBattleSearch();
    }

    if (cinfo.color) {
        assignNewColor(*cinfo.color);
    }
    if (cinfo.info) {
        assignTrainerInfo(*cinfo.info);
    }

    if (!cinfo.name || cinfo.name->toLower() == oldName.toLower()) {
        /* Clears the wainting name in case it's not clear,
           else something bad could happen */
        waiting_name = "";

        //Still needs to deal with afterChangeTeam event
        ontologin = true;

        if (cinfo.teams) {
            m_teams.init(*cinfo.teams);
            findTierAndRating(true);
        } else if (cinfo.team && cinfo.teamNum < m_teams.count()) {
            m_teams.team(cinfo.teamNum) = *cinfo.team;

            QString oldTier = team(cinfo.teamNum).tier;
            findTier(cinfo.teamNum);

            if (oldTier != team(cinfo.teamNum).tier) {
                syncTiers(oldTier);
                findRating(team(cinfo.teamNum).tier);
            } else {
                if (cinfo.color || cinfo.info) {
                    emit updated(id());
                }
            }
        } else {
            emit updated(id());
        }

        return;
    }

    if (!testNameValidity(*cinfo.name))
        return;

    if (cinfo.teams) {
        m_teams.init(*cinfo.teams);
    } else if (cinfo.team && cinfo.teamNum < m_teams.count()) {
        m_teams.team(cinfo.teamNum) = *cinfo.team;
    }

    testAuthentification(*cinfo.name);
}

/* The old tier param is there for us to remove the old tier
  in the ratings if no team has it anymore.

  The alternative would be to check all of ratings' keys*/
void Player::syncTiers(QString oldTier)
{
    tiers.clear();

    for (int i = 0; i < teamCount(); i++) {
        tiers.insert(team(i).tier);
    }

    if (!tiers.contains(oldTier)) {
        ratings().remove(oldTier);
    }
}

void Player::spectatingRequested(int id)
{
    if (!isLoggedIn()) {
        return; //INVALID BEHAVIOR
    }
    if (battlesSpectated.size() >= 2 || (auth() >= 3 && battlesSpectated.size() >= 16)) {
        sendMessage(tr("You're already watching %1 battles!").arg(battlesSpectated.size()));
        return;
    }
    emit spectatingRequested(this->id(), id);
}

void Player::sendMessage(const QString &mess, bool html)
{
    relay().sendMessage(mess, html);
}

void Player::sendPlayers(const QVector<reference<PlayerInfo> > & bundles)
{
    relay().notify(NetworkServ::PlayersList, Expander<decltype(bundles)>(bundles));
}

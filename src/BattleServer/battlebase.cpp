#include <cassert>
#include "../Shared/battlecommands.h"
#include "battlebase.h"
#include "pluginmanager.h"
#include "battlefunctions.h"

using namespace BattleCommands;

typedef BattlePStorage BP;
typedef BattleBase::TurnMemory TM;

BattleBase::BattleBase()
{
    timer = NULL;
}

void BattleBase::init(const BattlePlayer &p1, const BattlePlayer &p2, const ChallengeInfo &c, int id, const TeamBattle &t1, const TeamBattle &t2, BattleServerPluginManager *pluginManager)
{
    publicId() = id;
    conf.avatar[0] = p1.avatar;
    conf.avatar[1] = p2.avatar;
    conf.setTeam(0, new TeamBattle(t1));
    conf.setTeam(1, new TeamBattle(t2));
    team(0).name = p1.name;
    team(1).name = p2.name;
    conf.ids[0] = p1.id;
    conf.ids[1] = p2.id;
    conf.teamOwnership = true;
    conf.gen = team(0).gen;
    conf.clauses = c.clauses;
    conf.mode = c.mode;

    restrictedCount = p1.restrictedCount;
    teamCount = p1.teamCount;
    restricted[0] = p1.restrictedPokes;
    restricted[1] = p2.restrictedPokes;
    ratings[0] = p1.rating;
    ratings[1] = p2.rating;
    winMessage[0] = p1.win;
    winMessage[1] = p2.win;
    loseMessage[0] = p1.lose;
    loseMessage[1] = p2.lose;
    tieMessage[0] = p1.tie;
    tieMessage[1] = p2.tie;
    attacked() = -1;
    attacker() = -1;
    selfKoer() = -1;
    drawer() = -1;
    forfeiter() = -1;
    weather = 0;
    weatherCount = -1;
    terrain = 0;
    terrainCount = -1;

    /* timers for battle timeout */
    timeleft[0] = 5*60;
    timeleft[1] = 5*60;
    timeStopped[0] = true;
    timeStopped[1] = true;

    conf.flags.setFlag(BattleConfiguration::Rated, c.rated);

    if (mode() == ChallengeInfo::Doubles) {
        numberOfSlots() = 4;
    } else if (mode() == ChallengeInfo::Triples) {
        numberOfSlots() = 6;
    } else {
        numberOfSlots() = 2;
    }

    if (team(0).tier == team(1).tier) {
        tier() = team(0).tier;
    }
    currentForcedSleepPoke[0] = -1;
    currentForcedSleepPoke[1] = -1;

    for (int i = 0; i < numberOfSlots(); i++) {
        options.push_back(BattleChoices());
        hasChoice.push_back(false);
        couldMove.push_back(false);
    }

    if (clauses() & ChallengeInfo::ChallengeCup) {
        team(0).generateRandom(gen());
        team(1).generateRandom(gen());
    } else {
        /* Make sure teams are valid and koed pokemon are pushed to the back */
        for (int i = 0; i < 2; i++) {
            for (int k = 0; k < teamCount; k++) {
                if (team(i).poke(k).ko()) {
                    for (int j=k+1;j<6;j++){
                        if (!team(i).poke(j).ko()) {
                            /* Using std::swap because local index switching + rearrange team = mess */
                            std::swap(team(i).poke(k), team(i).poke(j));
                            break;
                        }
                    }
                }
            }
        };

        if (clauses() & ChallengeInfo::ItemClause) {
            QSet<int> alreadyItems[2];
            for (int i = 0; i < 6; i++) {
                int o1 = team(0).poke(i).item();
                int o2 = team(1).poke(i).item();

                if (alreadyItems[0].contains(o1)) {
                    team(0).poke(i).item() = 0;
                } else {
                    alreadyItems[0].insert(o1);
                }
                if (alreadyItems[1].contains(o2)) {
                    team(1).poke(i).item() = 0;
                } else {
                    alreadyItems[1].insert(o2);
                }
            }
        }
        if (clauses() & ChallengeInfo::SpeciesClause) {
            QSet<int> alreadyPokes[2];
            for (int i = 0; i < 6; i++) {
                int o1 = PokemonInfo::OriginalForme(team(0).poke(i).num()).pokenum;
                int o2 = PokemonInfo::OriginalForme(team(1).poke(i).num()).pokenum;

                if (alreadyPokes[0].contains(o1)) {
                    team(0).poke(i).num() = Pokemon::NoPoke;
                } else {
                    alreadyPokes[0].insert(o1);
                }
                if (alreadyPokes[1].contains(o2)) {
                    team(1).poke(i).num() = Pokemon::NoPoke;
                } else {
                    alreadyPokes[1].insert(o2);
                }
            }
        }
    }

    if (tier().length() > 0) {
        int maxLevel = p1.maxlevel;

        if (maxLevel < 100) {
            for (int i = 0; i < 6; i ++) {
                if (team(0).poke(i).level() > maxLevel) {
                    team(0).poke(i).level() = maxLevel;
                    team(0).poke(i).updateStats(gen());
                }
                if (team(1).poke(i).level() > maxLevel) {
                    team(1).poke(i).level() = maxLevel;
                    team(1).poke(i).updateStats(gen());
                }
            }
        }
    }

    buildPlugins(pluginManager);

    applyingMoveStatMods = false;
}

BattleBase::~BattleBase()
{
    /* This code needs to be in the derived destructor */
    //onDestroy();
}

void BattleBase::beginTurn()
{
    turn() += 1;
    /* Resetting temporary variables */
    for (int i = 0; i < numberOfSlots(); i++) {
        turnMemory(i).clear();
        turnMem(i).reset();
        tmove(i).reset();
    }

    for (int i = 0; i < numberOfSlots(); i++) {
        callpeffects(i, i, "TurnSettings");
    }
    attackCount() = 0;

    requestChoices();

    /* preventing the players from cancelling (like when u-turn/Baton pass) */
    for (int i = 0; i < numberOfSlots(); i++)
        couldMove[i] = false;

    analyzeChoices();
}

void BattleBase::start(ContextSwitcher &ctx)
{
    emit sendBattleInfos(id(Player1), publicId(), id(Player2), team(Player1), configuration(), tier());
    emit sendBattleInfos(id(Player2), publicId(), id(Player1), team(Player2), configuration(), tier());

    notify(All, BlankMessage,0);

    if (tier().length()>0) {
        notify(All, TierSection, Player1, tier());
    } else {
        notify(All, TierSection, Player1, QString("Mixed %1").arg(GenInfo::Version(gen())));
    }

    notify(All, Rated, Player1, rated());
    notify(All, BlankMessage,0);

    /* Beginning of the battle! */
    turn() = 0; /* here for Truant */

    blocked() = false;

    timer = new QBasicTimer();
    /* We are only warned of new events every 5 seconds */
    timer->start(5000,this);

    ContextCallee::start(ctx);
}


/* The battle loop !! */
void BattleBase::run()
{
#ifdef Q_OS_WIN
    /* Under windows you need to do that, as rand is per-thread. But on linux it'd screw up the random thing and
        interfere with other battles */
    srand(time(NULL));
    /* Get rid of the first predictable values for a better rand*/
    for (int i = 0; i < 10; i++)
        rand();
#endif
    unsigned long array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = rand();
        array[i] |= (rand() << 16);
    }
    rand_generator.seed(array, 10);

    if (clauses() & ChallengeInfo::RearrangeTeams) {
        rearrangeTeams();
    }

    rearrangeTime() = false;

    initializeEndTurnFunctions();

    engageBattle();

    forever
    {
        beginTurn();

        endTurn();
    }
}

void BattleBase::onDestroy()
{
    terminate();
    /* In the case the thread has not quited yet (anyway should quit in like 1 nano second) */
    wait();

    foreach(BattlePStorage *p, calls) {
        delete p;
    }
    delete timer;
}

void BattleBase::buildPlugins(BattleServerPluginManager *p)
{
    plugins = p->getBattlePlugins(this);

    foreach(BattlePlugin *pl, plugins) {
        calls.push_back(new BattlePStorage(pl));
        //qDebug() << "Created battle storage " << calls.back() << " for battle " << this;
    }
}

void BattleBase::removePlugin(BattlePlugin *p)
{
    int index = plugins.indexOf(p);
    //qDebug() << "Removing plugins at index " << index << "(this = " << this << ")";

    if (index != -1) {
        //qDebug() << "Index is not -1";
        plugins.removeAt(index);
        delete calls.takeAt(index);
        //qDebug() << "Remaining plugin size after operation: " << calls.size();
    }
}

void BattleBase::callp(int function)
{
    foreach(BattlePStorage *p, calls) {
        if (p->call(function, this) == -1) {
            removePlugin(p->plugin);
        }
    }
}


int BattleBase::spot(int id) const
{
    if (conf.ids[0] == id) {
        return 0;
    } else if (conf.ids[1] == id) {
        return 1;
    } else {
        return -1;
    }
}

int BattleBase::slot(int player, int poke) const
{
    return player + poke*2;
}

int BattleBase::slotNum(int slot) const
{
    return slot/2;
}


TeamBattle &BattleBase::team(int spot)
{
    assert((spot >= 0));
    return *conf.teams[spot];
}

const TeamBattle &BattleBase::team(int spot) const
{
    return *conf.teams[spot];
}

const TeamBattle& BattleBase::pubteam(int id) const
{
    return team(spot(id));
}

QList<int> BattleBase::revs(int p) const
{
    int player = this->player(p);
    int opp = opponent(player);
    QList<int> ret;
    for (int i = 0; i < numberPerSide(); i++) {
        if (!koed(slot(opp, i))) {
            ret.push_back(slot(opp, i));
        }
    }

    return ret;
}


QList<int> BattleBase::allRevs(int p) const
{
    int player = this->player(p);
    int opp = opponent(player);
    QList<int> ret;
    for (int i = 0; i < numberPerSide(); i++) {
        ret.push_back(slot(opp, i));
    }
    return ret;
}

int BattleBase::opponent(int player) const
{
    return 1-player;
}

int BattleBase::partner(int spot) const
{
    return slot(player(spot), !(spot/2));
}

const PokeBattle & BattleBase::poke(int player, int poke) const
{
    return team(player).poke(poke);
}

PokeBattle & BattleBase::poke(int player, int poke)
{
    return team(player).poke(poke);
}

const PokeBattle &BattleBase::poke(int slot) const
{
    return team(player(slot)).poke(slot/2);
}

PokeBattle &BattleBase::poke(int slot)
{
    return team(player(slot)).poke(slot/2);
}

QHash<quint16, quint16>& BattleBase::items(int player)
{
    return team(player).items;
}

const QHash<quint16, quint16>& BattleBase::items(int player) const
{
    return team(player).items;
}


int BattleBase::id(int spot) const
{
    if (spot >= 2) {
        return spectators.value(spot).first;
    } else {
        return conf.ids[spot];
    }
}

int BattleBase::rating(int spot) const
{
    return ratings[spot];
}

bool BattleBase::rated() const
{
    return configuration().rated();
}

int BattleBase::player(int slot) const
{
    return slot % 2;
}

int BattleBase::randomOpponent(int slot) const
{
    QList<int> opps = revs(slot);
    if (opps.empty()) return -1;

    return opps[randint(opps.size())];
}

int BattleBase::randomValidOpponent(int slot) const
{
    QList<int> opps = revs(slot);
    if (opps.empty())
        return allRevs(slot).front();

    return opps[randint(opps.size())];
}

const BattleConfiguration &BattleBase::configuration() const
{
    return conf;
}


bool BattleBase::koed(int player) const
{
    return poke(player).ko();
}

int BattleBase::countAlive(int player) const
{
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (!poke(player, i).ko()) {
            count += 1;
        }
    }
    return count;
}

int BattleBase::countBackUp(int player) const
{
    int count = 0;
    for (int i = numberOfSlots()/2; i < 6; i++) {
        if (poke(player, i).num() != 0 && !poke(player, i).ko()) {
            count += 1;
        }
    }
    return count;
}


bool BattleBase::canTarget(int attack, int attacker, int defender) const
{
    if (MoveInfo::Flags(attack, gen()) & Move::PulsingFlag) {
        return true;
    }

    return areAdjacent(attacker, defender);
}

bool BattleBase::areAdjacent(int attacker, int defender) const
{
    return std::abs(slotNum(attacker)-slotNum(defender)) <= 1;
}

void BattleBase::yield()
{
    blocked() = true;
    ContextCallee::yield();
    testWin();
}

void BattleBase::schedule()
{
    blocked() = false;
    ContextCallee::schedule();
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
void BattleBase::startClock(int player, bool broadCoast)
{
    if (!(clauses() & ChallengeInfo::NoTimeOut) && timeStopped[player]) {
        startedAt[player] = time(NULL);
        timeStopped[player] = false;

        (void) broadCoast; // should be used to tell if we tell everyone or not, but meh.
#ifdef QT5
        notify(player,ClockStart, player, quint16(timeleft[player].load()));
#else
        notify(player,ClockStart, player, quint16(timeleft[player]));
#endif
    }
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
void BattleBase::stopClock(int player, bool broadCoast)
{
    if (!(clauses() & ChallengeInfo::NoTimeOut)) {
        if (!timeStopped[player]) {
            timeStopped[player] = true;
#ifdef QT5
            timeleft[player] = std::max(long(0),timeleft[player].load() - (time(NULL) - startedAt[player].load()));
#else
            timeleft[player] = std::max(long(0),timeleft[player] - (time(NULL) - startedAt[player]));
#endif
        }

#ifdef QT5
        if (broadCoast) {
            timeleft[player] = std::min(int(timeleft[player].load()+20), 5*60);
            notify(All,ClockStop,player,quint16(timeleft[player].load()));
        } else {
            notify(player, ClockStop, player, quint16(timeleft[player].load()));
        }
#else
        if (broadCoast) {
            timeleft[player] = std::min(int(timeleft[player]+20), 5*60);
            notify(All,ClockStop,player,quint16(timeleft[player]));
        } else {
            notify(player, ClockStop, player, quint16(timeleft[player]));
        }
#endif
    }
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
int BattleBase::timeLeft(int player)
{
#ifdef QT5
    if (timeStopped[player]) {
        return timeleft[player].load();
    } else {
        return timeleft[player].load() - (time(NULL) - startedAt[player].load());
    }
#else
    if (timeStopped[player]) {
        return timeleft[player];
    } else {
        return timeleft[player] - (time(NULL) - startedAt[player]);
    }
#endif
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
void BattleBase::timerEvent(QTimerEvent *)
{
    if (timeLeft(Player1) <= 0 || timeLeft(Player2) <= 0) {
        schedule(); // the battle is finished, isn't it?
    } else {
#ifdef QT5
        /* If a player takes too long - more than 30 secs - tell the other player the time remaining */
        if (timeStopped[Player1] && !timeStopped[Player2] && (time(NULL) - startedAt[Player2].load()) > 30) {
            notify(AllButPlayer, ClockStart, Player2, quint16(timeLeft(Player2)));
        } else if (timeStopped[Player2] && !timeStopped[Player1] && (time(NULL) - startedAt[Player1].load()) > 30) {
            notify(AllButPlayer, ClockStart, Player1, quint16(timeLeft(Player1)));
        }
#else
        /* If a player takes too long - more than 30 secs - tell the other player the time remaining */
        if (timeStopped[Player1] && !timeStopped[Player2] && (time(NULL) - startedAt[Player2]) > 30) {
            notify(AllButPlayer, ClockStart, Player2, quint16(timeLeft(Player2)));
        } else if (timeStopped[Player2] && !timeStopped[Player1] && (time(NULL) - startedAt[Player1]) > 30) {
            notify(AllButPlayer, ClockStart, Player1, quint16(timeLeft(Player1)));
        }
#endif
    }
}

/*************************************************
  End of the warning.
  ************************************************/

void BattleBase::testWin()
{
    if (forfeiter() != -1) {
        exit();
    }

    /* No one wants a battle that long xd */
    if (turn() == 1024) {
        endBattle(Tie, Player1, Player2);
    }

    /* Mutual Draw */
    if (drawer() == -2) {
        endBattle(Tie, Player1, Player2);
    }

    int time1 = std::max(0, timeLeft(Player1));
    int time2 = std::max(0, timeLeft(Player2));

    if (time1 == 0 || time2 == 0) {
        notify(All,ClockStop,Player1,quint16(time1));
        notify(All,ClockStop,Player2,quint16(time2));
        notifyClause(ChallengeInfo::NoTimeOut);
        if (time1 == 0 && time2 ==0) {
            endBattle(Tie, Player1, Player2);
        } else if (time1 == 0) {
            endBattle(Win, Player2, Player1);
        } else {
            endBattle(Win, Player1, Player2);
        }
    }

    int c1 = countAlive(Player1);
    int c2 = countAlive(Player2);

    if (c1*c2==0) {
        if (c1 + c2 == 0) {
            if ((clauses() & ChallengeInfo::SelfKO) && selfKoer() != -1) {
                notifyClause(ChallengeInfo::SelfKO);
                endBattle(Win, opponent(player(selfKoer())), player(selfKoer()));
            }
            endBattle(Tie, Player1, Player2);
        } else if (c1 == 0) {
            endBattle(Win, Player2, Player1);
        } else {
            endBattle(Win, Player1, Player2);
        }
    }
}

void BattleBase::endBattle(int result, int winner, int loser)
{
    int time1 = std::max(0, timeLeft(Player1));
    int time2 = std::max(0, timeLeft(Player2));
    notify(All,ClockStop,Player1,quint16(time1));
    notify(All,ClockStop,Player2,quint16(time2));
    if (result == Tie) {
        notify(All, BattleEnd, Player1, qint8(Tie));

        emit battleFinished(publicId(), Tie, id(Player1), id(Player2));
        if (!tieMessage[Player1].isEmpty()) {
            notify(All, EndMessage, Player1, tieMessage[Player1]);
        }
        if (!tieMessage[Player2].isEmpty()) {
            notify(All, EndMessage, Player2, tieMessage[Player2]);
        }
        exit();
    }
    if (result == Win || result == Forfeit) {
        notify(All, BattleEnd, winner, qint8(result));
        if (!winMessage[winner].isEmpty()) {
            notify(All, EndMessage, winner, winMessage[winner]);
        }
        if (!loseMessage[loser].isEmpty()) {
            notify(All, EndMessage, loser, loseMessage[loser]);
        }

        emit battleFinished(publicId(), result, id(winner), id(loser));
        exit();
    }
}

void BattleBase::notifyClause(int clause)
{
    notify(All, Clause, intlog2(clause));
}

void BattleBase::notifyKO(int player)
{
    changeStatus(player,Pokemon::Koed);
    notify(All, Ko, player);
}

void BattleBase::notifyMiss(bool multiTar, int player, int target)
{
    if (multiTar) {
        notify(All, Avoid, target);
    } else {
        notify(All, Miss, player);
    }
}

void BattleBase::emitCommand(int slot, int players, const QByteArray &toSend)
{
    if (players == All) {
        emit battleInfo(publicId(), qint32(id(Player1)), toSend);
        emit battleInfo(publicId(), qint32(id(Player2)), toSend);

        spectatorMutex.lock();

        QHashIterator<int, QPair<int, QString> > it(spectators);
        while(it.hasNext()) {
            emit battleInfo(publicId(), qint32(it.next().value().first), toSend);
        }
        spectatorMutex.unlock();
    } else if (players == AllButPlayer) {
        emit battleInfo(publicId(), qint32(id(opponent(player(slot)))), toSend);

        spectatorMutex.lock();
        QHashIterator<int, QPair<int, QString> >  it(spectators);
        while(it.hasNext()) {
            emit battleInfo(publicId(), qint32(it.next().value().first), toSend);
        }
        spectatorMutex.unlock();
    } else {
        emit battleInfo(publicId(), qint32(id(players)), toSend);
    }
    callp(BP::emitCommand, slot, players, toSend);
}

void BattleBase::changeStatus(int team, int poke, int status)
{
    if (isOut(team, poke)) {
        changeStatus(slot(team, poke), status);
    } else {
        this->poke(team, poke).changeStatus(status);
        notify(All, AbsStatusChange, team, qint8(poke), qint8(status));
        //Sleep clause
        if (status != Pokemon::Asleep && currentForcedSleepPoke[team] == currentInternalId(slot(team, poke))) {
            currentForcedSleepPoke[team] = -1;
        }
    }
}

bool BattleBase::acceptSpectator(int id, bool authed) const
{
    QMutexLocker m(&spectatorMutex);
    if (spectators.contains(spectatorKey(id)) || this->id(0) == id || this->id(1) == id)
        return false;
    if (authed)
        return true;
    return !(clauses() & ChallengeInfo::DisallowSpectator);
}

void BattleBase::notifyChoices(int p)
{
    bool canMove = false;
    for (int i = 0; i < numberPerSide(); i++) {
        int slot = this->slot(p, i);
        if (couldMove[slot]) {
            notify(p, OfferChoice, slot, options[slot]);
        }
        canMove |= hasChoice[slot];
    }

    if (canMove) {
        notify(All, StartChoices, p);
    }
}

void BattleBase::addSpectator(QPair<int, QString> p)
{
    /* Simple guard to avoid multithreading problems -- would need to be improved :s */
    if (!blocked() && !finished()) {
        pendingSpectators.append(p);
        QTimer::singleShot(100, this, SLOT(clearSpectatorQueue()));

        return;
    }

    int id = p.first;

    int key;

    if (configuration().isInBattle(id)) {
        /* Player was likely dced */
        emit sendBattleInfos(id, publicId(), this->id(opponent(spot(id))), team(spot(id)), configuration(), tier());
        key = spot(id);

        notifyChoices(key);
    } else {
        /* Assumption: each id is a different player, so key is unique */
        key = spectatorKey(id);

        if (spectators.contains(key)) {
            // Then a guy was put on waitlist and tried again, w/e don't accept him
            return;
        }

        spectators[key] = p;

        if (tier().length() > 0)
            notify(key, TierSection, Player1, tier());
        else
            notify(key, TierSection, Player1, QString("Mixed %1").arg(GenInfo::Version(gen())));

        notify(key, Rated, Player1, rated());

        typedef QPair<int, QString> pair;
        foreach (pair spec, spectators) {
            if (spec.first != id)
                notify(key, Spectating, 0, true, qint32(spec.first), spec.second);
        }
    }

    notify(All, Spectating, 0, true, qint32(id), p.second);

    notify(key, BlankMessage, 0);

    notifySituation(key);
}

void BattleBase::removeSpectator(int id)
{
    spectatorMutex.lock();
    spectators.remove(spectatorKey(id));
    spectatorMutex.unlock();

    notify(All, Spectating, 0, false, qint32(id));
}


void BattleBase::playerForfeit(int forfeiterId)
{
    if (finished()) {
        return;
    }
    forfeiter() = spot(forfeiterId);
    notify(All, BattleEnd, opponent(forfeiter()), qint8(Forfeit));
}


void BattleBase::rearrangeTeams()
{
    rearrangeTime() = true;
    /* Here we'll give the possibility to rearrange teams */
    notify(Player1,RearrangeTeam,Player2,ShallowShownTeam(team(Player2)));
    notify(Player2,RearrangeTeam,Player1,ShallowShownTeam(team(Player1)));

    for (int player = Player1; player <= Player2; player++) {
        couldMove[player] = true;
        hasChoice[player] = true;

        startClock(player);
    }

    yield();

    for (int player = Player1; player <= Player2; player++) {
        team(player).setIndexes(choice(slot(player)).choice.rearrange.pokeIndexes);

        startClock(player);
    }
}


bool BattleBase::canCancel(int player)
{
    if (!blocked())
        return false;
    if (rearrangeTime())
        return true;

    for (int i = 0; i < numberOfSlots()/2; i++) {
        if (couldMove[slot(player,i)])
            return true;
    }

    return false;
}

void BattleBase::cancel(int player)
{
    if (rearrangeTime()) {
        notify(player,RearrangeTeam,opponent(player),ShallowShownTeam(team(opponent(player))));
        hasChoice[slot(player, 0)] = true;
    } else {
        notify(player, CancelMove, player);

        for (int i = 0; i < numberOfSlots()/2; i++) {
            if (couldMove[slot(player, i)]) {
                hasChoice[slot(player, i)] = true;
            }
        }
    }
    if (drawer() == player) {
        drawer() = -1;
    }

    startClock(player,false);
}

void BattleBase::addDraw(int player)
{
    if (finished()) {
        return;
    }

    if (drawer() == -1) {
        drawer() = player;
        return;
    }
    if (drawer() != player) {
        drawer() = -2;
        schedule();
    }
}

bool BattleBase::validChoice(const BattleChoice &b)
{
    if (!couldMove[b.slot()] || !hasChoice[b.slot()]) {
        return false;
    }

    if (rearrangeTime()) {
        if (!b.rearrangeChoice())
            return false;
    } else if (!b.match(options[b.slot()])) {
        return false;
    }

    if (b.moveToCenterChoice()) {
        return mode() == ChallengeInfo::Triples && slotNum(b.slot()) != 1;
    }

    int player = this->player(b.slot());

    /* If it's a switch, we check the receiving poke valid, if it's a move, we check the target */
    if (b.switchChoice()) {
        if (isOut(player, b.pokeSlot()) || poke(player, b.pokeSlot()).ko()) {
            return false;
        }
        /* Let's also check another switch hasn't been made to the same poke */
        for (int i = 0; i < numberOfSlots(); i++) {
            int p2 = this->player(i);
            if (i != b.slot() && p2 == player && couldMove[i] && hasChoice[i] == false && choice(i).switchChoice()
                    && choice(i).pokeSlot() == b.pokeSlot()) {
                return false;
            }
        }
        return true;
    }

    if (b.attackingChoice()){
        /* It's an attack, we check the target is valid */
        if (b.target() < 0 || b.target() >= numberOfSlots())
            return false;
        /* If there's a mega evo, we verify another hasn't been already chosen */
        if (b.mega()) {
            for (int i = 0; i < numberOfSlots(); i++) {
                int p2 = this->player(i);
                if (i != b.slot() && p2 == player && couldMove[i] && hasChoice[i] == false && choice(i).attackingChoice()
                        && choice(i).mega()) {
                    return false;
                }
            }
        }
        return true;
    }

    if (b.rearrangeChoice()) {
        if (slotNum(b.slot()) != 0)
            return false;

        bool used[6] = {false};

        /* Checks all the 6 indexes are different */
        for (int i = 0; i < 6; i++) {
            int x = b.choice.rearrange.pokeIndexes[i];

            if (x < 0 || x >= 6)
                return false;

            if (used[x])
                return false;

            used[x] = true;
        }

        /* Checks restricted pokemon (like in VGC) are restricted to a certain
         *  number! */
        if (tier().length() > 0) {
            if (!(clauses() & ChallengeInfo::ChallengeCup) && restrictedCount > 0) {
                int nrestricted = 0;
                for (int i = 0; i < teamCount; i++) {
                    if ((1 << b.choice.rearrange.pokeIndexes[i]) & restricted[player]) {
                        nrestricted++;
                    }
                }

                if (nrestricted > restrictedCount) {
                    return false;
                }
            }
        }

        /* Checks koed pokemon aren't sent in the front */
        int cnt = 0;
        for (int i = 0; i < teamCount; i++) {
            if (!team(player).poke(b.choice.rearrange.pokeIndexes[i]).ko()) {
                cnt++;
            }
        }

        for (int i = 0; i < numberPerSide() && i < cnt; i++) {
            if (team(player).poke(b.choice.rearrange.pokeIndexes[i]).ko()) {
                return false;
            }
        }

        return true;
    }

    if (b.itemChoice()) {
        int count = items(player).value(b.item());
        /* In doubles & triples, check if the user doesn't exhaust the item. For example if you are in triples and you have
          two potions, you can only use it on two pokemon! */
        for (int i = 0; i < numberPerSide(); i++) {
            int s = slot(player,i);
            if (couldMove[s] && !hasChoice[s] && choice(s).itemChoice() && choice(s).item() == b.item()) {
                count --;
            }
        }
        if (count <= 0) {
            return false;
        }
        if (items(player).contains(b.item())) {
            int tar = ItemInfo::Target(b.item(), gen());
            int itar = b.itemTarget();
            if (tar == Item::TeamPokemon) {
                if (this->player(itar) != player || slotNum(itar) < 0 || slotNum(itar) >= 6 || poke(itar).num() == Pokemon::NoPoke) {
                    return false;
                }
            } else if (tar == Item::FieldPokemon) {
                if (itar < 0 || itar >= numberOfSlots() || this->player(itar) != player || koed(itar)) {
                    return false;
                }
            } else if (tar == Item::Attack) {
                if (this->player(itar) != player || slotNum(itar) < 0 || slotNum(itar) >= 6 || poke(itar).ko()) {
                    return false;
                }
                if (b.itemAttack() < 0 || b.itemAttack() >= 4 || poke(itar).move(b.itemAttack()).num() == Move::NoMove) {
                    return false;
                }
            } else if (tar == Item::Opponent) {
                /* A pokeball can't be thrown if there are two wild pokemon in front of you */
                if (countAlive(opponent(player)) > 1) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool BattleBase::isOut(int, int poke) const
{
    return poke < numberPerSide();
}

bool BattleBase::isOut(int poke) const
{
    return slotNum(poke) < numberPerSide();
}

void BattleBase::storeChoice(const BattleChoice &b)
{
    choice(b.slot()) = b;
    hasChoice[b.slot()] = false;
}

bool BattleBase::allChoicesOkForPlayer(int player)
{
    for (int i = 0; i < numberOfSlots()/2; i++) {
        if (hasChoice[slot(player, i)] != false)
            return false;
    }
    return true;
}

int BattleBase::currentInternalId(int slot) const
{
    return team(this->player(slot)).internalId(poke(slot));
}

bool BattleBase::allChoicesSet()
{
    for (int i = 0; i < numberOfSlots(); i++) {
        if (hasChoice[i])
            return false;
    }
    return true;
}

void BattleBase::clearSpectatorQueue()
{
    if (!blocked() && !finished()) {
        QTimer::singleShot(100, this, SLOT(clearSpectatorQueue()));
        return;
    }
    if (pendingSpectators.size() > 0) {
        auto copy = pendingSpectators;

        pendingSpectators.clear();

        foreach (auto p, copy) {
            addSpectator(p);
        }
    }
}

void BattleBase::battleChoiceReceived(int id, const BattleChoice &b)
{
    int player = spot(id);

    /* Simple guard to avoid multithreading problems */
    if (!blocked()) {
        return;
    }

    if (finished()) {
        return;
    }

    /* Clear the queue of pending spectators */
    clearSpectatorQueue();

    if (b.slot() < 0 || b.slot() >= numberOfSlots()) {
        return;
    }

    if (player != this->player(b.slot())) {
        /* W00T! He tried to impersonate the other! Bad Guy! */
        //notify(player, BattleChat, opponent(player), QString("Say, are you trying to hack this game? Beware, i'll report you and have you banned!"));
        return;
    }

    /* If the player wants a cancel, we grant it, if possible */
    if (b.cancelled()) {
        if (canCancel(player)) {
            cancel(player);
        }
        return;
    }

    if (b.drawChoice()) {
        addDraw(player);
        return;
    }

    if (!validChoice(b)) {
        if (canCancel(player))
            cancel(player);
        return;
    }

    storeChoice(b);

    if (allChoicesOkForPlayer(player)) {
        stopClock(player,false);
    }

    if (allChoicesSet()) {
        /* Blocking any further cancels */
        for (int i = 0; i < numberOfSlots(); i++) {
            if (couldMove[i]) {
                couldMove[i] = false;
                stopClock(this->player(i), true);
            }
        }
        schedule();
    }
}

void BattleBase::battleChat(int id, const QString &str)
{
    notify(All, BattleChat, spot(id), str);
}

void BattleBase::spectatingChat(int id, const QString &str)
{
    notify(All, SpectatorChat, id, qint32(id), str);
}

void BattleBase::sendMoveMessage(int move, int part, int src, int type, int foe, int other, const QString &q)
{
    if (foe == -1) {
        notify(All, MoveMessage, src, quint16(move), uchar(part), qint8(type));
    } else if (other == -1) {
        notify(All, MoveMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe));
    } else if (q == "") {
        notify(All, MoveMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe), qint16(other));
    } else {
        notify(All, MoveMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe), qint16(other), q);
    }
}

void BattleBase::sendAbMessage(int move, int part, int src, int foe, int type, int other)
{
    if (foe == -1) {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type));
    } else if (other == -1) {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe));
    } else {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe), qint16(other));
    }
}

void BattleBase::sendItemMessage(int move, int src, int part, int foe, int berry, int stat)
{
    if (foe ==-1)
        notify(All, ItemMessage, src, quint16(move), uchar(part));
    else if (berry == -1)
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe));
    else if (stat == -1)
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe), qint16(berry));
    else
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe), qint16(berry), qint32(stat));
}

void BattleBase::sendBerryMessage(int move, int src, int part, int foe, int berry, int stat)
{
    sendItemMessage(move+8000,src,part,foe,berry,stat);
}

void BattleBase::notifyFail(int p)
{
    notify(All, Failed, p);
}

void BattleBase::engageBattle()
{
    for (int i = 0; i < 6; i++) {
        if (i >= teamCount) {
            poke(Player1,i).num() = 0;
            poke(Player2,i).num() = 0;
        }
    }
    //qDebug() << "Engaging battle " << this << ", calling plugins";
    /* Plugin call */
    callp(BP::battleStarting);

    for (int i = 0; i < 6; i++) {
        if (poke(Player1,i).ko()) {
            changeStatus(Player1, i, Pokemon::Koed);
        }
        if (poke(Player2,i).ko()) {
            changeStatus(Player2, i, Pokemon::Koed);
        }
    }

    // Check for set weather.
    if(weather != NormalWeather) {
        sendMoveMessage(57, weather - 1, 0, TypeInfo::TypeForWeather(weather));
    }

    for (int i = 0; i < numberOfSlots()/2; i++) {
        if (!poke(Player1, i).ko())
            sendPoke(slot(Player1, i), i);
        if (!poke(Player2, i).ko())
            sendPoke(slot(Player2, i), i);
    }

    for (int i = 0; i< numberOfSlots(); i++) {
        hasChoice[i] = false;
    }
}

inline bool comparePair(const std::pair<int,int> & x, const std::pair<int,int> & y) {
    return x.second>y.second;
}

std::vector<int> BattleBase::sortedBySpeed()
{
    std::vector<int> ret;

    std::vector<std::pair<int, int> > speeds;

    for (int i =0; i < numberOfSlots(); i++) {
        if (!koed(i)) {
            speeds.push_back(std::pair<int, int>(i, getStat(i, Speed)));
        }
    }

    if (speeds.size() == 0)
        return std::move(ret);

    std::sort(speeds.begin(), speeds.end(), &comparePair);

    /* Now for the speed tie */
    for (unsigned i = 0; i < speeds.size()-1;) {
        unsigned  j;
        for (j = i+1; j < speeds.size(); j++) {
            if (speeds[j].second != speeds[i].second) {
                break;
            }
        }

        if (j != i +1) {
            std::random_shuffle(speeds.begin() + i, speeds.begin() + j);
        }

        i = j;
    }

    /* Now assigning, removing the pairs */
    for (unsigned i =0; i < speeds.size(); i++) {
        ret.push_back(speeds[i].first);
    }
    return std::move(ret);
}

bool BattleBase::attacking()
{
    return attacker() != -1;
}

void BattleBase::setupLongWeather(int weather)
{
    weatherCount = -1;
    this->weather = weather;
}

void BattleBase::setupItems(int player, const QHash<quint16, quint16> &items)
{
    this->items(player) = items;
}

void BattleBase::notifySituation(int key)
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            notify(key, AbsStatusChange, i, qint8(j), qint8(poke(i, j).status()));
        }
        for (int k = 0; k < numberOfSlots()/ 2; k++) {
            int s = slot(i,k);
            if (!koed(s) && !rearrangeTime()) {
                notify(key, SendOut, s, true, quint8(k), opoke(s, i, k));

                if (hasSubstitute(s))
                    notify(key, Substitute, s, hasSubstitute(s));
            }
        }
    }

    notifyInfos(key);
}

void BattleBase::notifyInfos(int tosend)
{
    for (int p = 0; p < numberOfSlots(); p++) {
        if (!koed(p)) {
            BattleDynamicInfo infos = constructInfo(p);
            notify(tosend, DynamicInfo, p, infos);
            if (tosend == All || tosend == player(p)) {
                BattleStats stats = constructStats(p);
                notify(player(p), DynamicStats, p, stats);
            }
        }
    }
}

BattleStats BattleBase::constructStats(int player)
{
    BattleStats ret;

    if (fpoke(player).flags & BasicPokeInfo::Transformed) {
        for (int i = 0; i < 6; i++) {
            ret.stats[i] = -1;
        }
    } else {
        for (int i = 1; i < 6; i++) {
            ret.stats[i] = getStat(player, i);
        }
    }

    return ret;
}

BattleDynamicInfo BattleBase::constructInfo(int slot)
{
    BattleDynamicInfo ret;

    for (int i = 0; i < 8; i++) {
        ret.boosts[i] = fpoke(slot).boosts[i];
    }

    ret.flags = 0;

    return ret;
}


void BattleBase::BasicPokeInfo::init(const PokeBattle &p, Pokemon::gen gen)
{
    id = p.num().toPokeRef();
    weight = PokemonInfo::Weight(p.num());
    type1 = PokemonInfo::Type1(p.num(), gen);
    type2 = PokemonInfo::Type2(p.num(), gen);
    types = QVector<int>() << type1 << type2;
    ability = p.ability();
    flags = 0;

    for (int i = 0; i < 4; i++) {
        moves[i] = p.move(i).num();
        pps[i] = p.move(i).PP();
    }

    for (int i = 1; i < 6; i++)
        stats[i] = p.normalStat(i);

    for (int i = 0; i < 6; i++) {
        dvs[i] = p.dvs()[i];
    }

    for (int i = 0; i < 8; i++) {
        boosts[i] = 0;
    }

    level = p.level();
    substituteLife = 0;
    lastMoveUsed = 0;

    if (gen <= 1) {
        if (p.status() == Pokemon::Paralysed) {
            stats[Speed] /= 4;
        } else if (p.status() == Pokemon::Burnt) {
            /* Burn reduction is at attack time */
            //stats[Attack] /= 2;
        }
    }
}

void BattleBase::BasicMoveInfo::reset()
{
    memset(this, 0, sizeof(*this));
}

bool BattleBase::hasSubstitute(int player)
{
    return !koed(player) && (fpoke(player).substitute() || fpoke(player).is(BasicPokeInfo::HadSubstitute));
}

ShallowBattlePoke BattleBase::opoke(int slot, int player, int i) const
{
    (void) slot;
    return poke(player, i);
}

void BattleBase::requestChoices()
{
    for (int i = 0; i < numberOfSlots(); i ++)
        couldMove[i] = false;

    int count = 0;

    for (int i = 0; i < numberOfSlots(); i++) {
        count += requestChoice(i, false);
    }

    if (!allChoicesOkForPlayer(Player1)) {
        notify(All, StartChoices, Player1);
    }

    if (!allChoicesOkForPlayer(Player2)) {
        notify(All, StartChoices, Player2);
    }

    if (count > 0) {
        /* Send a brief update on the status */
        notifyInfos();
        /* Lock until ALL choices are received */
        yield();
    }

    notify(All, BeginTurn, All, turn());

    /* Now all the players gonna do is analyzeChoice(int player) */
}


bool BattleBase::requestChoice(int slot, bool acquire, bool custom)
{
    drawer() = -1;

    int player = this->player(slot);

    if (koed(slot) && countBackUp(player) == 0) {
        return false;
    }

    /* Custom choices bypass forced choices */
    if (turnMem(slot).contains(TurnMemory::NoChoice) && !koed(slot) && !custom) {
        return false;
    }

    couldMove[slot] = true;
    hasChoice[slot] = true;

    if (!custom)
        options[slot] = createChoice(slot);

    notify(player, OfferChoice, slot, options[slot]);

    startClock(player);

    if (acquire) {
        notify(All, StartChoices, player);
        yield();
    }

    /* Now all the players gonna do is analyzeChoice(int player) */
    return true;
}

bool BattleBase::isMovePossible(int player, int slot)
{
    return PP(player, slot) > 0 && !turnMemory(player).contains(QString("Move%1Blocked").arg(slot));
}

int BattleBase::PP(int player, int slot) const
{
    if (isOut(player)) {
        return fpoke(player).pps[slot];
    } else {
        return poke(player).move(slot).PP();
    }
}


bool BattleBase::hasMove(int player, int move) {
    for (int i = 0; i < 4; i++) {
        if (this->move(player, i) == move) {
            return true;
        }
    }
    return false;
}

int BattleBase::move(int player, int slot)
{
    if (isOut(player)) {
        return fpoke(player).moves[slot];
    } else {
        return poke(player).move(slot).num();
    }
}


bool BattleBase::hasMoved(int p)
{
    return turnMem(p).contains(TurnMemory::HasMoved) || turnMem(p).contains(TurnMemory::Incapacitated);
}

void BattleBase::notifySub(int player, bool sub)
{
    notify(All, Substitute, player, sub);
}

void BattleBase::setupChoices()
{
    /* If there's no choice then the effects are already taken care of */
    for (int i = 0; i < numberOfSlots(); i++) {
        if (!koed(i) && !turnMem(i).contains(TurnMemory::NoChoice) && !turnMem(i).contains(TurnMemory::KeepAttack) && choice(i).attackingChoice()) {
            if (!options[i].struggle())
                setupMove(i, move(i,choice(i).pokeSlot()));
            else
                setupMove(i, Move::Struggle);
        }
    }
}

void BattleBase::healLife(int player, int healing)
{
    if (healing == 0) {
        healing = 1;
    }
    if (!koed(player) && !poke(player).isFull())
    {
        healing = std::min(healing, poke(player).totalLifePoints() - poke(player).lifePoints());
        changeHp(player, poke(player).lifePoints() + healing);
    }
}

void BattleBase::changeHp(int player, int newHp)
{
    if (newHp > poke(player).totalLifePoints()) {
        newHp = poke(player).totalLifePoints();
    }

    if (newHp == poke(player).lifePoints()) {
        /* no change, so don't bother */
        return;
    }
    poke(player).lifePoints() = newHp;

    notify(this->player(player), ChangeHp, player, quint16(newHp));
    if (isOut(player)) {
        /* percentage calculus */
        notify(AllButPlayer, ChangeHp, player, quint16(poke(player).lifePercent()));
    }
}

void BattleBase::inflictSubDamage(int player, int damage, int source)
{
    (void) source;

    int life = fpoke(player).substituteLife;

    if (life <= damage) {
        fpoke(player).remove(BasicPokeInfo::Substitute);
        sendMoveMessage(128, 1, player);
        notifySub(player, false);
    } else {
        fpoke(player).substituteLife = life-damage;
        sendMoveMessage(128, 3, player);
    }
}

void BattleBase::koPoke(int player, int source, bool straightattack)
{
    (void) source;

    if (poke(player).ko()) {
        return;
    }

    qint16 damage = poke(player).lifePoints();

    changeHp(player, 0);

    if (straightattack) {
        notify(this->player(player), StraightDamage,player, qint16(damage));
        notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
    }

    // for when to notify the ko
    if (!attacking() || tmove(attacker()).power == 0 || gen() >= 5) {
        notifyKO(player);
    }

    //useful for third gen
    turnMem(player).add(TurnMemory::WasKoed);
}

bool BattleBase::wasKoed(int player) const
{
    return turnMem(player).contains(TM::WasKoed);
}


void BattleBase::requestSwitchIns()
{
    testWin();
    selfKoer() = -1;

    QSet<int> koedPlayers;
    QSet<int> koedPokes;
    QSet<int> sentPokes;

    for (int i = 0; i < numberOfSlots(); i++) {
        if (!koedPlayers.contains(player(i)) && koed(i) && countBackUp(player(i)) > 0) {
            koedPlayers.insert(player(i));
            koedPokes.insert(i);
        }
    }

    while (koedPokes.size() > 0) {
        notifyInfos();

        foreach(int p, koedPokes) {
            requestChoice(p, false);
        }

        if (!allChoicesOkForPlayer(Player1)) {
            notify(All, StartChoices, Player1);
        }

        if (!allChoicesOkForPlayer(Player2)) {
            notify(All, StartChoices, Player2);
        }

        yield();

        /* To clear the cancellable moves list */
        for (int i = 0; i < numberOfSlots(); i++)
            couldMove[i] = false;

        foreach(int p, koedPokes) {
            analyzeChoice(p);

            if (!koed(p)) {
                sentPokes.insert(p);
            }
        }

        koedPokes.clear();
        koedPlayers.clear();

        testWin();

        for (int i = 0; i < numberOfSlots(); i++) {
            if (!koedPlayers.contains(player(i)) && koed(i) && countBackUp(player(i)) > 0) {
                koedPlayers.insert(player(i));
                koedPokes.insert(i);
            }
        }
    }

//    std::vector<int> sorted = sortedBySpeed();

    /* Each wave calls the abilities in order , then next wave and so on. */
//    foreach(int p, sorted) {
//        if (sentPokes.contains(p)) {
//            callEntryEffects(p);
//        }
//    }
}

void BattleBase::requestEndOfTurnSwitchIns()
{
    requestSwitchIns();
    speedsVector = sortedBySpeed();
}


void BattleBase::analyzeChoice(int slot)
{
    attackCount() += 1;
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice(slot).attackingChoice()) {
        if (!wasKoed(slot)) {
            if (turnMem(slot).contains(TM::NoChoice) || turnMem(slot).contains(TM::KeepAttack))
                /* Automatic move */
                if (turnMemory(slot).contains("AutomaticMove")) {
                    useAttack(slot, turnMemory(slot)["AutomaticMove"].toInt(), true);
                } else {
                    /* Automatic move */
                    useAttack(slot, fpoke(slot).lastMoveUsed, true);
                }
            else {
                if (options[slot].struggle()) {
                    setupMove(slot, Move::Struggle);
                    useAttack(slot, Move::Struggle, true);
                } else {
                    useAttack(slot, choice(slot).attackSlot());
                }
            }
        }
    } else if (choice(slot).switchChoice()){
        if (!koed(slot)) /* if the pokemon isn't ko, it IS sent back */
            sendBack(slot);

        sendPoke(slot, choice(slot).pokeSlot());
    } else if (choice(slot).itemChoice()) {
    } else {
        /* FATAL FATAL */
    }
}

void BattleBase::sendBack(int player, bool silent)
{
    notify(All, SendBack, player, silent);
}

bool BattleBase::testStatus(int player)
{
    if (turnMem(player).contains(TM::HasPassedStatus)) {
        return true;
    }

    if (poke(player).status() == Pokemon::Asleep) {
        if (poke(player).statusCount() > 1) {
            poke(player).statusCount() -= 1;
            notify(All, StatusMessage, player, qint8(FeelAsleep));

            return false;
        } else {
            healStatus(player, Pokemon::Asleep);
            notify(All, StatusMessage, player, qint8(FreeAsleep));

            return false;
        }
    }
    if (poke(player).status() == Pokemon::Frozen)
    {
        notify(All, StatusMessage, player, qint8(PrevFrozen));
        return false;
    }

    if (turnMem(player).contains(TM::Flinched)) {
        notify(All, Flinch, player);

        return false;
    }
    if (isConfused(player) && tmove(player).attack != 0) {
        if (pokeMemory(player)["ConfusedCount"].toInt() > 0) {
            inc(pokeMemory(player)["ConfusedCount"], -1);

            notify(All, StatusMessage, player, qint8(FeelConfusion));

            if (coinflip(1, 2)) {
                pokeMemory(player).remove("PetalDanceCount"); //RBY
                inflictConfusedDamage(player);
                return false;
            }
        } else {
            healConfused(player);
            notify(All, StatusMessage, player, qint8(FreeConfusion));
        }
    }

    if (poke(player).status() == Pokemon::Paralysed) {
        if (coinflip(1, 4)) {
            pokeMemory(player).remove("PetalDanceCount"); //RBY
            notify(All, StatusMessage, player, qint8(PrevParalysed));
            return false;
        }
    }

    return true;
}

void BattleBase::healStatus(int player, int status)
{
    if (poke(player).status() == status || status == 0) {
        changeStatus(this->player(player), slotNum(player), Pokemon::Fine);
    }
}

void BattleBase::healConfused(int player)
{
    poke(player).removeStatus(Pokemon::Confused);
    pokeMemory(player).remove("ConfusedCount");
}

bool BattleBase::isConfused(int player)
{
    return poke(player).hasStatus(Pokemon::Confused);
}

void BattleBase::inflictConfusedDamage(int player)
{
    notify(All, StatusMessage, player, qint8(HurtConfusion));

    tmove(player).type = Pokemon::Curse;
    tmove(player).power = 40;
    tmove(player).attack = Move::NoMove;
    turnMem(player).typeMod = 0;
    turnMem(player).stab = 2;
    tmove(player).category = Move::Physical;
    int damage = calculateDamage(player, player);
    inflictDamage(player, damage, player, true, gen() <= 1); //in RBY the damage is to the sub
}

void BattleBase::changeSprite(int player, Pokemon::uniqueId newForme)
{
    notify(All, ChangeTempPoke, player, quint8(TempSprite), newForme);
}

void BattleBase::losePP(int player, int move, int loss)
{
    int PP = this->PP(player, move);

    PP = std::max(PP-loss, 0);
    changePP(player, move, PP);
}

void BattleBase::changePP(int player, int move, int PP)
{
    if (isOut(player)) {
        fpoke(player).pps[move] = PP;
    }
    poke(player).move(move).PP() = PP;
    notify(this->player(player), ChangePP, player, quint8(move), quint8(this->PP(player, move)));
}

bool BattleBase::testFail(int player)
{
    if (turnMem(player).failed() == true) {
        /* Silently or not ? */
        notify(All, Failed, player, !turnMem(player).failingMessage());
        return true;
    }
    return false;
}

PokeFraction BattleBase::getStatBoost(int player, int stat)
{
    int boost = fpoke(player).boosts[stat];

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */

    if (stat <= 5) {
        return PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else if (stat == Accuracy) {
        /* Accuracy */
        return PokeFraction(std::max(3+boost, 3), std::max(3-boost, 3));
    } else {
        /* Evasion */
        return PokeFraction(std::max(3-boost, 3), std::max(3+boost, 3));
    }
}

void BattleBase::calculateTypeModStab(int orPlayer, int orTarget)
{
    int player = orPlayer == - 1 ? attacker() : orPlayer;
    int target = orTarget == - 1 ? attacked() : orTarget;

    int type = tmove(player).type; /* move type */
    int typeadv[] = {getType(target,1),getType(target,2)};
    int typepok[] = {getType(player,1),getType(player,2)};
    int typeffs[] = {TypeInfo::Eff(type, typeadv[0], gen()),TypeInfo::Eff(type, typeadv[1], gen())};
    int typemod = 0;

    for (int i = 0; i < 2; i++) {
        typemod += convertTypeEff(typeffs[i]);
    }

    // Counter hits regardless of type matchups in Gen 1.
    if (tmove(player).attack == Move::Counter) {
        typemod = 0;
    }

    int stab;
    if(type == Type::Curse) {
        // Not only Curse -- PO also has Struggle and second type
        // of single-typed pkmn as the ???-type
        stab = 2;
    }else{
        stab = 2 + (type==typepok[0] || type==typepok[1]);
    }

    turnMem(player).stab = stab;
    turnMem(player).typeMod = typemod; /* is attack effective? or not? etc. */
}

int BattleBase::convertTypeEff(int typeeff)
{
    if (typeeff == 0) {
        return -100;
    }
    return std::min(typeeff-2, 1);
}

int BattleBase::repeatNum(int player)
{
    if (tmove(player).repeatMin == 0)
        return 1;

    int min = tmove(player).repeatMin;
    int max = tmove(player).repeatMax;

    return minMax(min, max, gen().num, randint());
}

void BattleBase::testCritical(int player, int target)
{
    (void) target;

    /* In RBY, Focus Energy reduces crit by 75%; in statium, it's * 4 */
    int up (1), down(1);
    if (tmove(player).critRaise & 1) {
        up *= 8;
    }
    if (tmove(player).critRaise & 2) {
        if (gen() == Gen::RedBlue || gen() == Gen::Yellow) {
            down = 4;
        } else {
            up *= 4;
        }
    }
    PokeFraction critChance(up, down);
    int randnum = randint(512);
    int baseSpeed = PokemonInfo::BaseStats(fpoke(player).id, gen()).baseSpeed();
    bool critical = randnum < std::min(510, baseSpeed * critChance);

    if (critical) {
        turnMem(player).add(TM::CriticalHit);
        notify(All, CriticalHit, player);
    } else {
        turnMem(player).remove(TM::CriticalHit);
    }
}

void BattleBase::healDamage(int player, int target)
{
    int healing = tmove(player).healing;

    if ((healing > 0 && koed(target)) || (healing < 0 && koed(player)))
        return;

    if (healing > 0) {
        //In RBY, if the HP difference is 255, it fails
        if(poke(target).lifePoints() < poke(target).totalLifePoints() && ((gen() != Gen::RedBlue && gen() != Gen::Yellow)
                                                                          || (poke(target).totalLifePoints()-poke(target).lifePoints()) % 256 != 255)) {
            sendMoveMessage(60, 0, target, tmove(player).type);

            int damage = poke(target).totalLifePoints() * healing / 100;

            if (gen() >= 5 && damage * 100 / healing < poke(target).totalLifePoints())
                damage += 1;

            healLife(target, damage);
        }else{
            // No HP to heal
            notifyFail(player);
        }
    } else if (healing < 0){
        notify(All, Recoil, player, true);
        inflictDamage(player, -poke(player).totalLifePoints() * healing / 100, player);
    }
}

void BattleBase::unthaw(int player)
{
    notify(All, StatusMessage, player, qint8(FreeFrozen));
    healStatus(player, Pokemon::Frozen);
}

void BattleBase::notifyHits(int spot, int number)
{
    notify(All, Hit, spot, quint8(number));
}


void BattleBase::testFlinch(int player, int target)
{
    int rate = tmove(player).flinchRate;

    if (rate && coinflip(rate*255/100, 256)) {
        turnMem(target).add(TM::Flinched);
        pokeMemory(target).remove("Recharging"); //Flinch remove Recharge Turn for Hyper Beam in RBY
    }
}

void BattleBase::applyMoveStatMods(int player, int target)
{
    applyingMoveStatMods = true;
    bool sub = hasSubstitute(target);

    BasicMoveInfo &fm = tmove(player);

    int cl= fm.classification;

    /* First we check if there's even an effect... */
    if (cl != Move::StatAndStatusMove && cl != Move::StatChangingMove && cl != Move::StatusInducingMove
            && cl != Move::OffensiveSelfStatChangingMove && cl != Move::OffensiveStatusInducingMove
            && cl != Move::OffensiveStatChangingMove)
    {
        applyingMoveStatMods = false;
        return;
    }

    if (cl == Move::OffensiveSelfStatChangingMove) {
        target = player;
    }

    if (koed(target)) {
        applyingMoveStatMods = false;
        return;
    }

    /* Doing Stat Changes */
    for (int i = 2; i >= 0; i--) {
        char stat = fm.statAffected >> (i*8);

        if (!stat)
            break;

        signed char increase = char (fm.boostOfStat >> (i*8));

        int rate = char (fm.rateOfStat >> (i*8));

        if (increase < 0 && target != player && sub) {
            if (rate == 0 && cl != Move::OffensiveStatusInducingMove) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
            }
            applyingMoveStatMods = false;
            return;
        }

        if (rate != 0 && rate != 100 && !coinflip(rate*255/100, 256)) {
            continue;
        }

        if (increase > 0) {
            if (stat == AllStats) {
                for (int i = 1; i <= 5; i++) {
                    inflictStatMod(target, i, increase, player);
                }
            } else {
                inflictStatMod(target, stat, increase, player);
            }
        } else {
            /* If we are blocked by a secondary effect, let's stop here */
            if (!inflictStatMod(target, stat, increase, player)) {
                //return; or not, hyper cutter blocks only a part of tickle
            }
        }
    }

    /* Now Status */

    if (cl != Move::StatAndStatusMove && cl != Move::StatusInducingMove && cl != Move::OffensiveStatusInducingMove) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.status > Pokemon::Confused || fm.status == 0) {
        applyingMoveStatMods = false;
        return; // Other status effects than status and confusion are, on PO, dealt as special moves. Should probably be changed
    }

    int rate = fm.rate;

    if (target != player && sub) {
        bool fail = false;

        if (cl == Move::OffensiveStatusInducingMove) {
            //Secondary status
            fail = fm.status == Pokemon::Poisoned || fm.status == Pokemon::Paralysed || fm.status == Pokemon::Burnt || fm.status == Pokemon::Frozen;
        } else if (cl == Move::StatusInducingMove) {
            //Primary status
            fail = fm.status == Pokemon::Poisoned || fm.status == Pokemon::Confused;
        }
        if (fail) {
            if (rate == 0 && cl != Move::OffensiveStatChangingMove) {
                sendMoveMessage(128, 2, player,0,target, tmove(player).attack);
            }
            applyingMoveStatMods = false;
            return;
        }
    }

    /* Then we check if the effect hits */

    if (rate != 0 && rate != 100 && !coinflip(rate*255/100, 256)) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.status == Pokemon::Confused)
        inflictConfused(target, player, true);
    else
        inflictStatus(target, fm.status, player, fm.minTurns, fm.maxTurns);

    applyingMoveStatMods = false;
}

void BattleBase::inflictConfused(int player, int attacker, bool tell)
{
    //fixme: insomnia/owntempo/...
    if (isConfused(player)) {
        if (this->attacker() == attacker && attacker != player && canSendPreventSMessage(player, attacker))
        {
            notify(All, AlreadyStatusMessage, player, quint8(Pokemon::Confused));
        }
        return;
    }

    poke(player).addStatus(Pokemon::Confused);
    pokeMemory(player)["ConfusedCount"] = randint(4) + 1;

    notify(All, StatusChange, player, qint8(Pokemon::Confused), true, !tell);
}

bool BattleBase::canSendPreventSMessage(int, int attacker)
{
    return attacking() && tmove(attacker).rate == 0;
}

void BattleBase::inflictStatus(int player, int status, int attacker, int minTurns, int maxTurns)
{
    //fixme: mist + intimidate
    if (poke(player).status() != Pokemon::Fine) {
        if (this->attacker() == attacker &&
                (tmove(attacker).classification == Move::StatusInducingMove ||
                 tmove(attacker).classification == Move::StatAndStatusMove) && canSendPreventSMessage(player, attacker)) {
            if (poke(player).status() == status) {
                notify(All, AlreadyStatusMessage, player, quint8(poke(player).status()));
            }
            else {
                notify(All, Failed, player);
            }
        }
        return;
    }
    if (!canGetStatus(player, status))
        return;

    if (status == Pokemon::Asleep)
    {
        if (sleepClause() && currentForcedSleepPoke[this->player(player)] != -1) {
            notifyClause(ChallengeInfo::SleepClause);
            return;
        } else {
            currentForcedSleepPoke[this->player(player)] = currentInternalId(player);
            pokeMemory(player).remove("Recharging"); //For RBY Hyper Beam
        }
    } else if (status == Pokemon::Frozen)
    {
        if (clauses() & ChallengeInfo::FreezeClause) {
            for (int i = 0; i < 6; i++) {
                if (poke(this->player(player),i).status() == Pokemon::Frozen) {
                    notifyClause(ChallengeInfo::FreezeClause);
                    return;
                }
            }
        }
    }

    changeStatus(player, status, true, minTurns == 0 ? 0 : minTurns-1 + randint(maxTurns - minTurns + 1));
}

bool BattleBase::canGetStatus(int player, int status)
{
    switch (status) {
    case Pokemon::Burnt: return !hasType(player, Pokemon::Fire);
    case Pokemon::Poisoned: return !hasType(player, Pokemon::Poison);
    case Pokemon::Frozen: return !hasType(player, Pokemon::Ice);
    default:
        return true;
    }
}

bool BattleBase::hasType(int player, int type) const
{
    return getTypes(player).indexOf(type) != -1;
}

int BattleBase::getType(int player, int slot) const
{
    return fpoke(player).types.count() >= slot ? fpoke(player).types[slot-1] : Type::Curse;
}

QVector<int> BattleBase::getTypes(int player) const
{
    return fpoke(player).types;
}

bool BattleBase::inflictStatMod(int player, int stat, int mod, int attacker, bool tell)
{
    /* Gen 1 has only Special, which means no Satk or Sdef.
       For simplicity we map all Sdef changes to Satk and treat
       Satk as meaning "Special". */
    if (gen().num == 1 && stat == SpDefense) {
        stat = SpAttack;
    }

    if (mod > 0)
        return gainStatMod(player, stat, std::abs(mod), attacker, tell);
    else
        return loseStatMod(player, stat, std::abs(mod), attacker, tell);
}



bool BattleBase::gainStatMod(int player, int stat, int bonus, int , bool tell)
{
    int boost = fpoke(player).boosts[stat];
    if (boost < 6 && (gen() > 2 || getStat(player, stat) < 999)) {
        notify(All, StatChange, player, qint8(stat), qint8(bonus), !tell);
        changeStatMod(player, stat, std::min(boost+bonus, 6));
    } else {
        notify(All, WontGoHigher, player, qint8(stat));
    }

    return true;
}

void BattleBase::changeStatMod(int player, int stat, int newstat)
{
    fpoke(player).boosts[stat] = newstat;
}

void BattleBase::failSilently(int player)
{
    turnMem(player).remove(TM::FailingMessage);
    turnMem(player).add(TM::Failed);
}


void BattleBase::changeTempMove(int player, int slot, int move)
{
    fpoke(player).moves[slot] = move;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempMove), quint8(slot), quint16(move));
    changePP(player,slot,std::min(MoveInfo::PP(move, gen()), 5));
}

Pokemon::uniqueId BattleBase::pokenum(int player) {
    return fpoke(player).id;
}

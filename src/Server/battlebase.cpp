#include "battlebase.h"
#include "../Shared/battlecommands.h"
#include "pluginmanager.h"

using namespace BattleCommands;

typedef BattlePStorage BP;


BattleBase::BattleBase()
{
    timer = NULL;
}

BattleBase::~BattleBase()
{
    /* This code needs to be in the derived destructor */
    //onDestroy();
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

void BattleBase::buildPlugins(PluginManager *p)
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
        notify(player,ClockStart, player, quint16(timeleft[player]));
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
            timeleft[player] = std::max(0,timeleft[player] - (QAtomicInt(time(NULL)) - startedAt[player]));
        }

        if (broadCoast) {
            timeleft[player] = std::min(int(timeleft[player]+20), 5*60);
            notify(All,ClockStop,player,quint16(timeleft[player]));
        } else {
            notify(player, ClockStop, player, quint16(timeleft[player]));
        }
    }
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
int BattleBase::timeLeft(int player)
{
    if (timeStopped[player]) {
        return timeleft[player];
    } else {
        return timeleft[player] - (QAtomicInt(time(NULL)) - startedAt[player]);
    }
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
        /* If a player takes too long - more than 30 secs - tell the other player the time remaining */
        if (timeStopped[Player1] && !timeStopped[Player2] && (QAtomicInt(time(NULL)) - startedAt[Player2]) > 30) {
            notify(Player1, ClockStop, Player2, quint16(timeLeft(Player2)));
        } else if (timeStopped[Player2] && !timeStopped[Player1] && (QAtomicInt(time(NULL)) - startedAt[Player1]) > 30) {
            notify(Player2, ClockStop, Player1, quint16(timeLeft(Player1)));
        }
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
    notify(All,ClockStop,Player1,time1);
    notify(All,ClockStop,Player2,time2);
    if (result == Tie) {
        notify(All, BattleEnd, Player1, qint8(Tie));

        emit battleFinished(publicId(), Tie, id(Player1), id(Player2));
        exit();
    }
    if (result == Win || result == Forfeit) {
        notify(All, BattleEnd, winner, qint8(result));
        notify(All, EndMessage, winner, winMessage[winner]);
        notify(All, EndMessage, loser, loseMessage[loser]);

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

void BattleBase::changeStatus(int player, int status, bool tell, int turns)
{
    (void) player;
    (void) status;
    (void) tell;
    (void) turns;
}

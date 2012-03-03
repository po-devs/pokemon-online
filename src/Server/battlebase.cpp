#include "player.h"
#include "battlebase.h"
#include "../Shared/battlecommands.h"
#include "pluginmanager.h"
#include "tiermachine.h"
#include "tier.h"

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


/* The battle loop !! */
void BattleBase::run()
{
#ifdef WIN32
    /* Under windows you need to do that, as rand is per-thread. But on linux it'd screw up the random thing and
        interfere with other battles */
    srand(time(NULL));
    /* Get rid of the first predictable values for a better rand*/
    for (int i = 0; i < 10; i++)
        rand();
#else
# ifdef WIN64
    /* Under windows you need to do that, as rand is per-thread. But on linux it'd screw up the random thing */
    srand(time(NULL));
    for (int i = 0; i < 10; i++)
        rand();
# endif
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


bool BattleBase::acceptSpectator(int id, bool authed) const
{
    QMutexLocker m(&spectatorMutex);
    if (spectators.contains(spectatorKey(id)) || this->id(0) == id || this->id(1) == id)
        return false;
    if (authed)
        return true;
    return !(clauses() & ChallengeInfo::DisallowSpectator);
}

void BattleBase::addSpectator(Player *p)
{
    /* Simple guard to avoid multithreading problems -- would need to be improved :s */
    if (!blocked() && !finished()) {
        pendingSpectators.append(QPointer<Player>(p));
        QTimer::singleShot(100, this, SLOT(clearSpectatorQueue()));

        return;
    }

    int id = p->id();

    int key;

    if (configuration().isInBattle(id)) {
        p->startBattle(publicId(), this->id(opponent(spot(id))), team(spot(id)), configuration());
        key = spot(id);
    } else {
        /* Assumption: each id is a different player, so key is unique */
        key = spectatorKey(id);

        if (spectators.contains(key)) {
            // Then a guy was put on waitlist and tried again, w/e don't accept him
            return;
        }

        spectators[key] = QPair<int, QString>(id, p->name());

        p->spectateBattle(publicId(), configuration());

        if (tier().length() > 0)
            notify(key, TierSection, Player1, tier());

        notify(key, Rated, Player1, rated());

        typedef QPair<int, QString> pair;
        foreach (pair spec, spectators) {
            if (spec.first != id)
                notify(key, Spectating, 0, true, qint32(spec.first), spec.second);
        }
    }

    notify(All, Spectating, 0, true, qint32(id), p->name());

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

        if (tier().length() > 0) {
            team(player).setIndexes(b.choice.rearrange.pokeIndexes);
            if (!TierMachine::obj()->isValid(team(player), tier()) && !(clauses() & ChallengeInfo::ChallengeCup)) {
                team(player).resetIndexes();
                return false;
            } else {
                team(player).resetIndexes();
                return true;
            }
        }

        return true;
    }

    return false;
}

bool BattleBase::isOut(int, int poke)
{
    return poke < numberOfSlots()/2;
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
        QList<QPointer<Player> > copy = pendingSpectators;

        pendingSpectators.clear();

        foreach (QPointer<Player> p, copy) {
            if (p) {
                addSpectator(p);
            }
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
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe), qint16(berry), qint16(stat));
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
    if (tier().length() != 0) {
        Tier &t = TierMachine::obj()->tier(tier());

        t.fixTeam(team(0));
        t.fixTeam(team(1));
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

std::vector<int> && BattleBase::sortedBySpeed()
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
    for (unsigned i = 0; i < speeds.size()-1; ) {
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

#include "battle.h"
#include "player.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "tiermachine.h"
#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>

BattleSituation::BattleSituation(Player &p1, Player &p2, const ChallengeInfo &c, int id)
    : /*spectatorMutex(QMutex::Recursive), */team1(p1.team()), team2(p2.team())
{
    publicId() = id;
    timer = NULL;
    myid[0] = p1.id();
    myid[1] = p2.id();
    winMessage[0] = p1.winningMessage();
    winMessage[1] = p2.winningMessage();
    loseMessage[0] = p1.losingMessage();
    loseMessage[1] = p2.losingMessage();
    attacked() = -1;
    attacker() = -1;
    gen() = std::max(p1.gen(), p2.gen());
    applyingMoveStatMods = false;
    weather = 0;
    weatherCount = -1;

    /* timers for battle timeout */
    timeleft[0] = 5*60;
    timeleft[1] = 5*60;
    timeStopped[0] = true;
    timeStopped[1] = true;
    clauses() = c.clauses;
    rated() = c.rated;
    doubles() = c.mode == ChallengeInfo::Doubles;
    numberOfSlots() = doubles() ? 4 : 2;
    if (rated())
        tier() = p1.tier();
    currentForcedSleepPoke[0] = -1;
    currentForcedSleepPoke[1] = -1;
    p1.addBattle(publicId());
    p2.addBattle(publicId());

    for (int i = 0; i < numberOfSlots(); i++) {
        mycurrentpoke.push_back(-1);
        options.push_back(BattleChoices());
        slotzone.push_back(context());
        pokelong.push_back(context());
        turnlong.push_back(context());
        fieldpokes.push_back(BasicPokeInfo());
        fieldmoves.push_back(BasicMoveInfo());
        choice.push_back(BattleChoice());
        hasChoice.push_back(false);
        couldMove.push_back(false);
    }

    if (clauses() & ChallengeInfo::ChallengeCup) {
        team1.generateRandom(gen());
        team2.generateRandom(gen());
    } else {
        if (clauses() & ChallengeInfo::ItemClause) {
            QSet<int> alreadyItems[2];
            for (int i = 0; i < 6; i++) {
                int o1 = team1.poke(i).item();
                int o2 = team2.poke(i).item();

                if (alreadyItems[0].contains(o1)) {
                    team1.poke(i).item() = 0;
                } else {
                    alreadyItems[0].insert(o1);
                }
                if (alreadyItems[1].contains(o2)) {
                    team2.poke(i).item() = 0;
                } else {
                    alreadyItems[1].insert(o2);
                }
            }
        }
        if (clauses() & ChallengeInfo::SpeciesClause) {
            QSet<int> alreadyPokes[2];
            for (int i = 0; i < 6; i++) {
                int o1 = PokemonInfo::OriginalForme(team1.poke(i).num()).pokenum;
                int o2 = PokemonInfo::OriginalForme(team2.poke(i).num()).pokenum;

                if (alreadyPokes[0].contains(o1)) {
                    team1.poke(i).num() = Pokemon::NoPoke;
                } else {
                    alreadyPokes[0].insert(o1);
                }
                if (alreadyPokes[1].contains(o2)) {
                    team2.poke(i).num() = Pokemon::NoPoke;
                } else {
                    alreadyPokes[1].insert(o2);
                }
            }
        }
    }
}

BattleSituation::~BattleSituation()
{
    terminate();
    /* In the case the thread has not quited yet (anyway should quit in like 1 nano second) */
    wait();
    delete timer;
}

void BattleSituation::start(ContextSwitcher &ctx)
{
    for (int i = 0; i < 6; i++) {
        if (poke(Player1,i).ko()) {
            changeStatus(Player1, i, Pokemon::Koed);
        }
        if (poke(Player2,i).ko()) {
            changeStatus(Player2, i, Pokemon::Koed);
        }
    }

    notify(All, BlankMessage,0);

    if (rated()) {
        notify(All, TierSection, Player1, tier());

        QPair<int,int> firstChange = TierMachine::obj()->pointChangeEstimate(team1.name, team2.name, tier());
        QPair<int,int> secondChange = TierMachine::obj()->pointChangeEstimate(team2.name, team1.name, tier());

        notify(Player1, PointEstimate, Player1, qint8(firstChange.first), qint8(firstChange.second));
        notify(Player2, PointEstimate, Player2, qint8(secondChange.first), qint8(secondChange.second));
    }

    notify(All, Rated, Player1, rated());

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        //False for saying this is not in battle message but rule message
        if (clauses() & (1 << i))
            notify(All, Clause, i, false);
    }


    notify(All, BlankMessage,0);

    /* Beginning of the battle! */
    turn() = 0; /* here for Truant */

    sendPoke(slot(Player1), 0);
    sendPoke(slot(Player2), 0);
    if (doubles()) {
        if (!poke(Player1, 1).ko())
            sendPoke(slot(Player1, 1), 1);
        if (!poke(Player2, 1).ko())
            sendPoke(slot(Player2, 1), 1);
    }

    /* For example, if two pokemons are brought out
       with a weather ability, the slower one acts last */
    std::vector<int> pokes = sortedBySpeed();

    foreach(int p, pokes)
        callEntryEffects(p);;

    for (int i = 0; i< numberOfSlots(); i++) {
        hasChoice[i] = false;
    }

    blocked() = false;

    timer = new QBasicTimer();
    /* We are only warned of new events every 5 seconds */
    timer->start(5000,this);

    ContextCallee::start(ctx);
}

int BattleSituation::spot(int id) const
{
    if (myid[0] == id) {
	return 0;
    } else if (myid[1] == id) {
	return 1;
    } else {
	return -1;
    }
}

int BattleSituation::slot(int player, int poke) const
{
    return player + poke*2;
}

bool BattleSituation::acceptSpectator(int id, bool authed) const
{
    QMutexLocker m(&spectatorMutex);
    if (spectators.contains(spectatorKey(id)) || this->id(0) == id || this->id(1) == id)
        return false;
    if (authed)
        return true;
    return !(clauses() & ChallengeInfo::DisallowSpectator);
}

void BattleSituation::notifyClause(int clause, bool active)
{
    notify(All, Clause,active? intlog2(clause) : clause, active);
}

void BattleSituation::addSpectator(Player *p)
{
    /* Simple guard to avoid multithreading problems -- would need to be improved :s */
    if (!blocked() && !finished()) {
        pendingSpectators.append(QPointer<Player>(p));
        return;
    }

    p->spectateBattle(publicId(), configuration());
    int id = p->id();

    /* Assumption: each id is a different player, so key is unique */
    int key = spectatorKey(id);

    if (spectators.contains(key)) {
        // Then a guy was put on waitlist and tried again, w/e don't accept him
        return;
    }
    spectators[key] = id;

    notify(key, Rated, Player1, rated());
    if (rated())
        notify(key, TierSection, Player1, tier());

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        //False for saying this is not in battle message but rule message
        if (clauses() & (1 << i))
            notify(key, Clause, i, false);
    }


    foreach (int specId, spectators) {
        if (specId != id)
            notify(key, Spectating, 0, true, qint32(specId));
    }

    notify(All, Spectating, 0, true, qint32(id));
    notify(key, BlankMessage, 0);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            notify(key, AbsStatusChange, i, qint8(j), qint8(poke(i, j).status()));
        }
        for (int k = 0; k < numberOfSlots()/ 2; k++) {
            int s = slot(i,k);
            if (!koed(s)) {
                notify(key, SendOut, s, false, quint8(currentPoke(s)), opoke(i, currentPoke(s)));
                notify(key, ChangeTempPoke,s, quint8(TempSprite),pokenum(s));
            }
        }
    }
}

void BattleSituation::removeSpectator(int id)
{
    qDebug() << "Removing spectator";
    spectatorMutex.lock();
    spectators.remove(spectatorKey(id));
    spectatorMutex.unlock();

    notify(All, Spectating, 0, false, qint32(id));
}

int BattleSituation::id(int spot) const
{
    if (spot >= 2) {
        return spectators.value(spot);
    } else {
        return myid[spot];
    }
}

int BattleSituation::player(int slot) const
{
    return slot % 2;
}

int BattleSituation::randomOpponent(int slot) const
{
    QList<int> opps = revs(slot);
    if (opps.empty())
        return -1;

    return opps[true_rand()%opps.size()];
}

int BattleSituation::randomValidOpponent(int slot) const
{
    QList<int> opps = revs(slot);
    if (opps.empty())
        return allRevs(slot).front();

    return opps[true_rand()%opps.size()];
}

TeamBattle &BattleSituation::team(int spot)
{
    if (spot == 0) {
	return team1;
    } else {
	return team2;
    }
}

const TeamBattle &BattleSituation::team(int spot) const
{
    if (spot == 0) {
	return team1;
    } else {
	return team2;
    }
}

const TeamBattle& BattleSituation::pubteam(int id)
{
    return team(spot(id));
}

QList<int> BattleSituation::revs(int p) const
{
    int player = this->player(p);
    int opp = opponent(player);
    QList<int> ret;
    if (!koed(slot(opp)))
        ret.push_back(slot(opp));
    if (doubles() && !koed(slot(opp, 1))) {
        ret.push_back(slot(opp, 1));
    }
    return ret;
}


QList<int> BattleSituation::allRevs(int p) const
{
    int player = this->player(p);
    int opp = opponent(player);
    QList<int> ret;
    ret.push_back(slot(opp));
    if (doubles()) {
        ret.push_back(slot(opp, true));
    }
    return ret;
}

int BattleSituation::opponent(int player) const
{
    return 1-player;
}

int BattleSituation::partner(int spot) const
{
    return slot(player(spot), !(spot/2));
}

const PokeBattle & BattleSituation::poke(int player, int poke) const
{
    return team(player).poke(poke);
}

PokeBattle & BattleSituation::poke(int player, int poke)
{
    return team(player).poke(poke);
}

const PokeBattle &BattleSituation::poke(int slot) const
{
    return team(player(slot)).poke(currentPoke(slot));
}

PokeBattle &BattleSituation::poke(int slot)
{
    return team(player(slot)).poke(currentPoke(slot));
}

int BattleSituation::currentPoke(int player) const
{
    return mycurrentpoke[player];
}

/* The battle loop !! */
void BattleSituation::run()
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
    true_rand2.seed(array, 10);

    forever
    {
        beginTurn();

        endTurn();
    }

}

void BattleSituation::beginTurn()
{
    turn() += 1;
    /* Resetting temporary variables */
    for (int i = 0; i < numberOfSlots(); i++) {
        turnlong[i].clear();
        fieldmoves[i].reset();
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

void BattleSituation::endTurn()
{
    testWin();

    /* Gen3 switches pokemons in before endturn effects */
    if (gen() == 3)
        requestSwitchIns();

    std::vector<int> players = sortedBySpeed();

    callzeffects(Player1, Player1, "EndTurn");
    callzeffects(Player2, Player2, "EndTurn");

    foreach (int player, players) {
        /* Wish */
        callseffects(player,player, "EndTurn2");
    }

    endTurnWeather();

    testWin();

    callbeffects(Player1,Player1,"EndTurn5");

    foreach (int player, players) {
        if (koed(player))
            continue;

        /* Ingrain, aquaring */
        callpeffects(player, player, "EndTurn60");

        /* Speed boost, shed skin, ?Harvest? */
        callaeffects(player,player, "EndTurn62");

//        if (koed(player)) <-- cannot be koed
//            continue;

        /* Lefties, black sludge */
        callieffects(player, player, "EndTurn63");

        if (koed(player)) {
            continue;
        }

        /* Leech Seed, Nightmare. Warning: Leech Seed and rapid spin are linked */
        callpeffects(player, player, "EndTurn64");

        endTurnStatus(player);

        if (koed(player)) {
            continue;
        }

        /* Status orbs */
        callieffects(player, player, "EndTurn66");

        /* Trapping moves damage and Curse. Warning: Rapid spin and trapping moves are linked */
        callpeffects(player, player, "EndTurn68");

        if (koed(player))
            continue;

        /* Bad dreams -- on others */
        callaeffects(player,player, "EndTurn69");

        /* Outrage, Uproar */
        callpeffects(player, player, "EndTurn610");

        /* Disable, taunt, encore, magnet rise, heal block, embargo */
        callpeffects(player, player, "EndTurn611");

        /* Roost */
        callpeffects(player, player, "EndTurn");

        /* Yawn */
        callpeffects(player, player, "EndTurn617");

        /* Sticky barb */
        callieffects(player, player, "EndTurn618");
    }

    testWin();


    foreach (int player, players) {
        /* Doom Desire */
        callseffects(player,player, "EndTurn7");
    }

    testWin();

    foreach (int player, players) {
        /* Perish Song */
        callpeffects(player,player, "EndTurn8");
    }

    testWin();

    callbeffects(Player1,Player1,"EndTurn9");

    requestSwitchIns();

    /* Slow Start */
    foreach (int player, sortedBySpeed()) {
        callaeffects(player,player, "EndTurn20.");
    }
}

void BattleSituation::notifyFail(int p)
{
    notify(All, Failed, p);
}

void BattleSituation::endTurnStatus(int player)
{
    if (koed(player) || hasWorkingAbility(player, Ability::MagicGuard))
        return;

    switch(poke(player).status())
    {
        case Pokemon::Burnt:
            notify(All, StatusMessage, player, qint8(HurtBurn));
            //HeatProof: burn does only 1/16
            inflictDamage(player, poke(player).totalLifePoints()/(8*(1+hasWorkingAbility(player,Ability::Heatproof))), player);
            break;
        case Pokemon::Poisoned:
            //PoisonHeal
            if (hasWorkingAbility(player, Ability::PoisonHeal)) {
                sendAbMessage(45,0,player,0,Pokemon::Poison);
                healLife(player, poke(player).totalLifePoints()/8);
            } else {
                notify(All, StatusMessage, player, qint8(HurtPoison));
                if (poke(player).statusCount() == 0)
                    inflictDamage(player, poke(player).totalLifePoints()/8, player);
                else {
                    inflictDamage(player, poke(player).totalLifePoints() * (15-poke(player).statusCount()) / 16, player);
                    poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
                }
            }
            break;
    }
}

bool BattleSituation::requestChoice(int slot, bool acquire, bool custom)
{
    int player = this->player(slot);

    if (koed(slot) && countBackUp(player) == 0) {
        return false;
    }

    if (turnlong[slot].contains("NoChoice") && !koed(slot)) {
	return false;
    }

    couldMove[slot] = true;
    hasChoice[slot] = true;

    if (!custom)
        options[slot] = createChoice(slot);

    notify(player, OfferChoice, slot, options[slot]);

    startClock(player);

    if (acquire) {
        notify(player, StartChoices, player);
        yield();
    }

    /* Now all the players gonna do is analyzeChoice(int player) */
    return true;
}

void BattleSituation::requestChoices()
{
    for (int i = 0; i < numberOfSlots(); i ++)
        couldMove[i] = false;

    int count = 0;

    for (int i = 0; i < numberOfSlots(); i++) {
        count += requestChoice(i, false);
    }

    if (!allChoicesOkForPlayer(Player1)) {
        notify(Player1, StartChoices, Player1);
    }

    if (!allChoicesOkForPlayer(Player2)) {
        notify(Player2, StartChoices, Player2);
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

void BattleSituation::notifyInfos()
{
    for (int p = 0; p < numberOfSlots(); p++) {
        if (!koed(p)) {
            BattleStats stats = constructStats(p);
            notify(player(p), DynamicStats, p, stats);
            BattleDynamicInfo infos = constructInfo(p);
            notify(All, DynamicInfo, p, infos);
        }
    }
}

bool BattleSituation::koed(int player) const
{
    return currentPoke(player) == -1 || poke(player).ko();
}

bool BattleSituation::wasKoed(int player) const
{
    return currentPoke(player) == -1 || turnlong[player].contains("WasKoed");
}

BattleChoices BattleSituation::createChoice(int slot)
{
    /* First let's see for attacks... */
    if (koed(slot)) {
        return BattleChoices::SwitchOnly(slot);
    }

    BattleChoices ret;
    ret.numSlot = slot;

    /* attacks ok, lets see which ones then */
    callpeffects(slot, slot, "MovesPossible");
    callieffects(slot, slot, "MovesPossible");
    callbeffects(slot, slot,"MovesPossible");

    for (int i = 0; i < 4; i++) {
        if (!isMovePossible(slot,i)) {
	    ret.attackAllowed[i] = false;
	}
    }

    if (!hasWorkingItem(slot, Item::ShedShell)) /* Shed Shell */
    {
        if (linked(slot, "Blocked") || linked(slot, "Trapped")) {
            ret.switchAllowed = false;
	}

        if (pokelong[slot].contains("Rooted")) {
	    ret.switchAllowed = false;
	}

        QList<int> opps = revs(slot);
        foreach(int opp, opps){
            callaeffects(opp, slot, "IsItTrapped");
            if (turnlong[slot].value("Trapped").toBool()) {
                ret.switchAllowed = false;
                break;
            }
        }
    }

    return ret;
}

bool BattleSituation::isMovePossible(int player, int move)
{
    return (poke(player).move(move).PP() > 0 && turnlong[player]["Move" + QString::number(move) + "Blocked"].toBool() == false);
}

void BattleSituation::analyzeChoice(int slot)
{
    int player = this->player(slot);

    stopClock(player, true);
    attackCount() += 1;
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice[slot].attack()) {
        turnlong[slot]["Target"] = choice[slot].target();
        if (!wasKoed(slot) && !turnlong[slot].value("HasMoved").toBool() && !turnlong[slot].value("CantGetToMove").toBool()) {
            if (turnlong[slot].contains("NoChoice"))
                /* Automatic move */
                useAttack(slot, pokelong[slot]["LastSpecialMoveUsed"].toInt(), true);
            else {
                if (options[slot].struggle()) {
                    MoveEffect::setup(Move::Struggle,slot,0,*this);
                    useAttack(slot, Move::Struggle, true);
                } else {
                    useAttack(slot, choice[slot].numSwitch);
                }
            }
	}
    } else {
        if (!koed(slot)) { /* if the pokemon isn't ko, it IS sent back */
            sendBack(slot);
	}
        sendPoke(slot, choice[slot].numSwitch);
    }
    notify(All, BlankMessage, Player1);
}

inline bool comparePair(const std::pair<int,int> & x, const std::pair<int,int> & y) {
    return x.second>y.second;
};

std::vector<int> BattleSituation::sortedBySpeed() {
    std::vector<int> ret;

    std::vector<std::pair<int, int> > speeds;

    for (int i =0; i < numberOfSlots(); i++) {
        if (!koed(i)) {
            speeds.push_back(std::pair<int, int>(i, getStat(i, Speed)));
        }
    }

    if (speeds.size() == 0)
        return ret;

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

    if (battlelong.value("TrickRoomCount").toInt() > 0) {
        std::reverse(ret.begin(),ret.end());
    }

    return ret;
}

void BattleSituation::analyzeChoices()
{
    /* If there's no choice then the effects are already taken care of */
    for (int i = 0; i < numberOfSlots(); i++) {
        if (!turnlong[i].contains("NoChoice") && choice[i].attack() && !options[i].struggle()) {
            MoveEffect::setup(move(i,choice[i].numSwitch), i, i, *this);
        }
    }

    std::map<int, std::vector<int>, std::greater<int> > priorities;
    std::vector<int> switches;

    std::vector<int> playersByOrder = sortedBySpeed();

    foreach(int i, playersByOrder) {
	if (choice[i].poke())
            switches.push_back(i);
        else {
            if (gen() >= 5) {
                callaeffects(i, i, "PriorityChoice");
            }
            priorities[fieldmoves[i].priority].push_back(i);
        }
    }

    foreach(int player, switches) {
	analyzeChoice(player);
        callEntryEffects(player);
    }

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;
    std::vector<int> players;

    for (it = priorities.begin(); it != priorities.end(); ++it) {
	/* There's another priority system: Ability stall, and Item lagging tail */
        std::map<int, std::vector<int>, std::greater<int> > secondPriorities;

        foreach (int player, it->second) {
            callaeffects(player,player, "TurnOrder"); //Stall
            callieffects(player,player, "TurnOrder"); //Lagging tail & ...
            secondPriorities[turnlong[player]["TurnOrder"].toInt()].push_back(player);
	}

        for(std::map<int, std::vector<int> >::iterator it = secondPriorities.begin(); it != secondPriorities.end(); ++it) {
            foreach(int p, it->second) {
                players.push_back(p);
            }
        }
    }

    /* The loop is separated, cuz all TurnOrders must be called at the beggining of the turn,
       cf custap berry */
    if (gen() >= 4) {
        foreach(int p, players) {
            analyzeChoice(p);
            testWin();
        }
    } else { // gen <= 3
        for(unsigned i = 0; i < players.size(); i++) {
            if (!doubles()) {
                if (koed(0) || koed(1))
                    break;
            } else {
                requestSwitchIns();
            }

            analyzeChoice(players[i]);
            testWin();
        }
    }
}

void BattleSituation::notifySub(int player, bool sub)
{
    notify(All, Substitute, player, sub);
}

bool BattleSituation::canCancel(int player)
{
    return blocked() && (couldMove[slot(player,0)] || (doubles() && couldMove[slot(player, 1)]));
}

void BattleSituation::cancel(int player)
{
    notify(player, CancelMove, player);

    if (couldMove[slot(player, 0)]) {
        hasChoice[slot(player, 0)] = true;
    }

    if (doubles() && couldMove[slot(player, 1)]) {
        hasChoice[slot(player, 1)] = true;
    }

    startClock(player,false);
}

bool BattleSituation::validChoice(const BattleChoice &b)
{
    if (!couldMove[b.numSlot] || !hasChoice[b.numSlot] || !b.match(options[b.numSlot])) {
        return false;
    }

    int player = this->player(b.numSlot);

    /* If it's a switch, we check the receiving poke valid, if it's a move, we check the target */
    if (b.poke()) {
        if (isOut(player, b.numSwitch) || poke(player, b.numSwitch).ko()) {
            return false;
        }
        /* Let's also check another switch hasn't been made to the same poke */
        for (int i = 0; i < numberOfSlots() / 2; i++) {
            int p2 = this->player(i);
            if (i != b.numSlot && p2 == player && couldMove[i] && hasChoice[i] == false && choice[i].poke() && choice[i].numSwitch == b.numSwitch) {
                return false;
            }
        }
    } else {
        /* It's an attack, we check the target is valid */
        if (b.numSwitch == -1) {
            if (b.target() < 0 || b.target() >= numberOfSlots() || b.target() == b.numSlot || koed(b.target()))
                return false;
        } else {
            int target = MoveInfo::Target(move(b.numSlot, b.numSwitch), gen());

            if (target == Move::ChosenTarget) {
                if (b.target() < 0 || b.target() >= numberOfSlots() || b.target() == b.numSlot || koed(b.target()))
                    return false;
            } else if (doubles() && target == Move::PartnerOrUser) {
                if (b.target() < 0 || b.target() >= numberOfSlots() || this->player(b.target()) != this->player(b.numSlot) || koed(b.target()))
                    return false;
            }
        }
    }

    return true;
}

bool BattleSituation::isOut(int player, int poke)
{
    return doubles() ? (currentPoke(slot(player, 0)) == poke || currentPoke(slot(player, 1)) == poke ) : (currentPoke(slot(player)) == poke);
}

void BattleSituation::storeChoice(const BattleChoice &b)
{
    choice[b.numSlot] = b;
    hasChoice[b.numSlot] = false;
}

bool BattleSituation::allChoicesOkForPlayer(int player)
{
    return doubles () ? (hasChoice[slot(player, 0)] == false && hasChoice[slot(player, 1)] == false) : (hasChoice[slot(player)] == false);
}


bool BattleSituation::allChoicesSet()
{
    for (int i = 0; i < numberOfSlots(); i++) {
        if (hasChoice[i])
            return false;
    }
    return true;
}

void BattleSituation::battleChoiceReceived(int id, const BattleChoice &b)
{
    int player = spot(id);

    /* Simple guard to avoid multithreading problems */
    if (!blocked()) {
        return;
    }

    if (finished()) {
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

    if (b.numSlot < 0 || b.numSlot >= numberOfSlots()) {
        return;
    }

    if (player != this->player(b.numSlot)) {
        /* W00T! He tried to impersonate the other! Bad Guy! */
        notify(player, BattleChat, opponent(player), QString("Say, are you trying to hack this game? Beware, i'll report you and have you banned!"));
        return;
    }

    /* If the player wants a cancel, we grant it, if possible */
    if (b.cancelled()) {
        if (canCancel(player)) {
            cancel(player);
        }
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
            couldMove[i] = false;
        }
        schedule();
    }
}

void BattleSituation::yield()
{
    blocked() = true;
    ContextCallee::yield();
    testWin();
}

void BattleSituation::schedule()
{
    blocked() = false;
    ContextCallee::schedule();
}

/*****************************************
  Beware of the multi threading problems.
  Don't change the order of the instructions.
  ****************************************/
void BattleSituation::startClock(int player, bool broadCoast)
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
void BattleSituation::stopClock(int player, bool broadCoast)
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
int BattleSituation::timeLeft(int player)
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
void BattleSituation::timerEvent(QTimerEvent *)
{
    if (timeLeft(Player1) <= 0 || timeLeft(Player2) <= 0) {
        schedule(); // the battle is finished, isn't it?
    }
}

/*************************************************
  End of the warning.
  ************************************************/

void BattleSituation::battleChat(int id, const QString &str)
{
    notify(All, BattleChat, spot(id), str);
}

void BattleSituation::spectatingChat(int id, const QString &str)
{
    notify(All, SpectatorChat, id, qint32(id), str);
}


/* Battle functions! Yeah! */

void BattleSituation::sendPoke(int slot, int pok, bool silent)
{
    int player = this->player(slot);

    if (poke(player,pok).num() == Pokemon::Giratina_O && poke(player,pok).item() != Item::GriseousOrb)
        changeForme(player,pok,Pokemon::Giratina);
    
    changeCurrentPoke(slot, pok);

    notify(player, SendOut, slot, silent, ypoke(player, pok));
    notify(AllButPlayer, SendOut, slot, silent, quint8(pok), opoke(player, pok));

    /* reset temporary variables */
    pokelong[slot].clear();
    /* Give new values to what needed */
    fieldpokes[slot].id = poke(slot).num().toPokeRef();
    fieldpokes[slot].weight = PokemonInfo::Weight(poke(slot).num());
    fieldpokes[slot].type1 = PokemonInfo::Type1(poke(slot).num(), gen());
    fieldpokes[slot].type2 = PokemonInfo::Type2(poke(slot).num(), gen());
    fieldpokes[slot].ability = poke(slot).ability();

    if (poke(player, pok).statusCount() > 0) {
        if (poke(player, pok).status() == Pokemon::Poisoned)
            poke(player, pok).statusCount() = 15;
        else if (poke(player, pok).status() != Pokemon::Asleep)
            poke(player, pok).statusCount() = 0;
    }

    for (int i = 0; i < 4; i++) {
        fieldpokes[slot].moves[i] = poke(slot).move(i).num();
    }

    for (int i = 1; i < 6; i++)
        fieldpokes[slot].stats[i] = poke(slot).normalStat(i);

    for (int i = 0; i < 6; i++) {
        fieldpokes[slot].dvs[i] = poke(slot).dvs()[i];
    }

    fieldpokes[slot].level = poke(slot).level();

    /* Increase the "switch count". Switch count is used to see if the pokemon has switched
       (like for an attack like attract), it is imo more effective that other means */
    inc(slotzone[slot]["SwitchCount"], 1);

    if (poke(player,pok).num() == Pokemon::Arceus && ItemInfo::isPlate(poke(player,pok).item())) {
        int type = ItemInfo::PlateType(poke(player,pok).item());
        
        if (type != Type::Normal) {
            changeAForme(slot, type);
        }
    }

    turnlong[slot]["CantGetToMove"] = true;

    ItemEffect::setup(poke(slot).item(),slot,*this);

    calleffects(slot, slot, "UponSwitchIn");
    callseffects(slot, slot, "UponSwitchIn");
    callzeffects(player, slot, "UponSwitchIn");
}

void BattleSituation::callEntryEffects(int player)
{
    if (!koed(player)) {
        acquireAbility(player, poke(player).ability(), true);
        calleffects(player, player, "AfterSwitchIn");
    }
}

void BattleSituation::calleffects(int source, int target, const QString &name)
{

    if (turnlong[source].contains("Effect_" + name)) {
	turnlong[source]["TurnEffectCall"] = true;
        turnlong[source]["TurnEffectCalled"] = name;
        QSet<QString> &effects = *turnlong[source].value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
	    turnlong[source]["EffectBlocked"] = false;
	    turnlong[source]["EffectActivated"] = effect;
            callpeffects(target, source, "BlockTurnEffects");
	    if (turnlong[source]["EffectBlocked"].toBool() == true) {
		continue;
	    }

            MoveMechanics::function f = turnlong[source].value("Effect_" + name + "_" + effect).value<MoveMechanics::function>();

            if (f)
                f(source, target, *this);
	}
	turnlong[source]["TurnEffectCall"] = false;
    }
}

void BattleSituation::callpeffects(int source, int target, const QString &name)
{
    if (pokelong[source].contains("Effect_" + name)) {
        turnlong[source]["PokeEffectCall"] = true;
        QSet<QString> &effects = *pokelong[source].value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MoveMechanics::function f = pokelong[source].value("Effect_" + name + "_" + effect).value<MoveMechanics::function>();

            /* If a pokemons dies from leechseed,its status changes, and so nightmare function would be removed
               but still be in the foreach, causing a crash */
            if (f)
                f(source, target, *this);
        }
	turnlong[source]["PokeEffectCall"] = false;
    }
}

void BattleSituation::callbeffects(int source, int target, const QString &name, bool stopOnFail)
{
    if (battlelong.contains("Effect_" + name)) {
        QSet<QString> &effects = *battlelong.value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
            MoveMechanics::function f = battlelong.value("Effect_" + name + "_" + effect).value<MoveMechanics::function>();

            if (f)
                f(source, target, *this);

            if (stopOnFail && testFail(source))
                return;
	}
    }
}

void BattleSituation::callzeffects(int source, int target, const QString &name)
{
    if (teamzone[source].contains("Effect_" + name)) {
        QSet<QString> &effects = *teamzone[source].value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
            MoveMechanics::function f = teamzone[source].value("Effect_" + name + "_" + effect).value<MoveMechanics::function>();

            if (f)
                f(source, target, *this);
	}
    }
}

void BattleSituation::callseffects(int source, int target, const QString &name)
{
    if (slotzone[source].contains("Effect_" + name)) {
        QSet<QString> &effects = *slotzone[source].value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MoveMechanics::function f = slotzone[source].value("Effect_" + name + "_" + effect).value<MoveMechanics::function>();

            if (f)
                f(source, target, *this);
        }
    }
}

void BattleSituation::callieffects(int source, int target, const QString &name)
{
    //Klutz
    if (hasWorkingItem(source, poke(source).item()))
	ItemEffect::activate(name, poke(source).item(), source, target, *this);
}

void BattleSituation::callaeffects(int source, int target, const QString &name)
{
    if (hasWorkingAbility(source, ability(source)))
        AbilityEffect::activate(name, ability(source), source, target, *this);
}

void BattleSituation::sendBack(int player, bool silent)
{
    if (!silent)
        notify(All, SendBack, player);

    /* Just calling pursuit directly here, forgive me for this */
    if (!turnlong[player].value("BatonPassed").toBool()) {
        QList<int> opps = revs(player);
        foreach(int opp, opps) {
            if (fieldmoves[opp].attack == Move::Pursuit && !turnlong[opp]["HasMoved"].toBool()) {
                fieldmoves[opp].power = fieldmoves[opp].power * 2;
                choice[opp].targetPoke = player;
                analyzeChoice(opp);

                if (koed(player)) {
                    Mechanics::removeFunction(turnlong[player],"UponSwitchIn","BatonPass");
                    break;
                }
            }
        }
    }

    if (!koed(player)) {
        /* Natural cure bypasses gastro acid (tested in 4th gen, but not role play/skill swap),
           so we don't check if the ability is working, and just make a test
           directly. */
        if (ability(player) == Ability::NaturalCure) {
            healStatus(player, poke(player).status());
        } else if (ability(player) == Ability::Regeneration) {
            healLife(player, poke(player).totalLifePoints() / 3);
        }
	changeCurrentPoke(player, -1);
    }
}

bool BattleSituation::testAccuracy(int player, int target, bool silent)
{
    int acc = fieldmoves[player].accuracy;
    int tarChoice = fieldmoves[player].targets;
    bool muliTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

    turnlong[target].remove("EvadeAttack");
    callpeffects(target, player, "TestEvasion"); /*dig bounce ..., still calling it there cuz x2 attacks
            like EQ on dig need their boost even if lock on */

    if (pokelong[player].contains("LockedOn") && pokelong[player].value("LockedOnEnd").toInt() >= turn()
            && pokelong[player].value("LockedOn") == target &&
                    pokelong[player].value("LockedOnCount").toInt() == slotzone[target]["SwitchCount"].toInt()) {
        return true;
    }

    if (pokelong[player].value("BerryLock").toBool()) {
        pokelong[player].remove("BerryLock");
        return true;
    }

    //No Guard
    if ((hasWorkingAbility(player, Ability::NoGuard) || hasWorkingAbility(target, Ability::NoGuard))) {
        return true;
    }

    if (turnlong[target].contains("EvadeAttack")) {
        if (!silent) {
            if (muliTar) {
                notify(All, Avoid, target);
            } else {
                notify(All, Miss, player);
            }
        }
        return false;
    }

    if (acc == 0 || acc == 101) {
	return true;
    }

    //OHKO
    int move  = turnlong[player]["MoveChosen"].toInt();

    if (MoveInfo::isOHKO(move, gen())) {
        bool ret = (true_rand() % 100) < unsigned (30 + poke(player).level() - poke(target).level());
        if (!ret && !silent) {
            if (muliTar) {
                notify(All, Avoid, target);
            } else {
                notify(All, Miss, player);
            }
        }
        return ret;
    }

    turnlong[player].remove("Stat6ItemModifier");
    turnlong[player].remove("Stat6AbilityModifier");
    turnlong[target].remove("Stat7ItemModifier");
    turnlong[target].remove("Stat7AbilityModifier");
    callieffects(player,target,"StatModifier");
    callaeffects(player,target,"StatModifier");
    callieffects(target,player,"StatModifier");
    callaeffects(target,player,"StatModifier");
    if (doubles()) {
        int partner = this->partner(player);

        if (!koed(partner)) {
            callaeffects(partner, player, "PartnerStatModifier");
        }
    }
    /* no *=: remember, we're working with fractions & int, changing the order might screw up by 1 % or so
	due to the ever rounding down to make an int */
    acc = acc * getStatBoost(player, Accuracy) * getStatBoost(target, Evasion)
            * (20+turnlong[player].value("Stat6ItemModifier").toInt())/20
            * (20-turnlong[target].value("Stat7ItemModifier").toInt())/20
            * (20+turnlong[player].value("Stat6AbilityModifier").toInt())/20
            * (20+turnlong[player].value("Stat6PartnerAbilityModifier").toInt())/20
            * (20-turnlong[target].value("Stat7AbilityModifier").toInt())/20;

    if (true_rand() % 100 < unsigned(acc)) {
	return true;
    } else {
        if (!silent) {
            if (muliTar) {
                notify(All, Avoid, target);
            } else {
                notify(All, Miss, player);
            }
        }
	calleffects(player,target,"MissAttack");
	return false;
    }
}

void BattleSituation::testCritical(int player, int target)
{
    /* Shell armor, Battle Armor */
    if (hasWorkingAbility(target, 8) || hasWorkingAbility(target, 85) || teamzone[this->player(target)].value("LuckyChantCount").toInt() > 0) {
	return;
    }

    int randnum = true_rand() % 48;
    int minch;
    int craise = fieldmoves[player].critRaise;

    if (hasWorkingAbility(player, Ability::SuperLuck)) { /* Super Luck */
	craise += 1;
    }

    switch(craise) {
	case 0: minch = 3; break;
	case 1: minch = 6; break;
	case 2: minch = 12; break;
	case 3: minch = 16; break;
        case 4: case 5: minch = 24; break;
        case 6: default: minch = 48;
    }

    bool critical = randnum<minch;

    turnlong[player]["CriticalHit"] = critical;

    if (critical) {
	notify(All, CriticalHit, player);
    }
}

bool BattleSituation::testStatus(int player)
{
    if (turnlong[player].value("HasPassedStatus") == true) {
        return true;
    }

    if (poke(player).status() == Pokemon::Asleep) {
        if (poke(player).statusCount() > 0) {
            //Early bird
            poke(player).statusCount() -= 1 + hasWorkingAbility(player, Ability::EarlyBird);
            notify(All, StatusMessage, player, qint8(FeelAsleep));
            if (!turnlong[player].value("SleepingMove").toBool())
                return false;
        } else {
            healStatus(player, Pokemon::Asleep);
            notify(All, StatusMessage, player, qint8(FreeAsleep));
        }
    }
    if (poke(player).status() == Pokemon::Frozen)
    {
        if (true_rand() % 255 > 51 && (!fieldmoves[player].flags & Move::UnthawingFlag) )
        {
            notify(All, StatusMessage, player, qint8(PrevFrozen));
            return false;
        }
        healStatus(player, Pokemon::Frozen);
        notify(All, StatusMessage, player, qint8(FreeFrozen));
    }

    if (turnlong[player]["Flinched"].toBool()) {
        notify(All, Flinch, player);
        //SteadFast
        if (hasWorkingAbility(player, Ability::Steadfast)) {
            sendAbMessage(60,0,player);
            inflictStatMod(player, Speed, 1, player);
        }
        return false;
    }
    if (isConfused(player)) {
        if (pokelong[player]["ConfusedCount"].toInt() > 0) {
            inc(pokelong[player]["ConfusedCount"], -1);

            notify(All, StatusMessage, player, qint8(FeelConfusion));

            if (true_rand() % 2 == 0) {
                inflictConfusedDamage(player);
                return false;
            }
        } else {
            healConfused(player);
            notify(All, StatusMessage, player, qint8(FreeConfusion));
        }
    }

    if (poke(player).status() == Pokemon::Paralysed) {
        //MagicGuard
        if (!hasWorkingAbility(player, Ability::MagicGuard) && true_rand() % 4 == 0) {
            notify(All, StatusMessage, player, qint8(PrevParalysed));
            return false;
        }
    }

    return true;
}

void BattleSituation::inflictConfusedDamage(int player)
{
    notify(All, StatusMessage, player, qint8(HurtConfusion));

    fieldmoves[player].type = Pokemon::Curse;
    fieldmoves[player].power = 40;
    turnlong[player]["TypeMod"] = 4;
    turnlong[player]["Stab"] = 2;
    fieldmoves[player].category = Move::Physical;
    int damage = calculateDamage(player, player);
    inflictDamage(player, damage, player, true);
}

void BattleSituation::testFlinch(int player, int target)
{
    //Inner focus, shield dust
    if (hasWorkingAbility(target, Ability::ShieldDust)) {
        return;
    }

    int rate = fieldmoves[player].flinchRate;

    if (hasWorkingAbility(target, Ability::InnerFocus)) {
        if (rate == 100) {
            sendAbMessage(12,0,player);
        }
        return;
    }


    int randnum = true_rand() % 100;
    /* Serene Grace */
    if (hasWorkingAbility(player, Ability::SereneGrace)) {
        randnum /= 2;
    }

    if (randnum % 100 < rate) {
	turnlong[target]["Flinched"] = true;
    }

    if (hasWorkingItem(player, Item::KingsRock)) /* King's rock */
    {
        if (true_rand() % 100 < 10) {
	    turnlong[target]["Flinched"] = true;
	}
    }
}

bool BattleSituation::testFail(int player)
{
    if (turnlong[player]["Failed"].toBool() == true) {
        if (turnlong[player]["FailingMessage"].toBool()) {
	    notify(All, Failed, player);
	}
	return true;
    }
    return false;
}

bool BattleSituation::attacking()
{
    return attacker() != -1;
}

void BattleSituation::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    int oldAttacker = attacker();
    int oldAttacked = attacked();

    attacker() = player;

    int attack;

    if (specialOccurence) {
	attack = move;
    } else {
        //Quick claw, special case
        if (gen() >= 4 && turnlong[player].value("QuickClawed").toBool()) {
            //The message only shows up if it's not the last pokemon to move
            for (int i = 0; i < numberOfSlots(); i++) {
                if (!turnlong[i].value("HasMoved").toBool() && !turnlong[i].value("CantGetToMove").toBool() && !koed(i) && i != player) {
                    sendItemMessage(17, player);
                    break;
                }
            }
        }
	attack = this->move(player,move);
        pokelong[player]["MoveSlot"] = move;
    }

    turnlong[player]["HasMoved"] = true;

    calleffects(player,player,"EvenWhenCantMove");

    if (!testStatus(player)) {
        goto trueend;
    }

    //Just for truant
    callaeffects(player, player, "DetermineAttackPossible");
    if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
        goto trueend;
    }

    callpeffects(player, player, "DetermineAttackPossible");
    if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
        goto trueend;
    }

    turnlong[player]["HasPassedStatus"] = true;

    turnlong[player]["MoveChosen"] = attack;

    callbeffects(player,player,"MovePossible");
    if (turnlong[player]["ImpossibleToMove"].toBool()) {
        goto trueend;
    }

    callpeffects(player, player, "MovePossible");
    if (turnlong[player]["ImpossibleToMove"].toBool()) {
        goto trueend;
    }

    if (!specialOccurence) {
        if (!isMovePossible(player, move)) {
            goto trueend;
        }

        pokelong[player][QString("Move%1Used").arg(move)] = true;

        pokelong[player]["LastMoveUsed"] = attack;
        pokelong[player]["LastMoveUsedTurn"] = turn();
    } else if (attack != 0 && pokelong[player].value("LastMoveUsedTurn").toInt() != turn() && attack != Move::Struggle) {
	/* Recharge moves have their attack as 0 on the recharge turn : Blast Burn , ...
	  So that's why attack is tested against 0. */
	/* Those are needed for when a choiced move is used, or torment is used, and for example
		the foe used Assit + Fly or simply fly. */
        if (attack != Move::Struggle) {
            pokelong[player]["LastMoveUsed"] = attack;
        }
	pokelong[player]["LastMoveUsedTurn"] = turn();
    }

    //For metronome calling fly / sky attack / ...
    pokelong[player]["LastSpecialMoveUsed"] = attack;

    calleffects(player, player, "MoveSettings");

    if (tellPlayers && !turnlong[player].contains("TellPlayers")) {
        notify(All, UseAttack, player, qint16(attack));
    }

    /* Lightning Rod & Storm Drain */
    foreach(int poke, sortedBySpeed()) {
        if (poke != player) {
            callaeffects(poke, player, "GeneralTargetChange");
        }
    }

    callbeffects(player,player, "GeneralTargetChange");

    targetList.clear();
    switch(Move::Target(fieldmoves[player].targets)) {
    case Move::Field: case Move::TeamParty: case Move::OpposingTeam:
    case Move::TeamSide: case Move::IndeterminateTarget: case Move::Partner:
	case Move::User: targetList.push_back(player); break;
        case Move::Opponents: 
            targetList = sortedBySpeed();
            for (unsigned i = 0; i < targetList.size(); i++) {
                if (this->player(targetList[i]) == this->player(player) ) {
                    targetList.erase(targetList.begin()+i, targetList.begin() + i + 1);
                    i--;
                }
            }
            break;
        case Move::All:
            targetList = sortedBySpeed();
            break;
        case Move::AllButSelf:
            targetList = sortedBySpeed();
            for (unsigned i = 0; i < targetList.size(); i++) {
                if (targetList[i] == player) {
                    targetList.erase(targetList.begin()+i, targetList.begin() + i + 1);
                    i--;
                }
            }
            break;
        case Move::ChosenTarget: case Move::MeFirstTarget: {
                if (doubles()) {
                    int target = turnlong[player]["Target"].toInt();
                    if (!koed(target) && target != player) {
                        targetList.push_back(target);
                        break;
                    }
                }
            }
            /* There is no "break" here and it is normal. Do not change the order */
        case Move::RandomTarget :
            {
                if (!turnlong[player].contains("TargetChanged")) {
                    int randomOpponent = this->randomOpponent(player);
                    if (randomOpponent != - 1)
                        targetList.push_back(randomOpponent);
                } else {
                    targetList.push_back(turnlong[player]["Target"].toInt());
                }
                break;
            }
        case Move::PartnerOrUser:
            if (!doubles()) {
                targetList.push_back(player);
            } else {
                /* Acupressure can be called with sleep talk, so the target needs to be checked */
                int target = turnlong[player]["Target"].toInt();
                if (!koed(target) && this->player(target) == this->player(player)) {
                    targetList.push_back(target);
                } else {
                    targetList.push_back(randomValidOpponent(randomValidOpponent(player)));
                }
            }
            break;
    }

    if (!specialOccurence && !turnlong[player].contains("NoChoice")) {
        //Pressure
        int ppsum = 1;

        foreach(int poke, targetList) {
            if (poke != player && hasWorkingAbility(poke, Ability::Pressure)) {
                ppsum += 1;
            }
        }

        losePP(player, move, ppsum);
    }

    if (targetList.size() == 0) {
	notify(All, NoOpponent, player);
        goto end;
    }

    callaeffects(player,player, "BeforeTargetList");
    callieffects(player, player, "BeforeTargetList");
    calleffects(player, player, "BeforeTargetList");

    foreach(int target, targetList) {
        attacked() = target;
        if (!specialOccurence && (fieldmoves[player].flags & Move::MemorableFlag) ) {
	    pokelong[target]["MirrorMoveMemory"] = attack;
	}

	turnlong[player]["Failed"] = false;
	turnlong[player]["FailingMessage"] = true;
	if (target != -1 && koed(target)) {
            calleffects(player,target,"AttackSomehowFailed");
	    continue;
	}
	if (target != player && !testAccuracy(player, target)) {
            calleffects(player,target,"AttackSomehowFailed");
	    continue;
	}
        //fixme: try to get protect to work on a calleffects(target, player), and wide guard/priority guard on callteffects(this.player(target), player)
        /* Protect, ... */
        callbeffects(player, target, "DetermineGeneralAttackFailure", true);
        if (testFail(player)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }
        /* Coats */
        callbeffects(player, target, "DetermineGeneralAttackFailure2", true);
        if (testFail(player)) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }

        if (target != player) {
            callaeffects(target,player,"OpponentBlock");
        }
        if (turnlong[target].contains(QString("Block%1").arg(player))) {
            calleffects(player,target,"AttackSomehowFailed");
            continue;
        }

        if (fieldmoves[player].power > 0)
        {
            calculateTypeModStab();

            calleffects(player, target, "BeforeCalculatingDamage");
            /* For charge */
            callpeffects(player, target, "BeforeCalculatingDamage");

            int typemod = turnlong[player]["TypeMod"].toInt();
            if (typemod == 0) {
		/* If it's ineffective we just say it */
		notify(All, Effective, target, quint8(typemod));
		calleffects(player,target,"AttackSomehowFailed");
		continue;
	    }

	    callpeffects(player, target, "DetermineAttackFailure");
	    if (testFail(player)) {
		calleffects(player,target,"AttackSomehowFailed");
		continue;
	    }
	    calleffects(player, target, "DetermineAttackFailure");
	    if (testFail(player)){
		calleffects(player,target,"AttackSomehowFailed");
		continue;
	    }

	    calleffects(player, target, "BeforeHitting");

            int num = repeatNum(player);
	    bool hit = num > 1;

            int i;
            for (i = 0; i < num && !koed(target); i++) {
                turnlong[target]["HadSubstitute"] = false;
		bool sub = hasSubstitute(target);
                turnlong[target]["HadSubstitute"] = sub;

                if (fieldmoves[player].power > 1 && i == 0)
                    notify(All, Effective, target, quint8(typemod));

                if (fieldmoves[player].power > 1) {
		    testCritical(player, target);
		    int damage = calculateDamage(player, target);
		    inflictDamage(target, damage, player, true);
		} else {
		    calleffects(player, target, "CustomAttackingDamage");
		}

		calleffects(player, target, "UponAttackSuccessful");
                healDamage(player, target);

                if (fieldmoves[player].flags & Move::ContactFlag) {
		    if (!sub)
			callieffects(target, player, "UponPhysicalAssault");
                    callaeffects(target,player,"UponPhysicalAssault");
		}
		/* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
		applyMoveStatMods(player, target);

                /* For berries that activate after taking damage */
                callieffects(target, player, "TestPinch");

		if (!sub && !koed(target))
		    testFlinch(player, target);
            }

            if (hit) {
                notifyHits(i);
            }

            if (!koed(player)) {
                calleffects(player, target, "AfterAttackSuccessful");
            }

            turnlong[target]["HadSubstitute"] = false;
        } else {
            callpeffects(player, target, "DetermineAttackFailure");
	    if (testFail(player))
		continue;
	    calleffects(player, target, "DetermineAttackFailure");
	    if (testFail(player))
		continue;
            int type = fieldmoves[player].type; /* move type */
            if ( target != player &&
                 ((type == Type::Fire && hasType(target, Type::Fire)) ||
                  (type == Type::Poison && (hasType(target, Type::Poison))) ||
                  ((attack == Move::ThunderWave || attack == Move::Toxic || attack == Move::Smog || attack == Move::Will_O_Wisp)
                                       && TypeInfo::Eff(type, getType(target, 1)) * TypeInfo::Eff(type, getType(target, 2)) == 0
                                       && !pokelong[target].value(QString("%1Sleuthed").arg(type)).toBool()))){
                notify(All, Failed, player);
                continue;
            }

	    calleffects(player, target, "BeforeHitting");

	    applyMoveStatMods(player, target);
	    calleffects(player, target, "UponAttackSuccessful");
            healDamage(player, target);

	    if (!koed(player))
		calleffects(player, target, "AfterAttackSuccessful");
	}
        if (fieldmoves[player].type == Type::Fire && poke(target).status() == Pokemon::Frozen) {
	    notify(All, StatusMessage, target, qint8(FreeFrozen));
	    healStatus(target, Pokemon::Frozen);
	}
	pokelong[target]["LastAttackToHit"] = attack;
    }

    if (!specialOccurence && attack != Move::Struggle) {
            battlelong["LastMoveSuccesfullyUsed"] = attack;
            pokelong[player]["LastMoveSuccessfullyUsed"] = attack;
            pokelong[player]["LastMoveSuccessfullyUsedTurn"] = turn();
            battlelong["LastMoveSuccessfullyUsed"] = attack;
    }

    callieffects(player, player, "AfterTargetList");
    end:
    callieffects(player, player, "BeforeEnding");
    calleffects(player, player, "BeforeEnding");
    trueend:
    pokelong[player]["HasMovedOnce"] = true;
    /* For U-TURN, so that none of the variables of the switchin are afflicted, it's put at the utmost end */
    calleffects(player, player, "AfterAttackFinished");

    attacker() = oldAttacker;
    attacked() = oldAttacked;
}

void BattleSituation::notifyHits(int number)
{
    notify(All, Hit, Player1, quint8(number));
}

bool BattleSituation::hasMove(int player, int move) {
    for (int i = 0; i < 4; i++) {
	if (this->move(player, i) == move) {
	    return true;
	}
    }
    return false;
}

void BattleSituation::calculateTypeModStab()
{
    int player = attacker();
    int target = attacked();

    int type = fieldmoves[player].type; /* move type */
    int typeadv[] = {getType(target, 1), getType(target, 2)};
    int typepok[] = {getType(player, 1), getType(player, 2)};
    int typeffs[] = {TypeInfo::Eff(type, typeadv[0]),TypeInfo::Eff(type, typeadv[1])};
    int typemod = 1;

    for (int i = 0; i < 2; i++) {
        if (typeffs[i] == 0) {
            if (type == Type::Ground && !isFlying(target)) {
                typemod *= 2;
                continue;
            }
            if (pokelong[target].value(QString::number(typeadv[i])+"Sleuthed").toBool()) {
                typemod *= 2;
                continue;
            }
            /* Scrappy */
            if (hasType(target, Pokemon::Ghost) && hasWorkingAbility(player,Ability::Scrappy)) {
                typemod *= 2;
                continue;
            }
        }
        typemod *= typeffs[i];
    }

    if (type == Type::Ground && isFlying(target)) {
        typemod = 0;
    }

    int stab = 2 + (type==typepok[0] || type==typepok[1]);

    turnlong[player]["Stab"] = stab;
    turnlong[player]["TypeMod"] = typemod; /* is attack effective? or not? etc. */
}

bool BattleSituation::hasWorkingAbility(int player, int ab)
{
    if (attacking()) {
        // Mold Breaker
        if (player == attacked() && player != attacker() &&
            (hasWorkingAbility(attacker(), ability(attacker()))
            &&( ability(attacker()) == Ability::MoldBreaker || ability(attacker()) == Ability::TeraVoltage ||  ability(attacker()) == Ability::TurboBlaze))) {
            return false;
        }
    }
    return pokelong[player].value("AbilityNullified").toBool() ? false : fieldpokes[player].ability == ab;
}

void BattleSituation::acquireAbility(int play, int ab, bool firstTime) {
    fieldpokes[play].ability = ab;
    AbilityEffect::setup(ability(play),play,*this, firstTime);
}

int BattleSituation::ability(int player) {
    return fieldpokes[player].ability;
}

int BattleSituation::weight(int player) {
    if (hasWorkingAbility(player, Ability::HeavyMetal)) {
        return fieldpokes[player].weight * 2;
    } else if (hasWorkingAbility(player, Ability::LightMetal)) {
        return fieldpokes[player].weight / 2;
    } else {
        return fieldpokes[player].weight;
    }
}

Pokemon::uniqueId BattleSituation::pokenum(int player) {
    return fieldpokes[player].id;
}

bool BattleSituation::hasWorkingItem(int player, int it)
{
    //Klutz
    return poke(player).item() == it && !pokelong[player].value("Embargoed").toBool() && !hasWorkingAbility(player, Ability::Klutz)
            && battlelong.value("MagicRoomCount").toInt() == 0
            && !(ItemInfo::isBerry(poke(player).item()) && opponentsHaveWorkingAbility(player, Ability::Anxiety));
}

bool BattleSituation::opponentsHaveWorkingAbility(int play, int ability)
{
    QList<int> opponents = revs(play);

    foreach(int opponent, opponents) {
        if (hasWorkingAbility(opponent, ability))
            return true;
    }
    return false;
}

int BattleSituation::move(int player, int slot)
{
    return fieldpokes[player].moves[slot];
}

void BattleSituation::inflictRecoil(int source, int target)
{
    //Rockhead, MagicGuard
    if (koed(source) || hasWorkingAbility(source,Ability::RockHead) || hasWorkingAbility(source,Ability::MagicGuard))
        return;

    int recoil = fieldmoves[source].recoil;

    if (recoil == 0)
        return;

    notify(All, Recoil, recoil < 0 ? source : target, bool(recoil < 0));

    int damage = std::abs(recoil) * turnlong[source].value("DamageInflicted").toInt() / 100;

    if (recoil < 0) {
        inflictDamage(source, damage, source, false);
    } else  {
        if (hasWorkingItem(source, Item::BigRoot)) /* Big root */ {
            damage = damage * 13 / 10;
        }

        if (hasWorkingAbility(target, Ability::LiquidOoze)) {
            sendMoveMessage(1,2,source,Pokemon::Poison,target);
            inflictDamage(source,damage,source,false);
        } else {
            healLife(source, damage);
        }
    }
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    applyingMoveStatMods = true;
    bool sub = hasSubstitute(target);

    BasicMoveInfo &fm = fieldmoves[player];
    int cl= fm.classification;

    /* First we check if there's even an effect... */
    if (cl != Move::StatAndStatusMove && cl != Move::StatChangingMove && cl != Move::StatusInducingMove
        && cl != Move::OffensiveSelfStatChangingMove && cl != Move::OffensiveStatusInducingMove
        && cl != Move::OffensiveStatChangingMove)
    {
        applyingMoveStatMods = false;
	return;
    }

    if ( (cl == Move::OffensiveStatChangingMove || cl == Move::OffensiveStatusInducingMove)&& hasWorkingAbility(target, Ability::ShieldDust)) {
//        sendAbMessage(24,0,targeted,0,Pokemon::Bug);
        applyingMoveStatMods = false;
        return;
    }

    bool statChange = false;
    bool negativeStatChange = false;

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

        char increase = char (fm.boostOfStat >> (i*8));
        int rate = char (fm.rateOfStat >> (i*8));

        if (increase < 0 && target != player && sub) {
            if (rate == 0 && cl != Move::OffensiveStatusInducingMove) {
                sendMoveMessage(128, 2, player,0,target, fieldmoves[player].attack);
            }
            applyingMoveStatMods = false;
            return;
        }

        /* Then we check if the effect hits */
        int randnum = true_rand() % 100;
        /* Serene Grace */
        if (hasWorkingAbility(player,Ability::SereneGrace)) {
            rate *= 2;
        }

        if (rate != 0 && randnum > rate) {
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
            if (!inflictStatMod(target, stat, increase, player))
                return;
            negativeStatChange = true;
        }

        statChange = true;
    }

    if (statChange == true) {
        callieffects(target, player, "AfterStatChange");
        if (target != player && negativeStatChange && gen() >= 5) {
            callaeffects(target, player, "AfterNegativeStatChange");
        }
    }

    /* Now Status */

    if (cl != Move::StatAndStatusMove && cl != Move::StatusInducingMove && cl != Move::OffensiveStatusInducingMove) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.statusKind > Pokemon::Confused) {
        applyingMoveStatMods = false;
        return; // Other status effects than status and confusion are, on PO, dealt as special moves. Should probably be changed
    }

    int rate = fm.rate;

    if (target != player && sub) {
        if (rate == 0 && cl != Move::OffensiveStatChangingMove) {
            sendMoveMessage(128, 2, player,0,target, fieldmoves[player].attack);
        }
        applyingMoveStatMods = false;
        return;
    }

    /* Then we check if the effect hits */
    int randnum = true_rand() % 100;
    /* Serene Grace */
    if (hasWorkingAbility(player,Ability::SereneGrace)) {
        rate *= 2;
    }

    if (rate != 0 && randnum > rate) {
        applyingMoveStatMods = false;
        return;
    }

    if (fm.status == Pokemon::Confused)
        inflictConfused(target, player, true);
    else
        inflictStatus(target, fm.status, player, fm.minTurns, fm.maxTurns);

    applyingMoveStatMods = false;
}

void BattleSituation::healConfused(int player)
{
    pokelong[player]["Confused"] = false;
}

bool BattleSituation::isConfused(int player)
{
    return pokelong[player].value("Confused").toBool();
}

void BattleSituation::healStatus(int player, int status)
{
    if (poke(player).status() == status || status == 0) {
	changeStatus(player, Pokemon::Fine);
    }
}

bool BattleSituation::canGetStatus(int player, int status) {
    switch (status) {
    case Pokemon::Paralysed: return !hasWorkingAbility(player, Ability::Limber);
    case Pokemon::Asleep: return !hasWorkingAbility(player, Ability::Insomnia) && !hasWorkingAbility(player, Ability::VitalSpirit);
    case Pokemon::Burnt: return !hasType(player, Pokemon::Fire) && !hasWorkingAbility(player, Ability::WaterVeil);
    case Pokemon::Poisoned: return !hasType(player, Pokemon::Poison) && !hasType(player, Pokemon::Steel) && !hasWorkingAbility(player, Ability::Immunity);
    case Pokemon::Frozen: return !isWeatherWorking(Sunny) && !hasType(player, Pokemon::Ice) && !hasWorkingAbility(player, Ability::MagmaArmor);
    default:
        return false;
    }
}

bool BattleSituation::inflictStatMod(int player, int stat, int mod, int attacker, bool tell, bool *negative)
{
    bool pos = (mod > 0) ^ hasWorkingAbility(player, Ability::Perversity);
    if (negative)
        *negative = !pos;

    if (pos)
        return gainStatMod(player, stat, std::abs(mod), attacker, tell);
    else
        return loseStatMod(player, stat, std::abs(mod), attacker, tell);
}

bool BattleSituation::gainStatMod(int player, int stat, int bonus, int , bool tell)
{
    QString path = QString("Boost%1").arg(stat);
    int boost = pokelong[player].value(path).toInt();
    if (boost < 6) {
        if (tell)
            notify(All, StatChange, player, qint8(stat), qint8(bonus));
        changeStatMod(player, stat, std::min(boost+bonus, 6));
    }

    return true;
}

bool BattleSituation::loseStatMod(int player, int stat, int malus, int attacker, bool tell)
{
    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnlong[player].remove(q);
        turnlong[player]["StatModType"] = QString("Stat");
        turnlong[player]["StatModded"] = stat;
        turnlong[player]["StatModification"] = -malus;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnlong[player].contains(q)) {
            return false;
        }

        if(teamzone[this->player(player)].value("MistCount").toInt() > 0) {
            if (canSendPreventMessage(player, attacker))
                sendMoveMessage(86, 2, player,Pokemon::Ice,player, fieldmoves[attacker].attack);
            return false;
        }
    }

    QString path = QString("Boost%1").arg(stat);
    int boost = pokelong[player].value(path).toInt();
    if (boost > -6) {
        if (tell)
            notify(All, StatChange, player, qint8(stat), qint8(-malus));
        changeStatMod(player, stat, std::max(boost-malus, -6));

        if (!applyingMoveStatMods) {
            callieffects(player, attacker, "AfterStatChange");
            if (player != attacker) {
                callaeffects(player, attacker, "AfterNegativeStatChange");
            }
        }
    } else {
        //fixme: can't decrease message
    }

    return true;
}


void BattleSituation::inflictStatus(int player, int status, int attacker, int minTurns, int maxTurns)
{
    //fixme: mist + intimidate
    if (poke(player).status() != Pokemon::Fine) {
        if (this->attacker() == attacker &&
            (fieldmoves[attacker].category == Move::StatusInducingMove ||
             fieldmoves[attacker].category == Move::StatAndStatusMove) && canSendPreventSMessage(player, attacker)) {
            if (poke(player).status() == status)
                notify(All, AlreadyStatusMessage, player, quint8(poke(player).status()));
            else
                notify(All, Failed, player);
        }
        return;
    }
    if (status == Pokemon::Asleep && isThereUproar()) {
        sendMoveMessage(141,4,player);
        return;
    }
    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnlong[player].remove(q);
        turnlong[player]["StatModType"] = QString("Status");
        turnlong[player]["StatusInflicted"] = status;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnlong[player].contains(q)) {
            return;
        }

        if(teamzone[this->player(player)].value("SafeGuardCount").toInt() > 0) {
            sendMoveMessage(109, 2, player,Pokemon::Psychic, player, fieldmoves[player].attack);
            return;
        }
    }

    if (!canGetStatus(player, status))
        return;

    if (status == Pokemon::Asleep)
    {
        if (sleepClause() && currentForcedSleepPoke[this->player(player)] != -1) {
            notifyClause(ChallengeInfo::SleepClause, true);
            return;
        } else {
            currentForcedSleepPoke[this->player(player)] = currentPoke(player);
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

    changeStatus(player, status, true, minTurns == 0 ? 0 : minTurns-1 + true_rand() % (maxTurns - minTurns + 1));
    if (status == Pokemon::Frozen && poke(player).num() == Pokemon::Shaymin_S) {
        changeForme(player, currentPoke(player), Pokemon::Shaymin_S);
    }
    if (attacker != player && status != Pokemon::Asleep && status != Pokemon::Frozen && poke(attacker).status() == Pokemon::Fine && canGetStatus(attacker,status)
        && hasWorkingAbility(player, Ability::Synchronize)) //Synchronize
    {
        sendAbMessage(61,0,player,attacker);
        inflictStatus(attacker, status, player);
    }
}

void BattleSituation::inflictConfused(int player, int attacker, bool tell)
{
    //fixme: insomnia/owntempo/...
    if (pokelong[player].value("Confused").toBool()) {
        if (this->attacker() == attacker && canSendPreventSMessage(player, attacker))
            notify(All, AlreadyStatusMessage, player, quint8(Pokemon::Confused));
    }

    if (attacker != player) {
        QString q = QString("StatModFrom%1Prevented").arg(attacker);
        turnlong[player].remove(q);
        turnlong[player]["StatModType"] = QString("Status");
        turnlong[player]["StatusInflicted"] = Pokemon::Confused;
        callaeffects(player, attacker, "PreventStatChange");
        if (turnlong[player].contains(q)) {
            return;
        }

        if(teamzone[this->player(player)].value("SafeGuardCount").toInt() > 0) {
            sendMoveMessage(109, 2, player,Pokemon::Psychic, player, fieldmoves[attacker].attack);
            return;
        }
    }

    pokelong[player]["Confused"] = true;
    pokelong[player]["ConfusedCount"] = (true_rand() % 4) + 1;

    if (tell)
        notify(All, StatusChange, player, qint8(Pokemon::Confused));

    callieffects(player, player,"AfterStatusChange");
}


void BattleSituation::callForth(int weather, int turns)
{
    weatherCount = turns;
    if (weather != this->weather) {
        this->weather = weather;
        foreach (int i, sortedBySpeed()) {
            callaeffects(i,i,"WeatherChange");
        }
    }
}

void BattleSituation::endTurnWeather()
{
    int weather = this->weather;

    if (weather == NormalWeather) {
	return;
    }

    int count = weatherCount - 1;
    if (count == 0) {
	notify(All, WeatherMessage, Player1, qint8(EndWeather), qint8(weather));
	callForth(NormalWeather, -1);
    } else {
	notify(All, WeatherMessage, Player1, qint8(ContinueWeather), qint8(weather));
	/* And now the weather damage! */
        if (isWeatherWorking(weather)) {
	    QSet<int> immuneTypes;
	    if (weather == Hail) {
		immuneTypes << Pokemon::Ice;
	    } else {
		immuneTypes << Pokemon::Rock << Pokemon::Ground << Pokemon::Steel;
	    }
            std::vector<int> players = sortedBySpeed();
            foreach (int i, players) {
                callaeffects(i,i,"WeatherSpecial");
                if (!turnlong[i].contains("WeatherSpecialed") && (weather == Hail || weather == SandStorm) &&!immuneTypes.contains(getType(i,1))
                        && !immuneTypes.contains(getType(i,2)) && !hasWorkingAbility(i, Ability::MagicGuard)) {
		    notify(All, WeatherMessage, i, qint8(HurtWeather),qint8(weather));
		    inflictDamage(i, poke(i).totalLifePoints()/16, i, false);
		}
	    }
	}
	if (count > 0) {
            weatherCount = count;
	}
    }
}

bool BattleSituation::isWeatherWorking(int weather) {
    if (this->weather != weather)
        return false;

    //Air lock & Cloud nine

    for (int i = 0; i < numberOfSlots(); i++)  {
        if (!koed(i) && (hasWorkingAbility(i, Ability::AirLock) || hasWorkingAbility(i, Ability::CloudNine))) {
            return false;
        }
    }
    return true;
}

bool BattleSituation::isSeductionPossible(int seductor, int naiveone) {
    //Oblivious
    return !hasWorkingAbility(naiveone,Ability::Oblivious) && poke(seductor).gender() != Pokemon::Neutral
            && poke(naiveone).gender() != Pokemon::Neutral && poke(seductor).gender() != poke(naiveone).gender();
}

bool BattleSituation::hasType(int player, int type)
{
    return getType(player,1) == type  || getType(player,2) == type;
}

int BattleSituation::getType(int player, int slot)
{
    int types[] = {fieldpokes[player].type1,fieldpokes[player].type2};

    if (types[slot-1] == Pokemon::Flying && pokelong[player].value("Roosted").toBool())
    {
	return Pokemon::Curse;
    }

    return types[slot-1];
}

bool BattleSituation::isFlying(int player)
{
    return !battlelong.value("Gravity").toBool() && !hasWorkingItem(player, Item::IronBall) && !pokelong[player].value("Rooted").toBool() &&
           !pokelong[player].value("Roosted").toBool() && (hasWorkingAbility(player, Ability::Levitate)
                                                           || hasType(player, Pokemon::Flying)
                                                           || pokelong[player].value("MagnetRiseCount").toInt() > 0);
}

bool BattleSituation::hasSubstitute(int player)
{
    return !koed(player) && (pokelong[player].value("Substitute").toBool() || turnlong[player].value("HadSubstitute").toBool());
}

void BattleSituation::changeStatus(int player, int status, bool tell, int turns)
{
    if (poke(player).status() == status) {
	return;
    }

    /* Guts needs to know if teh poke rested or not */
    if (pokelong[player].value("Rested").toBool()) {
        pokelong[player].remove("Rested");
    }

    //Sleep clause
    if (status != Pokemon::Asleep && currentForcedSleepPoke[this->player(player)] == currentPoke(player)) {
        currentForcedSleepPoke[this->player(player)] = -1;
    }

    if (tell)
        notify(All, StatusChange, player, qint8(status));
    notify(All, AbsStatusChange, player, qint8(currentPoke(player)), qint8(status), turns > 0);
    poke(player).changeStatus(status);
    if (turns != 0)
        poke(player).statusCount() = turns;
    else if (status == Pokemon::Asleep) {
        if (gen() <= 4) {
            poke(player).statusCount() = 1 + (true_rand()) % 4;
        } else {
            poke(player).statusCount() = 2 + (true_rand()) % 3;
        }
    }
    else {
        poke(player).statusCount() = 0;
    }
    callpeffects(player, player,"AfterStatusChange");
    callieffects(player, player,"AfterStatusChange");
}

void BattleSituation::changeStatus(int team, int poke, int status)
{
    if (isOut(team, poke)) {
        changeStatus(currentPoke(slot(team,0)) == poke ? slot(team,0) : slot(team,1), status);
    } else {
        this->poke(team, poke).changeStatus(status);
        notify(All, AbsStatusChange, team, qint8(poke), qint8(status));
        //Sleep clause
        if (status != Pokemon::Asleep && currentForcedSleepPoke[team] == poke) {
            currentForcedSleepPoke[team] = -1;
        }
    }
}

bool BattleSituation::hasMinimalStatMod(int player, int stat)
{
    return pokelong[player].value(QString("Boost%1").arg(stat)).toInt() <= -6;
}

bool BattleSituation::hasMaximalStatMod(int player, int stat)
{
    return pokelong[player].value(QString("Boost%1").arg(stat)).toInt() >= 6;
}

void BattleSituation::preventStatMod(int player, int attacker)
{
    turnlong[player][QString("StatModFrom%1Prevented").arg(attacker)] = true;
    turnlong[player][QString("StatModFrom%1DPrevented").arg(attacker)] = true;
}

bool BattleSituation::canSendPreventMessage(int defender, int attacker) {
    return attacking() || (!turnlong[defender].contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
            fieldmoves[attacker].rateOfStat== 0);
}

bool BattleSituation::canSendPreventSMessage(int defender, int attacker) {
    return attacking() || (!turnlong[defender].contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
            fieldmoves[attacker].rate == 0);
}

void BattleSituation::changeStatMod(int player, int stat, int newstat)
{
    QString path = tr("Boost%1").arg(stat);
    pokelong[player][path] = newstat;
}

int BattleSituation::calculateDamage(int p, int t)
{
    callaeffects(p,t,"DamageFormulaStart");

    context &move = turnlong[p];
    PokeBattle &poke = this->poke(p);

    int level = fieldpokes[p].level;
    int attack, def;
    bool crit = move["CriticalHit"].toBool();

    int attackused = move["MoveChosen"].toInt();

    int cat = fieldmoves[p].category;
    if (cat == Move::Physical) {
	attack = getStat(p, Attack);
	def = getStat(t, Defense);
    } else {
	attack = getStat(p, SpAttack);
        def = getStat(t, (attackused == Move::PsychoShock || attackused == Move::PsychoBreak || attackused == Move::SkinSword) ? Defense : SpDefense);
    }

    if ( (attackused == Move::Explosion || attackused == Move::Selfdestruct) && gen() <= 4) /* explosion / selfdestruct */
	def/=2;

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = true_rand() % (255-217) + 217;
    //Spit Up
    if (attackused == Move::SpitUp)
        randnum = 255;
    int ch = 1 + (crit * (1+hasWorkingAbility(p,Ability::Sniper))); //Sniper
    int power = fieldmoves[p].power;
    int type = fieldmoves[p].type;

    if (move.contains("HelpingHanded")) {
        power = power * 3 / 2;
    }

    move.remove("BasePowerItemModifier");
    callieffects(p,t,"BasePowerModifier");
    power = power * (10+move["BasePowerItemModifier"].toInt())/10;

    QString sport = "Sported" + QString::number(type);
    if (battlelong.contains(sport) && pokelong[battlelong[sport].toInt()].value(sport).toBool()) {
	power /= 2;
    }

    move.remove("BasePowerAbilityModifier");
    move.remove("BasePowerFoeAbilityModifier");
    callaeffects(p,t,"BasePowerModifier");
    callaeffects(t,p,"BasePowerFoeModifier");
    power = power * (20+move.value("BasePowerAbilityModifier").toInt())/20 * (20+move.value("BasePowerFoeAbilityModifier").toInt())/20;

    int damage = ((((level * 2 / 5) + 2) * power * attack / 50) / def);
    //Guts, burn
    damage = damage * (
            (poke.status() == Pokemon::Burnt && cat == Move::Physical && !hasWorkingAbility(p,Ability::Guts))
                       ? PokeFraction(1,2) : PokeFraction(1,1));

    /* Light screen / Reflect */
    if (!crit && !hasWorkingAbility(p, Ability::SlipThrough) &&
        teamzone[this->player(t)].value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
        if (!doubles())
            damage /= 2;
        else {
            damage = damage * 2 / 3;
        }
    }
    /* Damage reduction in doubles, which occur only
       if there's more than one alive target. */
    if (doubles()) {
        /* In gen 3, attacks that hit everyone don't have reduced damage */
        if (gen() >= 4 || (fieldmoves[p].targets != Move::All && fieldmoves[p].targets != Move::AllButSelf) ) {
            if (attackused == Move::Explosion || attackused == Move::Selfdestruct) {
                damage = damage * 3 / 4;
            } else {
                foreach (int tar, targetList) {
                    if (tar != t && !koed(tar)) {
                        damage = damage * 3/4;
                        break;
                    }
                }
            }
        }
    }
    if (isWeatherWorking(Sunny)) {
        if (type == Type::Fire) {
	    damage = damage * 3 /2;
        } else if (type == Type::Water) {
	    damage /= 2;
	}
    } else if (isWeatherWorking(Rain)) {
        if (type == Type::Water) {
	    damage = damage * 3/2;
        } else if (type == Type::Fire) {
	    damage /= 2;
	}
    }
    //FlashFire
    if (type == Type::Fire && pokelong[p].contains("FlashFired") && hasWorkingAbility(p, Ability::FlashFire)) {
        damage = damage * 3 / 2;
    }
    damage = (damage+2)*ch;
    move.remove("ItemMod2Modifier");
    callieffects(p,t,"Mod2Modifier");
    damage = damage*(10+move["ItemMod2Modifier"].toInt())/10/*Mod2*/;
    damage = damage *randnum*100/255/100*stab/2*typemod/4;

    /* Mod 3 */
    // FILTER / SOLID ROCK
    if (typemod > 4 && (hasWorkingAbility(t,Ability::Filter) || hasWorkingAbility(t,Ability::SolidRock))) {
        damage = damage * 3 / 4;
    }

    /* Expert belt */
    damage = damage * ((typemod > 4 && hasWorkingItem(p, Item::ExpertBelt))? 6 : 5)/5;

    move.remove("Mod3Berry");

    /* Berries of the foe */
    callieffects(t, p, "Mod3Items");

    damage = damage * (10 + turnlong[p].value("Mod3Berry").toInt())/ 10;

    return damage;
}

int BattleSituation::repeatNum(int player)
{
    if (turnlong[player].contains("RepeatCount")) {
        return turnlong[player]["RepeatCount"].toInt();
    }

    if (fieldmoves[player].repeatMin == 0)
        return 1;

    int min = fieldmoves[player].repeatMin;
    int max = fieldmoves[player].repeatMax;

    if (max == 3) {
        //Triple kick, done differently...
        return 1;
    }
    //Skill link
    if (hasWorkingAbility(player, Ability::SkillLink)) {
        return max;
    }

    if (min == max) {
	return min;
    } else if (min == 2 && max == 5) {
        switch (rand () % 8) {
        case 0: case 1: case 2: return 2;
        case 3: case 4: case 5: return 3;
        case 6: return 4;
        case 7: default: return 5;
        }
    } else {
        return min + (true_rand() % (max-min));
    }
}

void BattleSituation::inflictPercentDamage(int player, int percent, int source, bool straightattack) {
    inflictDamage(player,poke(player).totalLifePoints()*percent/100,source, straightattack);
}

void BattleSituation::inflictDamage(int player, int damage, int source, bool straightattack, bool goForSub)
{
    if (koed(player)) {
	return;
    }

    if (straightattack)
	callieffects(player, source, "BeforeTakingDamage");

    if (damage == 0) {
	damage = 1;
    }

    bool sub = hasSubstitute(player);

    if (sub && (player != source || goForSub) && straightattack) {
	inflictSubDamage(player, damage, source);
    } else {
	damage = std::min(int(poke(player).lifePoints()), damage);

	int hp  = poke(player).lifePoints() - damage;

        bool survivalItem = false;

        if (hp <= 0 && (straightattack && ((turnlong[player].contains("CannotBeKoedBy") && turnlong[player]["CannotBeKoedBy"].toInt() == source)
                                            || (turnlong[player].value("CannotBeKoed").toBool() && source != player)))) {
	    damage = poke(player).lifePoints() - 1;
	    hp = 1;
            survivalItem = true;
	}

	if (hp <= 0) {
	    koPoke(player, source, straightattack);
	} else {
	    changeHp(player, hp);

            if (straightattack) {
                notify(this->player(player), StraightDamage,player, qint16(damage));
                notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
            }

            /* Endure & Focus Sash */
            if (survivalItem) {
                callieffects(player, source, "UponSelfSurvival");
                calleffects(player, source, "UponSelfSurvival");
            }
	}
    }


    if (straightattack && player != source) {
	if (!sub) {
            /* If there's a sub its already taken care of */
            turnlong[source]["DamageInflicted"] = damage;
	    pokelong[player]["DamageTakenByAttack"] = damage;
	    turnlong[player]["DamageTakenByAttack"] = damage;
	    turnlong[player]["DamageTakenBy"] = source;
	}

	if (damage > 0) {
	    inflictRecoil(source, player);
	    callieffects(source,player, "UponDamageInflicted");
	    calleffects(source, player, "UponDamageInflicted");
	}
        callaeffects(player, source, "UponOffensiveDamageReceived");
        if (!sub) {
            calleffects(player, source, "UponOffensiveDamageReceived");
            callpeffects(player, source, "UponOffensiveDamageReceived");
            callieffects(player, source, "UponOffensiveDamageReceived");
        }
    }

    if (!sub)
	turnlong[player]["DamageTaken"] = damage;
}

void BattleSituation::changeTempMove(int player, int slot, int move)
{
    fieldpokes[player].moves[slot] = move;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempMove), quint8(slot), quint16(move));
    changePP(player,slot,std::min(MoveInfo::PP(move, gen()), 5));
}

void BattleSituation::changeSprite(int player, Pokemon::uniqueId newForme)
{
    notify(All, ChangeTempPoke, player, quint8(TempSprite), newForme);
}

void BattleSituation::inflictSubDamage(int player, int damage, int source)
{
    int life = pokelong[player]["SubstituteLife"].toInt();

    if (life <= damage) {
	pokelong[player]["Substitute"] = false;
	turnlong[source]["DamageInflicted"] = life;
	sendMoveMessage(128, 1, player);
        notifySub(player, false);
    } else {
	pokelong[player]["SubstituteLife"] = life-damage;
	turnlong[source]["DamageInflicted"] = damage;
        sendMoveMessage(128, 3, player);
    }
}

void BattleSituation::disposeItem(int  player) {
    int item = poke(player).item();
    if (item != 0) {
        teamzone[this->player(player)]["RecyclableItem"] = item;
    }
    loseItem(player);
}

void BattleSituation::eatBerry(int player, bool show) {
    if (show && !turnlong[player].value("BugBiter").toBool())
        sendItemMessage(8000,player,0, 0, poke(player).item());
    pokelong[player]["BerryUsed"] = poke(player).item();
    disposeItem(player);
}

void BattleSituation::acqItem(int player, int item) {
    if (poke(player).item() != 0)
        loseItem(player);
    poke(player).item() = item;
    ItemEffect::setup(poke(player).item(),player,*this);
}

void BattleSituation::loseItem(int player)
{
    poke(player).item() = 0;
}

void BattleSituation::changeForme(int player, int poke, const Pokemon::uniqueId &newforme)
{
    PokeBattle &p  = this->poke(player,poke);
    p.num() = newforme;
    p.ability() = PokemonInfo::Abilities(newforme).ab(0);

    for (int i = 1; i < 6; i++)
        p.setNormalStat(i,PokemonInfo::Stat(newforme,i,p.level(),p.dvs()[i], p.evs()[i]));

    if (poke == currentPoke(player)) {
        changeSprite(player, newforme);

        fieldpokes[player].id = newforme;
        acquireAbility(player, p.ability());

        for (int i = 1; i < 6; i++)
            fieldpokes[player].stats[i] = p.normalStat(i);

        fieldpokes[player].type1 = PokemonInfo::Type1(newforme);
        fieldpokes[player].type2 = PokemonInfo::Type2(newforme);
    }

    notify(All, ChangeTempPoke, player, quint8(DefiniteForme), quint8(poke),newforme);
}

void BattleSituation::changePokeForme(int slot, const Pokemon::uniqueId &newforme)
{
    int player = this->player(slot);
    int poke = this->currentPoke(slot);

    PokeBattle &p  = this->poke(player,poke);

    fieldpokes[player].id = newforme;
    fieldpokes[player].type1 = PokemonInfo::Type1(newforme);
    fieldpokes[player].type2 = PokemonInfo::Type2(newforme);

    for (int i = 1; i < 6; i++)
        fieldpokes[player].stats[i] = PokemonInfo::Stat(newforme,i,p.level(),p.dvs()[i], p.evs()[i]);

    notify(All, ChangeTempPoke, slot, quint8(AestheticForme), quint16(newforme.subnum));
}


void BattleSituation::changeAForme(int player, int newforme)
{
    fieldpokes[player].id.subnum = newforme;
    notify(All, ChangeTempPoke, player, quint8(AestheticForme), quint16(newforme));
}

void BattleSituation::healLife(int player, int healing)
{
    if ((pokelong[player].value("TurnEffectCall").toBool() || pokelong[player].value("PokeEffectCall").toBool()) && pokelong[player].value("HealBlockCount").toInt() > 0)
	return;
    if (healing == 0) {
	healing = 1;
    }
    if (!koed(player) && !poke(player).isFull())
    {
	healing = std::min(healing, poke(player).totalLifePoints() - poke(player).lifePoints());
	changeHp(player, poke(player).lifePoints() + healing);
    }
}

void BattleSituation::healDamage(int player, int target)
{
    int attack = fieldmoves[player].attack;

    if (koed(target) || attack == Move::MorningSun || attack == Move::Moonlight || attack == Move::Synthesis || attack == Move::Swallow)
        return;

    int healing = fieldmoves[player].healing;

    if (healing > 0) {
        sendMoveMessage(60, 0, player, fieldmoves[player].type);
        healLife(player, poke(target).totalLifePoints() * healing / 100);
    } else if (healing < 0){
        notify(All, Recoil, target, true);
        inflictDamage(player, -poke(target).totalLifePoints() * healing / 100, target);
   }
}

void BattleSituation::changeHp(int player, int newHp)
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
    notify(AllButPlayer, ChangeHp, player, quint16(poke(player).lifePercent())); /* percentage calculus */

    callieffects(player, player, "AfterHPChange");
    callaeffects(player, player, "AfterHPChange");
}

void BattleSituation::koPoke(int player, int source, bool straightattack)
{
    if (poke(player).ko()) {
	return;
    }

    qint16 damage = poke(player).lifePoints();

    changeHp(player, 0);

    if (straightattack) {
        notify(this->player(player), StraightDamage,player, qint16(damage));
        notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
    }

    changeStatus(player,Pokemon::Koed);

    notify(All, Ko, player);
    //useful for third gen
    turnlong[player]["WasKoed"] = true;

    if (straightattack && player!=source) {
	callpeffects(player, source, "AfterKoedByStraightAttack");
        callaeffects(source, player, "AfterKoing");
    }
}

void BattleSituation::requestSwitchIns()
{
    testWin();

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
            notify(Player1, StartChoices, Player1);
        }

        if (!allChoicesOkForPlayer(Player2)) {
            notify(Player2, StartChoices, Player2);
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

    std::vector<int> sorted = sortedBySpeed();

    /* Each wave calls the abilities in order , then next wave and so on. */
    foreach(int p, sorted) {
        if (sentPokes.contains(p)) {
            callEntryEffects(p);
        }
    }
}

void BattleSituation::requestSwitch(int slot)
{
    testWin();

    int player = this->player(slot);

    if (countBackUp(player) == 0) {
        //No poke to switch in, so we won't request a choice & such;
        return;
    }

    notifyInfos();

    options[slot] = BattleChoices::SwitchOnly(slot);

    requestChoice(slot,true,true);
    analyzeChoice(slot);
    callEntryEffects(slot);
}

bool BattleSituation::linked(int linked, QString relationShip)
{
    if (!pokelong[linked].contains(relationShip + "By"))
        return false;

    int linker = pokelong[linked][relationShip + "By"].toInt();

    return  !koed(linker) && slotzone[linker]["SwitchCount"].toInt() == pokelong[linked][relationShip + "Count"].toInt();
}

void BattleSituation::link(int linker, int linked, QString relationShip)
{
    pokelong[linked][relationShip+"By"] = linker;
    pokelong[linked][relationShip+"Count"] = slotzone[linker]["SwitchCount"].toInt();
}

int BattleSituation::countAlive(int player) const
{
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (!poke(player, i).ko()) {
	    count += 1;
	}
    }
    return count;
}

int BattleSituation::countBackUp(int player) const
{
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (poke(player, i).num() != 0 && !poke(player, i).ko() && currentPoke(slot(player)) != i && (!doubles() || currentPoke(slot(player, true)) != i)) {
            count += 1;
        }
    }
    return count;
}

void BattleSituation::testWin()
{
    int time1 = std::max(0, timeLeft(Player1));
    int time2 = std::max(0, timeLeft(Player2));

    /* No one wants a battle that long xd */
    if (turn() == 1024) {
        notify(All, BattleEnd, Player1, qint8(Tie));
        emit battleFinished(publicId(), Tie, id(Player1), id(Player2));
        exit();
    }

    if (time1 == 0 || time2 == 0) {
        notify(All,ClockStop,Player1,quint16(time1));
        notify(All,ClockStop,Player2,quint16(time2));
        notifyClause(ChallengeInfo::NoTimeOut,true);
        if (time1 == 0 && time2 ==0) {
            notify(All, BattleEnd, Player1, qint8(Tie));
            emit battleFinished(publicId(), Tie, id(Player1), id(Player2));
        } else if (time1 == 0) {
            notify(All, BattleEnd, Player2, qint8(Win));
            notify(All, EndMessage, Player2, winMessage[Player2]);
            notify(All, EndMessage, Player1, loseMessage[Player1]);
            emit battleFinished(publicId(), Win, id(Player2), id(Player1));
        } else {
            notify(All, BattleEnd, Player1, qint8(Win));
            notify(All, EndMessage, Player1, winMessage[Player1]);
            notify(All, EndMessage, Player2, loseMessage[Player2]);
            emit battleFinished(publicId(), Win, id(Player1), id(Player2));
        }
        exit();
    }

    int c1 = countAlive(Player1);
    int c2 = countAlive(Player2);

    if (c1*c2==0) {
        notify(All,ClockStop,Player1,time1);
        notify(All,ClockStop,Player2,time2);
        if (c1 + c2 == 0) {
            notify(All, BattleEnd, Player1, qint8(Tie));
            emit battleFinished(publicId(), Tie, id(Player1), id(Player2));
        } else if (c1 == 0) {
            notify(All, BattleEnd, Player2, qint8(Win));
            notify(All, EndMessage, Player2, winMessage[Player2]);
            notify(All, EndMessage, Player1, loseMessage[Player1]);
            emit battleFinished(publicId(), Win, id(Player2), id(Player1));
        } else {
            notify(All, BattleEnd, Player1, qint8(Win));
            notify(All, EndMessage, Player1, winMessage[Player1]);
            notify(All, EndMessage, Player2, loseMessage[Player2]);
            emit battleFinished(publicId(), Win, id(Player1), id(Player2));
        }
        exit();
    }
}

void BattleSituation::changeCurrentPoke(int player, int poke)
{
    mycurrentpoke[player] = poke;
}

void BattleSituation::changePP(int player, int move, int PP)
{
    poke(player).move(move).PP() = PP;

    notify(this->player(player), ChangePP, player, quint8(move), poke(player).move(move).PP());
}

void BattleSituation::losePP(int player, int move, int loss)
{
    int PP = poke(player).move(move).PP();

    PP = std::max(PP-loss, 0);
    changePP(player, move, PP);
}

void BattleSituation::gainPP(int player, int move, int gain)
{
    int PP = poke(player).move(move).PP();

    PP = std::min(PP+gain, int(poke(player).move(move).totalPP()));
    changePP(player, move, PP);

    notify(this->player(player), ChangePP, player, quint8(move), poke(player).move(move).PP());
}

int BattleSituation::getStat(int player, int stat)
{
    /* Attack and defense switched */
    if (pokelong[player].contains("PowerTricked") && (stat == 1 || stat == 2)) {
        stat = 3 - stat;
    }
    QString q = "Stat"+QString::number(stat);
    turnlong[player].remove(q+"AbilityModifier");
    turnlong[player].remove(q+"PartnerAbilityModifier");
    turnlong[player].remove(q+"ItemModifier");
    callieffects(player, player, "StatModifier");
    callaeffects(player, player, "StatModifier");

    if (doubles()) {
        int partner = this->partner(player);

        if (!koed(partner)) {
            callaeffects(partner, player, "PartnerStatModifier");
        }
    }
    int ret = fieldpokes[player].stats[stat]*getStatBoost(player, stat)*(20+turnlong[player].value(q+"AbilityModifier").toInt())/20;

    if (doubles()) {
        ret = ret * (20+turnlong[player].value(q+"PartnerAbilityModifier").toInt())/20;
    }

     ret = ret * (20+turnlong[player].value(q+"ItemModifier").toInt())/20;

    if (stat == Speed && teamzone[this->player(player)].value("TailWindCount").toInt() > 0){
        ret *= 2;
    }

    //QuickFeet
    if (stat == Speed && poke(player).status() == Pokemon::Paralysed && !hasWorkingAbility(player, Ability::QuickFeet)) {
	ret /= 4;
    }

    if (gen() >= 4 && stat == SpDefense && isWeatherWorking(SandStorm) && hasType(player,Pokemon::Rock))
	ret = ret * 3 / 2;

    if (ret == 0) {
	ret = 1;
    }

    return ret;
}

void BattleSituation::sendMoveMessage(int move, int part, int src, int type, int foe, int other, const QString &q)
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

void BattleSituation::sendAbMessage(int move, int part, int src, int foe, int type, int other)
{
    if (foe == -1) {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type));
    } else if (other == -1) {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe));
    } else {
        notify(All, AbilityMessage, src, quint16(move), uchar(part), qint8(type), qint8(foe), qint16(other));
    }
}



void BattleSituation::sendItemMessage(int move, int src, int part, int foe, int berry, int stat)
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

void BattleSituation::sendBerryMessage(int move, int src, int part, int foe, int berry, int stat)
{
    sendItemMessage(move+8000,src,part,foe,berry,stat);
}


void BattleSituation::fail(int player, int move, int part, int type, int trueSource)
{
    turnlong[player]["FailingMessage"] = false;
    turnlong[player]["Failed"] = true;
    sendMoveMessage(move, part, trueSource != -1? trueSource : player, type, player,turnlong[player]["MoveChosen"].toInt());
}

void BattleSituation::failSilently(int player)
{
    turnlong[player]["FailingMessage"] = false;
    turnlong[player]["Failed"] = true;
}

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = pokelong[player].value(QString("Boost%1").arg(stat)).toInt();

    if (hasWorkingAbility(player,Ability::Simple)) {
        boost = std::max(std::min(boost*2, 6),-6);
    }

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */
    int attacker = this->attacker();
    int attacked = this->attacked();

    if (attacker != -1 && attacked != -1) {
        //Unaware
        if (attacker != player && attacked == player) {
            if ( (hasWorkingAbility(attacker, Ability::Unaware) || fieldmoves[attacker].attack == Move::PaymentPlan
                  || fieldmoves[attacker].attack == Move::SacredSword)
                && (stat == SpDefense || stat == Defense))
                boost = 0;
        } else if (attacker == player && attacked != player && hasWorkingAbility(attacked, Ability::Unaware) && (stat == SpAttack || stat == Attack)) {
            boost = 0;
        }
        //Critical hit
        if (turnlong[attacker].value("CriticalHit").toBool()) {
            if ((stat == Attack || stat == SpAttack) && boost < 0) {
                boost = 0;
            } else if ((stat == Defense || stat == SpDefense) && boost > 0) {
                boost = 0;
            }
        }
    }

    if (stat <= 5) {
        return PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else if (stat == Accuracy) {
	/* Accuracy */
        return PokeFraction(std::max(3+boost, 3), std::max(3-boost, 3));
    } else {
	/* Evasion */
	if (pokelong[player].value("Sleuthed").toBool() && boost > 0) {
	    boost = 0;
	}
	if (battlelong.value("Gravity").toBool()) {
	    boost = std::max(boost-2,-6);
	}
        return PokeFraction(std::max(3-boost, 3), std::max(3+boost, 3));
    }
}

BattleConfiguration BattleSituation::configuration() const
{
    BattleConfiguration ret;

    ret.ids[0] = id(0);
    ret.ids[1] = id(1);
    ret.gen = gen();
    ret.doubles = doubles();

    return ret;
}

void BattleSituation::emitCommand(int slot, int players, const QByteArray &toSend)
{
    if (players == All) {
        emit battleInfo(publicId(), qint32(id(Player1)), toSend);
        emit battleInfo(publicId(), qint32(id(Player2)), toSend);

        spectatorMutex.lock();
        foreach(int id, spectators) {
            emit battleInfo(publicId(), qint32(id), toSend);
        }
        spectatorMutex.unlock();
    } else if (players == AllButPlayer) {
        emit battleInfo(publicId(), qint32(id(opponent(player(slot)))), toSend);

        spectatorMutex.lock();
        foreach(int id, spectators) {
            emit battleInfo(publicId(), qint32(id), toSend);
        }
        spectatorMutex.unlock();
    } else {
        emit battleInfo(publicId(), qint32(id(players)), toSend);
    }
}

BattleDynamicInfo BattleSituation::constructInfo(int slot)
{
    BattleDynamicInfo ret;

    int player = this->player(slot);

    for (int i = 0; i < 7; i++) {
        ret.boosts[i] = pokelong[slot].value("Boost" + QString::number(i+1)).toInt();
    }

    ret.flags = 0;
    if (teamzone[player].contains("Spikes")) {
        switch (teamzone[player].value("Spikes").toInt()) {
        case 1: ret.flags |= BattleDynamicInfo::Spikes; break;
        case 2: ret.flags |= BattleDynamicInfo::SpikesLV2; break;
        case 3: ret.flags |= BattleDynamicInfo::SpikesLV3; break;
        }
    }
    if (teamzone[player].contains("ToxicSpikes")) {
        switch (teamzone[player].value("ToxicSpikes").toInt()) {
        case 1: ret.flags |= BattleDynamicInfo::ToxicSpikes; break;
        case 2: ret.flags |= BattleDynamicInfo::ToxicSpikesLV2; break;
        }
    }
    if (teamzone[player].contains("StealthRock") && teamzone[player].value("StealthRock").toBool()) {
        ret.flags |= BattleDynamicInfo::StealthRock;
    }

    return ret;
}

BattleStats BattleSituation::constructStats(int player)
{
    BattleStats ret;

    for (int i = 0; i < 5; i++) {
        ret.stats[i] = getStat(player, i+1);
    }

    return ret;
}

void BattleSituation::addUproarer(int player)
{
    if (!battlelong.contains("Uproarer")) {
        QVariant v;
        v.setValue(QSharedPointer<QSet<int> > (new QSet<int>()));
        battlelong["Uproarer"] = v;
    }

    battlelong["Uproarer"].value< QSharedPointer<QSet<int> > >()->insert(player);
}

void BattleSituation::removeUproarer(int player)
{
    battlelong["Uproarer"].value<QSharedPointer<QSet<int> > >()->remove(player);
}

bool BattleSituation::isThereUproar()
{
    if (!battlelong.contains("Uproarer")) {
        return false;
    }

    foreach(int player, *battlelong["Uproarer"].value<QSharedPointer< QSet<int> > >()) {
        if (!koed(player) && pokelong[player].value("UproarUntil").toInt() >= turn()) {
            return true;
        }
    }

    return false;
}

void BattleSituation::BasicMoveInfo::reset()
{
    memset(this, 0, sizeof(*this));
}

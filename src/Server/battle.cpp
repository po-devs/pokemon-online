#include "battle.h"
#include "player.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "tier.h"
#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>

BattleSituation::BattleSituation(Player &p1, Player &p2, const ChallengeInfo &c, int id)
        :team1(p1.team()), team2(p2.team())
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

    /* timers for battle timeout */
    timeleft[0] = 5*60;
    timeleft[1] = 5*60;
    timeStopped[0] = true;
    timeStopped[1] = true;
    finished() = false;
    clauses() = c.clauses;
    rated() = c.rated;
    doubles() = c.mode == ChallengeInfo::Doubles;
    numberOfSlots() = doubles() ? 4 : 2;
    if (rated())
        tier() = p1.tier();
    currentForcedSleepPoke[0] = -1;
    currentForcedSleepPoke[1] = -1;
    p1.battle = this;
    p1.battleId() = publicId();
    p2.battle = this;
    p2.battleId() = publicId();

    for (int i = 0; i < numberOfSlots(); i++) {
        mycurrentpoke.push_back(-1);
        options.push_back(BattleChoices());
        slotzone.push_back(context());
        pokelong.push_back(context());
        turnlong.push_back(context());
        choice.push_back(BattleChoice());
        hasChoice.push_back(false);
        couldMove.push_back(false);
    }

    if (clauses() & ChallengeInfo::ChallengeCup) {
        team1.generateRandom();
        team2.generateRandom();
    } else {
        if (clauses() & ChallengeInfo::ItemClause) {
            QSet<int> alreadyItems[2];
            for (int i = 0; i < 6; i++) {
                int o1 = team1.poke(i).item();
                int o2 = team1.poke(i).item();

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
//        if (clauses() & ChallengeInfo::LevelBalance) {
//            for (int i = 0; i < 6; i++) {
//                team1.poke(i).level() = PokemonInfo::LevelBalance(p1.team().poke(i).num());
//                team1.poke(i).updateStats();
//            }
//            for (int i = 0; i < 6; i++) {
//                team2.poke(i).level() = PokemonInfo::LevelBalance(p2.team().poke(i).num());
//                team2.poke(i).updateStats();
//            }
//        }
        if (clauses() & ChallengeInfo::SpeciesClause) {
            QSet<int> alreadyPokes[2];
            for (int i = 0; i < 6; i++) {
                int o1 = PokemonInfo::OriginalForme(team1.poke(i).num());
                int o2 = PokemonInfo::OriginalForme(team2.poke(i).num());

                if (alreadyPokes[0].contains(PokemonInfo::OriginalForme(o1))) {
                    team1.poke(i).num() = 0;
                } else {
                    alreadyPokes[0].insert(o1);
                }
                if (alreadyPokes[1].contains(o2)) {
                    team2.poke(i).num() = 0;
                } else {
                    alreadyPokes[1].insert(o2);
                }
            }
        }
    }
    qDebug() << "BattleSituation between " << team1.name << " and " << team2.name << " instanciated.";
}

MirrorMoveAmn amn;

BattleSituation::~BattleSituation()
{
    /* releases the thread */
    {
	/* So the thread will quit immediately after being released */
	quit = true;
        /* Should be enough */
        sem.release(1000);
	/* In the case the thread has not quited yet (anyway should quit in like 1 nano second) */
	wait();
        delete timer;
    }
}

void BattleSituation::start()
{
    quit = false; /* doin' that cuz if any battle command is called why quit is set to true disasters happen */

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

    QThread::start();
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

void BattleSituation::addSpectator(int id)
{
    /* Assumption: each id is a different player, so key is unique */
    int key = spectatorKey(id);
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
                notify(key, ChangeTempPoke,s, quint8(TempSprite),quint16(pokenum(s)));
                if (forme(s) != poke(s).forme())
                    notify(key, ChangeTempPoke, s, quint8(AestheticForme), quint8(forme(s)));
            }
        }
    }
}

void BattleSituation::removeSpectator(int id)
{
    spectators.remove(spectatorKey(id));
    notify(All, Spectating, 0, false, qint32(id));
}

int BattleSituation::forme(int player)
{
    return pokelong[player]["Forme"].toInt();
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

    try {
        qDebug() << "BattleSituation between " << team1.name << " and " << team2.name << " begin running.";
	while (!quit)
	{
	    beginTurn();

	    endTurn();
	}
    } catch(const QuitException &ex) {
	; /* the exception is just there to get immediately out of the while , nothing more
	   We could even have while (1) instead of while(!quit) (but we don't! ;) )*/
    } catch(...) {
        qDebug() << "unkown exception caught in battle between " << team1.name << " and " << team2.name;
    }
}

void BattleSituation::beginTurn()
{
    turn() += 1;
    /* Resetting temporary variables */
    for (int i = 0; i < numberOfSlots(); i++)
        turnlong[i].clear();

    for (int i = 0; i < numberOfSlots(); i++) {
        callpeffects(i, i, "TurnSettings");
    }

    requestChoices();

    /* preventing the players from cancelling (like when u-turn/Baton pass) */
    for (int i = 0; i < numberOfSlots(); i++)
        couldMove[i] = false;

    analyzeChoices();
}

void BattleSituation::endTurn()
{
    testWin();

    std::vector<int> players = sortedBySpeed();

    callzeffects(Player1, Player1, "EndTurn");
    callzeffects(Player2, Player2, "EndTurn");

    foreach (int player, players) {
        callseffects(player,player, "EndTurn2");
    }

    endTurnWeather();

    callbeffects(Player1,Player1,"EndTurn5");

    foreach (int player, players) {
        /* Ingrain, aquaring */
        callpeffects(player, player, "EndTurn60");

        /* Speed boost, shed skin */
        callaeffects(player,player, "EndTurn62");

//        if (koed(player)) <-- cannot be koed
//            continue;

        /* Lefties, black sludge */
        callieffects(player, player, "EndTurn63");

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


    foreach (int player, players) {
        callseffects(player,player, "EndTurn7");
    }

    foreach (int player, players) {
        callpeffects(player,player, "EndTurn8");
    }

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
    if (koed(player))
        return;

    switch(poke(player).status())
    {
        case Pokemon::Burnt:
            notify(All, StatusMessage, player, qint8(HurtBurn));
            //HeatProof: burn does only 1/16
            inflictDamage(player, poke(player).totalLifePoints()/(8*(1+hasWorkingAbility(player,Ability::Heatproof))), player);
            break;
        case Pokemon::DeeplyPoisoned:
            //Poison Heal
            if (hasWorkingAbility(player, Ability::PoisonHeal)) {
                sendAbMessage(45,0,player,Pokemon::Poison);
                healLife(player, poke(player).totalLifePoints()/8);
            } else {
                notify(All, StatusMessage, player, qint8(HurtPoison));
                inflictDamage(player, poke(player).totalLifePoints()*(pokelong[player]["ToxicCount"].toInt()+1)/16, player);
            }
            pokelong[player]["ToxicCount"] = std::min(pokelong[player]["ToxicCount"].toInt()+1, 14);
            break;
        case Pokemon::Poisoned:
            //PoisonHeal
            if (hasWorkingAbility(player, Ability::PoisonHeal)) {
                sendAbMessage(45,0,player,Pokemon::Poison);
                healLife(player, poke(player).totalLifePoints()/8);
            } else {
                notify(All, StatusMessage, player, qint8(HurtPoison));
                inflictDamage(player, poke(player).totalLifePoints()/8, player);
            }
            break;
    }
}

void BattleSituation::testquit()
{
    if (quit) {
	throw QuitException();
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
        blocked() = true;
        sem.acquire(1); /* Lock until a choice is received */
    }

    //test to see if the quit was requested by system or if choice was received
    testquit();
    testWin();

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
        blocked() = true;
        sem.acquire(1);
    }

    //test to see if the quit was requested by system or if choice was received or if win time out
    testquit();
    testWin();

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
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice[slot].attack()) {
        turnlong[slot]["Target"] = choice[slot].target();
        if (!koed(slot) && !turnlong[slot].value("HasMoved").toBool() && !turnlong[slot].value("CantGetToMove").toBool()) {
            if (turnlong[slot].contains("NoChoice"))
                /* Automatic move */
                useAttack(slot, pokelong[slot]["LastSpecialMoveUsed"].toInt(), true);
            else {
                if (options[slot].struggle()) {
                    MoveEffect::setup(394,slot,0,*this);
                    useAttack(slot, 394, true);
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
	else
            priorities[turnlong[i]["SpeedPriority"].toInt()].push_back(i);
    }

    foreach(int player, switches) {
	analyzeChoice(player);
        callEntryEffects(player);
    }

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;

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
                analyzeChoice(p);
            }
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
            int target = MoveInfo::Target(move(b.numSlot, b.numSwitch));

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
        /* The battle thread can carry on */
        blocked() = false;
        /* Blocking any further cancels */
        for (int i = 0; i < numberOfSlots(); i++) {
            couldMove[i] = false;
        }
        sem.release(1);
    }
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
            timeleft[player] = std::min(int(timeleft[player]+30), 5*60);
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
        sem.release(1000); // the battle is finished, isn't it?
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
    pokelong[slot]["Num"] = poke(slot).num();
    pokelong[slot]["Weight"] = PokemonInfo::Weight(poke(slot).num());
    pokelong[slot]["Type1"] = PokemonInfo::Type1(poke(slot).num());
    pokelong[slot]["Type2"] = PokemonInfo::Type2(poke(slot).num());
    pokelong[slot]["Ability"] = poke(slot).ability();
    pokelong[slot]["Forme"] = poke(slot).forme();

    for (int i = 0; i < 4; i++) {
        pokelong[slot]["Move" + QString::number(i)] = poke(slot).move(i).num();
    }

    for (int i = 1; i < 6; i++)
        pokelong[slot][QString("Stat%1").arg(i)] = poke(slot).normalStat(i);

    for (int i = 0; i < 6; i++) {
        pokelong[slot][QString("DV%1").arg(i)] = poke(slot).dvs()[i];
    }

    pokelong[slot]["Level"] = poke(slot).level();

    /* Increase the "switch count". Switch count is used to see if the pokémon has switched
       (like for an attack like attract), it is imo more effective that other means */
    inc(slotzone[player]["SwitchCount"], 1);

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
        acquireAbility(player, poke(player).ability());
        calleffects(player, player, "AfterSwitchIn");
    }
}

void BattleSituation::calleffects(int source, int target, const QString &name)
{

    if (turnlong[source].contains("Effect_" + name)) {
	turnlong[source]["TurnEffectCall"] = true;
        turnlong[source]["TurnEffectCalled"] = name;
        QSet<QString> &effects = *turnlong[source]["Effect_" + name].value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
	    turnlong[source]["EffectBlocked"] = false;
	    turnlong[source]["EffectActivated"] = effect;
            callpeffects(target, source, "BlockTurnEffects");
	    if (turnlong[source]["EffectBlocked"].toBool() == true) {
		continue;
	    }

            MoveMechanics::function f = turnlong[source]["Effect_" + name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
	turnlong[source]["TurnEffectCall"] = false;
    }
}

void BattleSituation::callpeffects(int source, int target, const QString &name)
{
    if (pokelong[source].contains("Effect_" + name)) {
        turnlong[source]["PokeEffectCall"] = true;
        QSet<QString> &effects = *pokelong[source]["Effect_" + name].value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MoveMechanics::function f = pokelong[source]["Effect_" + name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
	turnlong[source]["PokeEffectCall"] = false;
    }
}

void BattleSituation::callbeffects(int source, int target, const QString &name)
{
    if (battlelong.contains("Effect_" + name)) {
        QSet<QString> &effects = *battlelong["Effect_" + name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
            MoveMechanics::function f = battlelong["Effect_" + name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callzeffects(int source, int target, const QString &name)
{
    if (teamzone[source].contains("Effect_" + name)) {
        QSet<QString> &effects = *teamzone[source]["Effect_" + name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
            MoveMechanics::function f = teamzone[source]["Effect_" + name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callseffects(int source, int target, const QString &name)
{
    if (slotzone[source].contains("Effect_" + name)) {
        QSet<QString> &effects = *slotzone[source]["Effect_" + name].value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MoveMechanics::function f = slotzone[source]["Effect_" + name + "_" + effect].value<MoveMechanics::function>();

            f(source, target, *this);
        }
    }
}

void BattleSituation::callieffects(int source, int target, const QString &name)
{
    //Klutz
    if (!pokelong[source].value("Embargoed").toBool() && !hasWorkingAbility(source, Ability::Klutz))
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
    QList<int> opps = revs(player);
    foreach(int opp, opps) {
        if (turnlong[opp].value("Attack").toInt() == Move::Pursuit && !turnlong[opp]["HasMoved"].toBool()) {
            turnlong[opp]["Power"] = turnlong[opp]["Power"].toInt() * 2;
            choice[opp].targetPoke = player;
            analyzeChoice(opp);

            if (koed(player)) {
                Mechanics::removeFunction(turnlong[player],"UponSwitchIn","BatonPass");
                break;
            }
        }
    }

    if (!koed(player)) {
        callaeffects(player,player,"UponSwitchOut");
	changeCurrentPoke(player, -1);
    }
}

bool BattleSituation::testAccuracy(int player, int target, bool silent)
{
    int acc = turnlong[player]["Accuracy"].toInt();
    int tarChoice = turnlong[player]["PossibleTargets"].toInt();
    bool muliTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

    turnlong[target].remove("EvadeAttack");
    callpeffects(target, player, "TestEvasion"); /*dig bounce ..., still calling it there cuz x2 attacks
            like EQ on dig need their boost even if lock on */

    if (pokelong[player].contains("LockedOn") && pokelong[player].value("LockedOnEnd").toInt() >= turn()
            && pokelong[player].value("LockedOn") == target && pokelong[player].value("LockedOnCount") == pokelong[target]["SwitchCount"].toInt()) {
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

    if (acc == 0) {
	return true;
    }

    //OHKO
    int move  = turnlong[player]["MoveChosen"].toInt();

    if (MoveInfo::isOHKO(move)) {
        bool ret = (true_rand() % 100) < 30 + poke(player).level() - poke(target).level();
        if (!ret && !silent) {
            if (muliTar) {
                notify(All, Avoid, target);
            } else {
                notify(All, Miss, player);
            }
        }
        return ret;
    }

    turnlong[player].remove("Stat7ItemModifier");
    turnlong[player].remove("Stat7AbilityModifier");
    turnlong[target].remove("Stat6ItemModifier");
    turnlong[target].remove("Stat6AbilityModifier");
    callieffects(player,target,"StatModifier");
    callaeffects(player,target,"StatModifier");
    callieffects(target,player,"StatModifier");
    callaeffects(target,player,"StatModifier");
    /* no *=: remember, we're working with fractions & int, changing the order might screw up by 1 % or so
	due to the ever rounding down to make an int */
    acc = acc * getStatBoost(player, 7) * getStatBoost(target, 6)
	    * (20+turnlong[player]["Stat7ItemModifier"].toInt())/20
            * (20-turnlong[target]["Stat6ItemModifier"].toInt())/20
            * (20+turnlong[player]["Stat7AbilityModifier"].toInt())/20
            * (20-turnlong[target]["Stat6AbilityModifier"].toInt())/20;

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
    int craise = turnlong[player]["CriticalRaise"].toInt();

    if (hasWorkingAbility(player, Ability::SuperLuck)) { /* Super Luck */
	craise += 1;
    }

    switch(craise) {
	case 0: minch = 3; break;
	case 1: minch = 6; break;
	case 2: minch = 12; break;
	case 3: minch = 16; break;
	case 4: default: minch = 24; break;
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

    switch (poke(player).status()) {
	case Pokemon::Asleep:
	{
	    if (poke(player).sleepCount() > 0) {
                //Early bird
                poke(player).sleepCount() -= 1 + hasWorkingAbility(player, Ability::EarlyBird);
		notify(All, StatusMessage, player, qint8(FeelAsleep));
		if (!turnlong[player].value("SleepingMove").toBool())
		    return false;
	    } else {
		healStatus(player, Pokemon::Asleep);
		notify(All, StatusMessage, player, qint8(FreeAsleep));
	    }
	    break;
	}
	case Pokemon::Paralysed:
	{
            //MagicGuard
            if (!hasWorkingAbility(player, Ability::MagicGuard) && true_rand() % 4 == 0) {
		notify(All, StatusMessage, player, qint8(PrevParalysed));
		return false;
	    }
	    break;
	}
	case Pokemon::Frozen:
	{
            if (true_rand() % 255 > 51)
	    {
		notify(All, StatusMessage, player, qint8(PrevFrozen));
		return false;
	    }
	    healStatus(player, Pokemon::Frozen);
	    notify(All, StatusMessage, player, qint8(FreeFrozen));
	    break;
	}

	case Pokemon::Fine:
	case Pokemon::Burnt:
	case Pokemon::DeeplyPoisoned:
	case Pokemon::Poisoned:
	default:
	    break;
    }
    if (turnlong[player]["Flinched"].toBool()) {
	notify(All, Flinch, player);
        //SteadFast
        if (hasWorkingAbility(player, Ability::Steadfast)) {
            sendAbMessage(60,0,player);
            gainStatMod(player, Speed, 1);
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

    return true;
}

void BattleSituation::inflictConfusedDamage(int player)
{
    notify(All, StatusMessage, player, qint8(HurtConfusion));

    turnlong[player]["Type"] = Pokemon::Curse;
    turnlong[player]["Power"] = 40;
    turnlong[player]["TypeMod"] = 4;
    turnlong[player]["Stab"] = 2;
    turnlong[player]["Category"] = Move::Physical;
    int damage = calculateDamage(player, player);
    inflictDamage(player, damage, player, true);
}

void BattleSituation::testFlinch(int player, int target)
{
    //Inner focus
    if (hasWorkingAbility(target, Ability::InnerFocus)) {
        return;
    }

    int rate = turnlong[player]["FlinchRate"].toInt();
    int randnum = true_rand() % 100;
    /* Serene Grace */
    if (hasWorkingAbility(player, Ability::SereneGrace)) {
        randnum /= 2;
    }

    if (randnum % 100 < rate) {
	turnlong[target]["Flinched"] = true;
    }

    if (hasWorkingItem(player, Item::KingsRock) && turnlong[player]["KingRock"].toBool()) /* King's rock */
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
    qDebug() << MoveInfo::Name(move) << " was used ! ";
    attacker() = player;

    int attack;

    if (specialOccurence) {
	attack = move;
    } else {
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

	callieffects(player,player, "RegMoveSettings");

        pokelong[player]["LastMoveUsed"] = attack;
        pokelong[player]["LastMoveUsedTurn"] = turn();
    }

    //For metronome calling fly / sky attack / w/e
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
    switch(Move::Target(turnlong[player]["PossibleTargets"].toInt())) {
	case Move::None: targetList.push_back(player); break;
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
        case Move::ChosenTarget: {
                if (doubles()) {
                    int target = turnlong[player]["Target"].toInt();
                    if (!koed(target)) {
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
                int target = turnlong[player]["Target"].toInt();
                if (!koed(target)) {
                    targetList.push_back(target);
                } else {
                    targetList.push_back(player);
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
	if (player != target && !specialOccurence && !amn.contains(attack)) {
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
        callbeffects(player, target, "DetermineGeneralAttackFailure");
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
	if (turnlong[player]["Power"].toInt() > 0)
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

            int num = repeatNum(player, turnlong[player]);
	    bool hit = num > 1;

            int i;
            for (i = 0; i < num && !koed(target); i++) {
                turnlong[target]["HadSubstitute"] = false;
		bool sub = hasSubstitute(target);
                turnlong[target]["HadSubstitute"] = sub;

                if (turnlong[player]["Power"].toInt() > 1 && i == 0)
                    notify(All, Effective, target, quint8(typemod));

		if (turnlong[player]["Power"].toInt() > 1) {
		    testCritical(player, target);
		    int damage = calculateDamage(player, target);
		    inflictDamage(target, damage, player, true);
		} else {
		    calleffects(player, target, "CustomAttackingDamage");
		}
		calleffects(player, target, "UponAttackSuccessful");

		if (turnlong[player]["PhysicalContact"].toBool()) {
		    if (!sub)
			callieffects(target, player, "UponPhysicalAssault");
                    callaeffects(target,player,"UponPhysicalAssault");
		}
		/* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
		applyMoveStatMods(player, target);

		battlelong["LastMoveSuccesfullyUsed"] = attack;
			     /* Chatter Mimic Sketch Struggle  */
		if (!specialOccurence && attack != Move::Chatter && attack != Move::Sketch && attack != Move::Struggle && attack != Move::Mimic) {
		    pokelong[player]["LastMoveSuccessfullyUsed"] = attack;
		    pokelong[player]["LastMoveSuccessfullyUsedTurn"] = turn();
                    battlelong["LastMoveSuccessfullyUsed"] = attack;
		}

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
	    callbeffects(player, target, "DetermineGeneralAttackFailure");
	    if (testFail(player))
		continue;
	    calleffects(player, target, "DetermineAttackFailure");
	    if (testFail(player))
		continue;
            int type = turnlong[player]["Type"].toInt(); /* move type */
            if ( target != player && ((type == Type::Electric && hasType(target, Type::Ground)) ||
                 (type == Type::Poison && (hasType(target, Type::Steel) || hasType(target, Type::Poison))) ||
                 (type == Type::Fire && hasType(target, Type::Fire))) ) {
                notify(All, Failed, player);
                continue;
            }

	    calleffects(player, target, "BeforeHitting");

	    applyMoveStatMods(player, target);
	    calleffects(player, target, "UponAttackSuccessful");

	    battlelong["LastMoveSuccesfullyUsed"] = attack;

	    /* this is put after calleffects to avoid endless sleepTalk/copycat for example */
            if (!specialOccurence && attack != Move::Chatter && attack != Move::Sketch && attack != Move::Struggle && attack != Move::Mimic) {
		    pokelong[player]["LastMoveSuccessfullyUsed"] = attack;
		    pokelong[player]["LastMoveSuccessfullyUsedTurn"] = turn();
                    battlelong["LastMoveSuccessfullyUsed"] = attack;
	    }
	    if (!koed(player))
		calleffects(player, target, "AfterAttackSuccessful");
	}
        if (turnlong[player]["Type"].toInt() == Type::Fire && poke(target).status() == Pokemon::Frozen) {
	    notify(All, StatusMessage, target, qint8(FreeFrozen));
	    healStatus(target, Pokemon::Frozen);
	}
	pokelong[target]["LastAttackToHit"] = attack;
    }

    callieffects(player, player, "AfterTargetList");
    end:
    calleffects(player, player, "BeforeEnding");
    trueend:
    pokelong[player]["HasMovedOnce"] = true;
    /* For U-TURN, so that none of the variables of the switchin are afflicted, it's put at the utmost end */
    calleffects(player, player, "AfterAttackFinished");

    attacker() = -1;
    attacked() = -1;
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

    int type = turnlong[player]["Type"].toInt(); /* move type */
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
        if (player == attacked() && player != attacker() && hasWorkingAbility(attacker(), Ability::MoldBreaker)) {
            return false;
        }
    }
    return pokelong[player].value("AbilityNullified").toBool() ? false : pokelong[player]["Ability"].toInt() == ab;
}

void BattleSituation::acquireAbility(int play, int ab) {
    pokelong[play]["Ability"] = ab;
    AbilityEffect::setup(ability(play),play,*this);
}

int BattleSituation::ability(int player) {
    return pokelong[player]["Ability"].toInt();
}

int BattleSituation::pokenum(int player) {
    return pokelong[player]["Num"].toInt();
}

bool BattleSituation::hasWorkingItem(int player, int it)
{
    //Klutz
    return poke(player).item() == it && !pokelong[player].value("Embargoed").toBool() && !hasWorkingAbility(player, Ability::Klutz);
}

int BattleSituation::move(int player, int slot)
{
    return pokelong[player]["Move"+QString::number(slot)].toInt();
}

void BattleSituation::inflictRecoil(int source, int)
{
    //Rockhead
    if (koed(source) || hasWorkingAbility(source,Ability::RockHead))
        return;

    int recoil = turnlong[source]["Recoil"].toInt();

    if (recoil == 0)
        return;

    notify(All, Recoil, source);
    inflictDamage(source, turnlong[source].value("DamageInflicted").toInt()/recoil, source);
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    bool sub = hasSubstitute(target);

    QString effect = turnlong[player]["StatEffect"].value<QString>();

    /* First we check if there's even an effect... */
    if (effect.length() == 0) {
	return;
    }

    /* Then we check if the effect hits */
    int randnum = true_rand() % 100;
    /* Serene Grace */
    if (hasWorkingAbility(player,Ability::SereneGrace)) {
        randnum /= 2;
    }
    int maxtogo = turnlong[player]["EffectRate"].toInt();

    if (maxtogo != 0 && randnum > maxtogo) {
	return;
    }

    bool statChanges[2] = {false};

    /* Splits effects between the opponent & self */
    QStringList effects = effect.split('|');

    foreach(QString effect, effects)
    {
	/* Now we parse the effect.. */
	bool self = (effect[0] == 'S');
	int targeted = self? player : target;

	/* If the effect is on the opponent, and the opponent is Koed / subbed, we don't do nothing */
	if (koed(targeted)) {
            continue;
	}

        if (!self && sub) {
            if (turnlong[player]["Power"].toInt() == 0)
                sendMoveMessage(128, 2, player,0,target,turnlong[player]["Attack"].toInt());
	    continue;
	}

        //Shield Dust
        if (!self && hasWorkingAbility(targeted, Ability::ShieldDust) && turnlong[player]["Power"].toInt() > 0) {
            sendAbMessage(24,0,targeted,0,Pokemon::Bug);
            continue;
        }

	/* There maybe different type of changes, aka status & mod in move 'Flatter' */
        QStringList changes = effect.mid(1).split('/');
	
	foreach (QString effect, changes)
	{
	    /* Now the kind of change itself */
            bool statusChange = effect.midRef(0,3) == "[S]"; /* otherwise it's stat change */

            if(!self && !statusChange && teamzone[this->player(targeted)].value("MistCount").toInt() > 0) {
		sendMoveMessage(86, 2, player,Pokemon::Ice,target,turnlong[player]["Attack"].toInt());
		continue;
	    }

            if(!self && statusChange && teamzone[this->player(targeted)].value("SafeGuardCount").toInt() > 0) {
		sendMoveMessage(109, 2, player,Pokemon::Psychic,target,turnlong[player]["Attack"].toInt());
		continue;
	    }
    
            QStringList possibilities = effect.mid(3).split('^');
    
	    /* Now we have a QStringLists that contains the different possibilities.
    
	       To know what to do know: we first chose one of the possibility:
		%code
		    mypossibility = possibilities[rand(1, possibilities.size()) - 1];
		%endcode
    
		Then inside that possibility is the list of actions to do
		%code
		    myposs = mypossibility.split('^');
		    foreach (QString s, myposs)
			analyze s
			do action
		    end for
		%endcode
	    */
    
            QStringList mychoice = possibilities[true_rand()%possibilities.size()].split('&');
    
	    foreach (QString s, mychoice) {
		std::string s2 = s.toStdString();
		char *ptr = const_cast<char *>(s2.c_str());
    
		/* Analyze choice */
		if (statusChange) {
		    int status = strtol(ptr, &ptr, 10);
		    bool heal = *ptr == '-';
    
		    if (status == -1) {
			if (heal) {
			    healConfused(targeted);
			} else {
			    inflictConfused(targeted);
			}
		    } else {
			if (heal) {
			    healStatus(targeted, status);
			} else {
                            inflictStatus(targeted, status, player);
			}
		    }
		} else /* StatMod change */
		{
		    int stat = strtol(ptr, &ptr, 10);
		    int mod = strtol(ptr+1, NULL, 10);
		    char sep = *ptr;
    
		    if (sep == '+') {
                        if (stat == Evasion && (clauses() & ChallengeInfo::EvasionClause)) {
                            notifyClause(ChallengeInfo::EvasionClause);
                            continue;
                        }
			gainStatMod(targeted, stat, mod);
		    } else if (sep == '-') {
                        loseStatMod(targeted, stat, mod, player);
		    } else {
			changeStatMod(targeted, stat, mod);
		    }
		    statChanges[self] = true;
		}
	    }
	}
    }
    if (statChanges[0] == true && !koed(player)) {
	callieffects(target,player,"AfterStatChange");
    }
    if (statChanges[1] == true && !koed(player)) {
	callieffects(player,player,"AfterStatChange");
    }
}

void BattleSituation::healConfused(int player)
{
    pokelong[player]["Confused"] = false;
}

void BattleSituation::inflictConfused(int player, bool tell)
{
    //OwnTempo
    if (!pokelong[player]["Confused"].toBool() && !hasWorkingAbility(player,Ability::OwnTempo)) {
	pokelong[player]["Confused"] = true;
        pokelong[player]["ConfusedCount"] = (true_rand() % 4) + 1;
        if (tell)
            notify(All, StatusChange, player, qint8(-1));

        callieffects(player, player,"AfterStatusChange");
    }
}

bool BattleSituation::isConfused(int player)
{
    return pokelong[player].value("Confused").toBool();
}

void BattleSituation::healStatus(int player, int status)
{
    if (poke(player).status() == status) {
	changeStatus(player, Pokemon::Fine);
    }
}

bool BattleSituation::canGetStatus(int player, int status) {
    switch (status) {
    case Pokemon::Paralysed: return !hasWorkingAbility(player, Ability::Limber);
    case Pokemon::Asleep: return !hasWorkingAbility(player, Ability::Insomnia) && !hasWorkingAbility(player, Ability::VitalSpirit);
    case Pokemon::Burnt: return !hasType(player, Pokemon::Fire) && !hasWorkingAbility(player, Ability::WaterVeil);
    case Pokemon::DeeplyPoisoned:
    case Pokemon::Poisoned: return !hasType(player, Pokemon::Poison) && !hasType(player, Pokemon::Steel) && !hasWorkingAbility(player, Ability::Immunity);
    case Pokemon::Frozen: return !isWeatherWorking(Sunny) && !hasType(player, Pokemon::Ice) && !hasWorkingAbility(player, Ability::MagmaArmor);
    default:
        return false;
    }
}

void BattleSituation::inflictStatus(int player, int status, int attacker)
{
    if (poke(player).status() != Pokemon::Fine) {
        if (this->attacker() == attacker && turnlong[attacker]["Power"].toInt() == 0) {
            if (poke(player).status() == status)
                notify(All, AlreadyStatusMessage, player);
            else
                notify(All, Failed, player);
        }
        return;
    }
    if (status == Pokemon::Asleep && isThereUproar()) {
        sendMoveMessage(141,4,player);
        return;
    }
    if (canGetStatus(player,status)) {
        if (attacker != player) {
            QString q = QString("StatModFrom%1Prevented").arg(attacker);
            turnlong[player].remove(q);
            turnlong[player]["StatModType"] = QString("Status");
            turnlong[player]["StatusInflicted"] = status;
            callaeffects(player, attacker, "PreventStatChange");
            if (turnlong[player].contains(q)) {
                return;
            }

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
        }
        changeStatus(player, status);
        if (status == Pokemon::Frozen && poke(player).num() == Pokemon::Shaymin_S) {
            changeForme(player, currentPoke(player), Pokemon::Shaymin_S);
        }
        if (attacker != player && status != Pokemon::Asleep && status != Pokemon::Frozen && poke(attacker).status() == Pokemon::Fine && canGetStatus(attacker,status)
            && hasWorkingAbility(player, Ability::Synchronize)) //Synchronize
        {
            sendAbMessage(61,0,player,attacker);
            // Toxic becomes normal poison upon poisoning
            inflictStatus(attacker,status == Pokemon::DeeplyPoisoned ? Pokemon::Poisoned : status, player);
        }
    }
}

int BattleSituation::weather()
{
    return battlelong["Weather"].toInt();
}

void BattleSituation::callForth(int weather, int turns)
{
    if (weather == this->weather()) {
        battlelong["WeatherCount"] = turns;
    } else {
	battlelong["WeatherCount"] = turns;
	battlelong["Weather"] = weather;
        foreach (int i, sortedBySpeed()) {
            callaeffects(i,i,"WeatherChange");
        }
    }
}

void BattleSituation::endTurnWeather()
{
    int weather = this->weather();

    if (weather == NormalWeather) {
	return;
    }

    int count = battlelong["WeatherCount"].toInt() - 1;
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
                if (!turnlong[i].contains("WeatherSpecialed") && (weather == Hail || weather == SandStorm) &&!immuneTypes.contains(getType(i,1)) && !immuneTypes.contains(getType(i,2))) {
		    notify(All, WeatherMessage, i, qint8(HurtWeather),qint8(weather));
		    inflictDamage(i, poke(i).totalLifePoints()/16, i, false);
		}
	    }
	}
	if (count > 0) {
	    battlelong["WeatherCount"] = count;
	}
    }
}

bool BattleSituation::isWeatherWorking(int weather) {
    if (this->weather() != weather)
        return false;

    //Air lock & Cloud nine

    for (int i = 0; i < numberOfSlots(); i++)  {
        if (hasWorkingAbility(i, Ability::AirLock) || hasWorkingAbility(i, Ability::CloudNine)) {
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
    int types[] = {pokelong[player]["Type1"].toInt(),pokelong[player]["Type2"].toInt()};


    if (!pokelong[player].value("Embargoed").toBool() && ItemInfo::isPlate(poke(player).item())) {
        //multitype
        if (hasWorkingAbility(player, Ability::Multitype)) {
	    types[0] = pokelong[player]["ItemArg"].toInt();
	    types[1] = Pokemon::Curse;
	}
    }

    if (types[slot-1] == Pokemon::Flying && pokelong[player].value("Roosted").toBool())
    {
	return Pokemon::Curse;
    }

    return types[slot-1];
}

bool BattleSituation::isFlying(int player)
{
    /* Item 212 is iron ball, ability is levitate */
    return !battlelong.value("Gravity").toBool() && !hasWorkingItem(player, Item::IronBall) && !pokelong[player].value("Rooted").toBool() &&
            (hasWorkingAbility(player, Ability::Levitate) ||  hasType(player, Pokemon::Flying) || pokelong[player].value("MagnetRiseCount").toInt() > 0);
}

bool BattleSituation::hasSubstitute(int player)
{
    return !koed(player) && (pokelong[player].value("Substitute").toBool() || turnlong[player].value("HadSubstitute").toBool());
}

void BattleSituation::changeStatus(int player, int status, bool tell)
{
    if (poke(player).status() == status) {
	return;
    }

    /* Guts needs to know if teh poke rested or not */
    if (pokelong[player].value("Rested").toBool()) {
        pokelong[player].remove("Rested");
    }

    //Sleep clause
    if (status != Pokemon::Asleep && currentForcedSleepPoke[player] == currentPoke(player)) {
        currentForcedSleepPoke[this->player(player)] = -1;
    }

    if (tell)
        notify(All, StatusChange, player, qint8(status));
    notify(All, AbsStatusChange, player, qint8(currentPoke(player)), qint8(status));
    poke(player).status() = status;
    if (status == Pokemon::Asleep) {
        poke(player).sleepCount() = (true_rand() % 4) +1;
    }
    if (status == Pokemon::DeeplyPoisoned) {
	pokelong[player]["ToxicCount"] = 0;
    }
    callpeffects(player, player,"AfterStatusChange");
    callieffects(player, player,"AfterStatusChange");
}

void BattleSituation::changeStatus(int team, int poke, int status)
{
    if (isOut(team, poke)) {
        changeStatus(currentPoke(slot(team,0)) == poke ? slot(team,0) : slot(team,1), status);
    } else {
	this->poke(team, poke).status() = status;
        notify(All, AbsStatusChange, team, qint8(poke), qint8(status));
        //Sleep clause
        if (status != Pokemon::Asleep && currentForcedSleepPoke[team] == poke) {
            currentForcedSleepPoke[team] = -1;
        }
    }
}

void BattleSituation::gainStatMod(int player, int stat, int bonus, bool tell)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost < 6) {
        if (tell)
            notify(All, StatChange, player, qint8(stat), qint8(bonus));
	changeStatMod(player, stat, std::min(boost+bonus, 6));
    }
}

void BattleSituation::loseStatMod(int player, int stat, int malus, int attacker)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost > -6) {
        if (attacker != player) {
            QString q = QString("StatModFrom%1Prevented").arg(attacker);
            turnlong[player].remove(q);
            turnlong[player]["StatModType"] = QString("Stat");
            turnlong[player]["StatModded"] = stat;
            turnlong[player]["StatModification"] = -malus;
            callaeffects(player, attacker, "PreventStatChange");
            if (turnlong[player].contains(q)) {
                return;
            }
        }
	notify(All, StatChange, player, qint8(stat), qint8(-malus));
	changeStatMod(player, stat, std::max(boost-malus, -6));
    }
}

void BattleSituation::preventStatMod(int player, int attacker) {
    turnlong[player][QString("StatModFrom%1Prevented").arg(attacker)] = true;
    turnlong[player][QString("StatModFrom%1DPrevented").arg(attacker)] = true;
}

bool BattleSituation::canSendPreventMessage(int defender, int attacker) {
    return attacking() || (!turnlong[defender].contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
            turnlong[attacker]["Power"].toInt() == 0);
}

void BattleSituation::changeStatMod(int player, int stat, int newstat)
{
    QString path = tr("Boost%1").arg(stat);
    pokelong[player][path] = newstat;
}

int BattleSituation::calculateDamage(int p, int t)
{
    callaeffects(p,t,"DamageFormulaStart");

    context &player = pokelong[p];
    context &move = turnlong[p];
    PokeBattle &poke = this->poke(p);

    int level = player["Level"].toInt();
    int attack, def;
    bool crit = move["CriticalHit"].toBool();

    int cat = move["Category"].toInt();
    if (cat == Move::Physical) {
	attack = getStat(p, Attack);
	def = getStat(t, Defense);
    } else {
	attack = getStat(p, SpAttack);
	def = getStat(t, SpDefense);
    }
    int attackused = move["MoveChosen"].toInt();

    if (attackused == Move::Explosion || attackused == Move::Selfdestruct) /* explosion / selfdestruct */
	def/=2;

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = true_rand() % (255-217) + 217;
    //Spit Up
    if (attackused == Move::SpitUp)
        randnum = 255;
    int ch = 1 + (crit * (1+hasWorkingAbility(p,Ability::Sniper))); //Sniper
    int power = move["Power"].toInt();
    int type = move["Type"].toInt();

    if (move.contains("HelpingHanded")) {
        power = power * 3 / 2;
    }

    callieffects(p,t,"BasePowerModifier");
    power = power * (10+move["BasePowerItemModifier"].toInt())/10;

    QString sport = "Sported" + QString::number(type);
    if (battlelong.contains(sport) && pokelong[battlelong[sport].toInt()].value(sport).toBool()) {
	power /= 2;
    }

    callaeffects(p,t,"BasePowerModifier");
    callaeffects(t,p,"BasePowerFoeModifier");
    power = power * (20+move["BasePowerAbilityModifier"].toInt())/20 * (20+move["BasePowerFoeAbilityModifier"].toInt())/20;

    int damage = ((((level * 2 / 5) + 2) * power * attack / 50) / def);
    //Guts, burn
    damage = damage * (
            (poke.status() == Pokemon::Burnt && move["Category"].toInt() == Move::Physical && !hasWorkingAbility(p,Ability::Guts))
                       ? PokeFraction(1,2) : PokeFraction(1,1));

    /* Light screen / Reflect */
    if (!crit && teamzone[this->player(t)].value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
        if (!doubles())
            damage /= 2;
        else {
            damage = damage * 2 / 3;
        }
    }
    /* Damage reduction in doubles, which occur only
       if there's more than one alive target. */
    if (doubles()) {
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

    /* Berries of the foe */
    callieffects(t, p, "Mod3Items");

    damage = damage * (10 + turnlong[p].value("Mod3Berry").toInt())/ 10;

    return damage;
}

int BattleSituation::repeatNum(int player, context &move)
{
    if (turnlong[player].contains("RepeatCount")) {
        return turnlong[player]["RepeatCount"].toInt();
    }

    int min = 1+move["RepeatMin"].toInt();
    int max = 1+move["RepeatMax"].toInt();

    //Skill link
    if (hasWorkingAbility(player, Ability::SkillLink)) {
        return max;
    }

    if (min == max) {
	return min;
    } else {
        return min + (true_rand() % (max-min));
    }
}

void BattleSituation::inflictPercentDamage(int player, int percent, int source, bool straightattack) {
    inflictDamage(player,poke(player).totalLifePoints()*percent/100,source, straightattack);
}

void BattleSituation::inflictDamage(int player, int damage, int source, bool straightattack)
{
    if (koed(player)) {
	return;
    }

    //Magic guard
    if (hasWorkingAbility(player, Ability::MagicGuard) && !straightattack) {
        return;
    }

    if (straightattack)
	callieffects(player, source, "BeforeTakingDamage");

    if (damage == 0) {
	damage = 1;
    }

    bool sub = hasSubstitute(player);

    if (sub && player != source && straightattack) {
	inflictSubDamage(player, damage, source);
    } else {
	damage = std::min(int(poke(player).lifePoints()), damage);

	int hp  = poke(player).lifePoints() - damage;

        if (hp <= 0 && (straightattack && ((turnlong[player].contains("CannotBeKoedBy") && turnlong[player]["CannotBeKoedBy"].toInt() == source)
                                            || (turnlong[player].value("CannotBeKoed").toBool() && source != player)))) {
	    damage = poke(player).lifePoints() - 1;
	    hp = 1;
	}

	if (hp <= 0) {
	    koPoke(player, source, straightattack);
	} else {
	    changeHp(player, hp);

            if (straightattack) {
                notify(this->player(player), StraightDamage,player, qint16(damage));
                notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
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
        if (!sub) {
            callieffects(player, source, "UponOffensiveDamageReceived");
            callaeffects(player, source, "UponOffensiveDamageReceived");
            calleffects(player, source, "UponOffensiveDamageReceived");
            callpeffects(player, source, "UponOffensiveDamageReceived");
        }
    }

    if (!sub)
	turnlong[player]["DamageTaken"] = damage;
}

void BattleSituation::changeTempMove(int player, int slot, int move)
{
    pokelong[player]["Move" + QString::number(slot)] = move;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempMove), quint8(slot), quint16(move));
    changePP(player,slot,5);
}

void BattleSituation::changeSprite(int player, int poke)
{
    notify(All, ChangeTempPoke, player, quint8(TempSprite), quint16(poke));
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
    disposeItem(player);
}

void BattleSituation::acqItem(int player, int item) {
    if (poke(player).item() != 0)
        loseItem(player);
    poke(player).item() = item;
    ItemEffect::setup(poke(player).item(),player,*this);
    callieffects(player,player,"AfterSetup");
}

void BattleSituation::loseItem(int player)
{
    //No Griseous Orb -> Giratina back to its ol' self
    if (!koed(player)) {
        if (pokenum(player) == Pokemon::Giratina_O) {
            changeForme(player,currentPoke(player),Pokemon::Giratina);
        } else if (pokenum(player) == Pokemon::Arceus) {
            if (getType(player, 1) != Type::Normal) {
                changeAForme(player, Type::Normal);
            }
        }
    }
    poke(player).item() = 0;
}

void BattleSituation::changeForme(int player, int poke, int newform)
{
    PokeBattle &p  = this->poke(player,poke);
    p.num() = newform;
    p.ability() = PokemonInfo::Abilities(newform).front();

    for (int i = 1; i < 6; i++)
        p.setNormalStat(i,PokemonInfo::Stat(newform,i,p.level(),p.dvs()[i], p.evs()[i]));

    if (poke == currentPoke(player)) {
        changeSprite(player, newform);

        pokelong[player]["Num"] = newform;
        acquireAbility(player, p.ability());

        for (int i = 1; i < 6; i++)
            pokelong[player][QString("Stat%1").arg(i)] = p.normalStat(i);
    }

    notify(All, ChangeTempPoke, player, quint8(DefiniteForm), quint8(poke),quint16(newform));
}

void BattleSituation::changeAForme(int player, int newforme)
{
    pokelong[player]["Forme"] = newforme;
    notify(All, ChangeTempPoke, player, quint8(AestheticForme), quint8(newforme));
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

void BattleSituation::changeHp(int player, int newHp)
{
    if (newHp == poke(player).lifePoints()) {
	/* no change, so don't bother */
	return;
    }
    poke(player).lifePoints() = newHp;

    notify(this->player(player), ChangeHp, player, quint16(newHp));
    notify(AllButPlayer, ChangeHp, player, quint16(poke(player).lifePercent())); /* percentage calculus */

    callieffects(player, player, "AfterHPChange");
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

    if (straightattack && player!=source) {
	callpeffects(player, source, "AfterKoedByStraightAttack");
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

        blocked() = true;
        sem.acquire(1);

        testquit();
        testWin(); // timeout win

        /* To clear the cancellable moves list */
        for (int i = 0; i < numberOfSlots(); i++)
            couldMove[i] = false;

        foreach(int p, koedPokes) {
            analyzeChoice(p);

            if (!koed(p))
                sentPokes.insert(p);
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

    foreach(int p, sentPokes) {
        callEntryEffects(p);
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

    return  !koed(linker) && slotzone[linker]["SwitchCount"] == pokelong[linked][relationShip + "Count"];
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
        emit battleFinished(Tie, id(Player1), id(Player2),rated(), tier());
        throw QuitException();
    }

    if (time1 == 0 || time2 == 0) {
        finished() = true;
        notify(All,ClockStop,Player1,quint16(time1));
        notify(All,ClockStop,Player2,quint16(time2));
        notifyClause(ChallengeInfo::NoTimeOut,true);
        if (time1 == 0 && time2 ==0) {
            notify(All, BattleEnd, Player1, qint8(Tie));
            emit battleFinished(Tie, id(Player1), id(Player2),rated(), tier());
        } else if (time1 == 0) {
            notify(All, BattleEnd, Player2, qint8(Win));
            notify(All, EndMessage, Player2, winMessage[Player2]);
            notify(All, EndMessage, Player1, loseMessage[Player1]);
            emit battleFinished(Win, id(Player2), id(Player1),rated(), tier());
        } else {
            notify(All, BattleEnd, Player1, qint8(Win));
            notify(All, EndMessage, Player1, winMessage[Player1]);
            notify(All, EndMessage, Player2, loseMessage[Player2]);
            emit battleFinished(Win, id(Player1), id(Player2),rated(), tier());
        }
        throw QuitException();
    }

    int c1 = countAlive(Player1);
    int c2 = countAlive(Player2);

    if (c1*c2==0) {
        finished() = true;
        notify(All,ClockStop,Player1,time1);
        notify(All,ClockStop,Player2,time2);
        if (c1 + c2 == 0) {
            notify(All, BattleEnd, Player1, qint8(Tie));
            emit battleFinished(Tie, id(Player1), id(Player2),rated(), tier());
        } else if (c1 == 0) {
            notify(All, BattleEnd, Player2, qint8(Win));
            notify(All, EndMessage, Player2, winMessage[Player2]);
            notify(All, EndMessage, Player1, loseMessage[Player1]);
            emit battleFinished(Win, id(Player2), id(Player1),rated(), tier());
        } else {
            notify(All, BattleEnd, Player1, qint8(Win));
            notify(All, EndMessage, Player1, winMessage[Player1]);
            notify(All, EndMessage, Player2, loseMessage[Player2]);
            emit battleFinished(Win, id(Player1), id(Player2),rated(), tier());
        }
        /* The battle is finished so we stop the battling thread */
        sem.acquire(1);
        throw QuitException();
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
    int ret = pokelong[player][q].toInt()*getStatBoost(player, stat)*(20+turnlong[player][q+"AbilityModifier"].toInt())/20;

    if (doubles()) {
        ret = ret * (20+turnlong[player][q+"PartnerAbilityModifier"].toInt())/20;
    }

     ret = ret * (20+turnlong[player][q+"ItemModifier"].toInt())/20;

    if (stat == Speed && teamzone[this->player(player)].value("TailWindCount").toInt() > 0){
        ret *= 2;
    }

    //QuickFeet
    if (stat == Speed && poke(player).status() == Pokemon::Paralysed && !hasWorkingAbility(player, Ability::QuickFeet)) {
	ret /= 4;
    }

    if (stat == SpDefense && isWeatherWorking(SandStorm) && hasType(player,Pokemon::Rock))
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
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe), qint16(berry), qint8(stat));
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

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = pokelong[player][QString("Boost%1").arg(stat)].toInt();

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
        if (attacker != player && attacked == player && hasWorkingAbility(attacker, Ability::Unaware) && (stat == SpDefense || stat == Defense)) {
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
    } else if (stat == 7) {
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

    return ret;
}

void BattleSituation::emitCommand(int slot, int players, const QByteArray &toSend)
{
    if (players == All) {
        emit battleInfo(publicId(), qint32(id(Player1)), toSend);
        emit battleInfo(publicId(), qint32(id(Player2)), toSend);

        foreach(int id, spectators) {
            emit battleInfo(publicId(), qint32(id), toSend);
        }
    } else if (players == AllButPlayer) {
        emit battleInfo(publicId(), qint32(id(opponent(player(slot)))), toSend);

        foreach(int id, spectators) {
            emit battleInfo(publicId(), qint32(id), toSend);
        }
    } else {
        emit battleInfo(publicId(), qint32(id(players)), toSend);
    }
}

BattleDynamicInfo BattleSituation::constructInfo(int slot)
{
    BattleDynamicInfo ret;

    int player = this->player(slot);

    for (int i = 0; i < 7; i++) {
        ret.boosts[i] = pokelong[slot]["Boost" + QString::number(i+1)].toInt();
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

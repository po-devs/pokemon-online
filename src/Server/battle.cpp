#include "battle.h"
#include "player.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>

BattleSituation::BattleSituation(Player &p1, Player &p2, const ChallengeInfo &c)
	:team1(p1.team()), team2(p2.team())
{
    myid[0] = p1.id();
    myid[1] = p2.id();
    mycurrentpoke[0] = -1;
    mycurrentpoke[1] = -1;
    finished() = false;
    clauses() = c.clauses;
    currentForcedSleepPoke[0] = -1;
    currentForcedSleepPoke[1] = -1;
    p1.battle = this;
    p2.battle = this;
    if (clauses() & ChallengeInfo::ChallengeCup) {
        team1.generateRandom();
        team2.generateRandom();
    } else {
        if (clauses() & ChallengeInfo::LevelBalance) {
            for (int i = 0; i < 6; i++) {
                team1.poke(i).level() = PokemonInfo::LevelBalance(p1.team().poke(i).num());
                team1.poke(i).updateStats();;
            }
            for (int i = 0; i < 6; i++) {
                team2.poke(i).level() = PokemonInfo::LevelBalance(p2.team().poke(i).num());
                team2.poke(i).updateStats();;
            }
        }
    }
}

MirrorMoveAmn amn;

BattleSituation::~BattleSituation()
{
    /* releases the thread */
    {
        qDebug() << "Destruction of a battle.";
	/* So the thread will quit immediately after being released */
	quit = true;
        /* Should be enough */
        sem.release(1000);
	/* In the case the thread has not quited yet (anyway should quit in like 1 nano second) */
	wait();
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


    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        //False for saying this is not in battle message but rule message
        if (clauses() & (1 << i))
            notify(All, Clause, i, false);
    }


    notify(All, BlankMessage,0);

    /* Beginning of the battle! */
    sendPoke(Player1, 0);
    sendPoke(Player2, 0);
    callEntryEffects(Player1);
    callEntryEffects(Player2);

    hasChoice[0] = false;
    hasChoice[1] = false;
    turn() = 0;

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

bool BattleSituation::acceptSpectator(int id) const
{
    return !(clauses() & ChallengeInfo::DisallowSpectator) && !spectators.contains(spectatorKey(id)) && this->id(0) != id && this->id(1) != id;
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
        if (!koed(i)) {
            notify(key, SendOut, i, opoke(i, currentPoke(i)));
        }
    }
}

void BattleSituation::removeSpectator(int id)
{
    spectators.remove(spectatorKey(id));
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

int BattleSituation::rev(int p) const
{
    return 1 - p;
}

const PokeBattle & BattleSituation::poke(int player, int poke) const
{
    return team(player).poke(poke);
}

PokeBattle & BattleSituation::poke(int player, int poke)
{
    return team(player).poke(poke);
}

const PokeBattle &BattleSituation::poke(int player) const
{
    return team(player).poke(currentPoke(player));
}

PokeBattle &BattleSituation::poke(int player)
{
    return team(player).poke(currentPoke(player));
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
#else
# ifdef WIN64
    /* Under windows you need to do that, as rand is per-thread. But on linux it'd screw up the random thing */
    srand(time(NULL));
# endif
#endif
    try {
	while (!quit)
	{
	    beginTurn();

	    endTurn();
	}
    } catch(const QuitException &ex) {
	; /* the exception is just there to get immediately out of the while , nothing more
	   We could even have while (1) instead of while(!quit) (but we don't! ;) )*/
    }
}

void BattleSituation::beginTurn()
{
    turn() += 1;
    /* Resetting temporary variables */
    turnlong[0].clear();
    turnlong[1].clear();

    callpeffects(Player1, Player2, "TurnSettings");
    callpeffects(Player2, Player1, "TurnSettings");

    requestChoices();
    /* preventing the players from cancelling (like when u-turn/Baton pass) */
    hasChoice[0] = false;
    hasChoice[1] = false;
    analyzeChoices();
}

void BattleSituation::endTurn()
{
    callzeffects(Player1, Player1, "EndTurn");
    callzeffects(Player2, Player2, "EndTurn");

    endTurnWeather();

    callbeffects(Player1,Player1,"EndTurn");

    if (!koed(Player1))
	callieffects(Player1, Player1, "EndTurn");
    if (!koed(Player2))
	callieffects(Player2, Player2, "EndTurn");

    callaeffects(Player1,Player2,"EndTurn");
    callaeffects(Player2,Player1,"EndTurn");

    callpeffects(Player1, Player2, "EndTurn");
    callpeffects(Player2, Player1, "EndTurn");

    endTurnStatus();

    requestSwitchIns();
}

void BattleSituation::endTurnStatus()
{
    for (int player = Player1; player <= Player2; player++)
    {
	if (!koed(player))
	    switch(poke(player).status())
	    {
		case Pokemon::Burnt:
		    notify(All, StatusMessage, player, qint8(HurtBurn));
                    //HeatProof: burn does only 1/16
                    inflictDamage(player, poke(player).totalLifePoints()/(8*(1+hasWorkingAbility(player,32))), player);
		    break;
		case Pokemon::DeeplyPoisoned:
                    //Poison Heal
                    if (hasWorkingAbility(player, 68)) {
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
                    if (hasWorkingAbility(player, 68)) {
                        sendAbMessage(45,0,player,Pokemon::Poison);
                        healLife(player, poke(player).totalLifePoints()/8);
                    } else {
                        notify(All, StatusMessage, player, qint8(HurtPoison));
                        inflictDamage(player, poke(player).totalLifePoints()/8, player);
                    }
		    break;
	    }
    }
}

void BattleSituation::testquit()
{
    if (quit) {
        qDebug() << "Quit required in battle";
	throw QuitException();
    }
}

bool BattleSituation::requestChoice(int player, bool acquire, bool custom)
{
    if (turnlong[player].contains("NoChoice") && !koed(player)) {
	return false;
    }

    couldMove[player] = hasChoice[player] = true;

    if (!custom)
	options[player] = createChoice(player);

    notify(player, OfferChoice, player, options[player]);

    if (acquire) {
        sem.acquire(1); /* Lock until a choice is received */
    }

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
    return true;
}

void BattleSituation::requestChoices()
{
    couldMove[Player1] = couldMove[Player2] = false;

    /* Gets the number of choices to be done */
    int count = int(requestChoice(Player1, false)) + requestChoice(Player2, false);

    if (count > 0) {
        /* Send a brief update on the status */
        notifyInfos();
        /* Lock until BOTH choices are received */
        sem.acquire(1);
    }

    //test to see if the quit was requested by system or if choice was received
    testquit();

    notify(All, BeginTurn, All, turn());

    /* Now all the players gonna do is analyzeChoice(int player) */
}

void BattleSituation::notifyInfos()
{
    for (int p = Player1; p <= Player2; p++) {
        if (!koed(p)) {
            BattleStats stats = constructStats(p);
            notify(p, DynamicStats, p, stats);
            BattleDynamicInfo infos = constructInfo(p);
            notify(All, DynamicInfo, p, infos);
        }
    }
}

bool BattleSituation::koed(int player) const
{
    return currentPoke(player) == -1 || poke(player).lifePoints() == 0;
}

BattleChoices BattleSituation::createChoice(int player)
{
    /* First let's see for attacks... */
    if (koed(player)) {
	return BattleChoices::SwitchOnly();
    }

    BattleChoices ret;

    /* attacks ok, lets see which ones then */
    callpeffects(player, player, "MovesPossible");
    callieffects(player, player, "MovesPossible");
    callbeffects(player,player,"MovesPossible");

    for (int i = 0; i < 4; i++) {
	if (!isMovePossible(player,i)) {
	    ret.attackAllowed[i] = false;
	}
    }

    if (!hasWorkingItem(player, 30)) /* Shed Shell */
    {
	if (pokelong[player].contains("BlockedBy")) {
	    int b = pokelong[player]["BlockedBy"].toInt();
	    if (pokelong[b].contains("Blocked") && pokelong[b]["Blocked"].toInt() == player) {
		ret.switchAllowed = false;
	    }
	}

	if (pokelong[player].contains("Rooted")) {
	    ret.switchAllowed = false;
	}

	if (pokelong[player].contains("TrappedBy")) {
	    int b = pokelong[player]["TrappedBy"].toInt();
	    if (pokelong[b].contains("Trapped") && pokelong[b]["Trapped"].toInt() == player) {
		ret.switchAllowed = false;
	    }
	}

        if (!koed(rev(player))) {
            callaeffects(rev(player),player, "IsItTrapped");
            if (turnlong[player].value("Trapped").toBool()) {
                ret.switchAllowed = false;
            }
        }
    }

    return ret;
}

bool BattleSituation::isMovePossible(int player, int move)
{
    return (poke(player).move(move).PP() > 0 && (turnlong[player]["Move" + QString::number(move) + "Blocked"].toBool() == false));
}

void BattleSituation::analyzeChoice(int player)
{
    qDebug() << "Analyzing choice of " << id(player);
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice[player].attack()) {
        if (!koed(player) && !turnlong[player].value("HasMoved").toBool() && !turnlong[player].value("CantGetToMove").toBool()) {
	    if (turnlong[player].contains("NoChoice"))
                /* Do not use LastMoveSuccessfullyUsed or you'll have problems with metronome */
                useAttack(player, pokelong[player]["LastMoveUsed"].toInt(), true);
            else {
                if (options[player].struggle()) {
                    MoveEffect::setup(394,player,rev(player),*this);
                    useAttack(player, 394, true);
                } else {
                    useAttack(player, choice[player].numSwitch);
                }
            }
	}
    } else {
        if (!koed(player)) { /* if the pokemon isn't ko, it IS sent back */
	    sendBack(player);
	}
	sendPoke(player, choice[player].numSwitch);
    }
    notify(All, BlankMessage, Player1);
    qDebug() << "End of analyzing";
}

std::vector<int> BattleSituation::sortedBySpeed() {
    std::vector<int> ret;

    if (getStat(Player1, Speed) < getStat(Player2, Speed)) {
        ret.push_back(Player2);
        ret.push_back(Player1);
    } else if (getStat(Player2, Speed) < getStat(Player1, Speed)) {
        ret.push_back(Player1);
        ret.push_back(Player2);
    } else {
        if (true_rand() % 2 == 0) {
            ret.push_back(Player1);
            ret.push_back(Player2);
        } else {
            ret.push_back(Player2);
            ret.push_back(Player1);
        }
    }

    if (battlelong.value("TrickRoomCount").toInt() > 0) {
        std::swap(ret[0], ret[1]);
    }

    return ret;
}

void BattleSituation::analyzeChoices()
{
    qDebug() << "Starting analyzing choices";
    /* If there's no choice then the effects are already taken care of */
    if (!turnlong[Player1].contains("NoChoice") && choice[Player1].attack() && !options[Player1].struggle()) {
        qDebug() << "Player1 " << id(Player1) << " chose to attack with slot " << choice[Player1].numSwitch;
        MoveEffect::setup(move(Player1,choice[Player1].numSwitch), Player1, Player2, *this);
    }
    qDebug() << "Going to the choice of player 2";
    if (!turnlong[Player2].contains("NoChoice") && choice[Player2].attack() && !options[Player2].struggle()) {
        qDebug() << "Player2 " << id(Player2) << " chose to attack with slot " << choice[Player2].numSwitch;
	MoveEffect::setup(move(Player2,choice[Player2].numSwitch), Player2, Player1, *this);
    }
    qDebug() << "Finished basic setup";

    std::map<int, std::vector<int>, std::greater<int> > priorities;
    std::vector<int> switches;

    std::vector<int> playersByOrder = sortedBySpeed();

    qDebug() << "SortedBySpeed: " << id(playersByOrder[0]) << " " << id(playersByOrder[1]);

    foreach(int i, playersByOrder) {
	if (choice[i].poke())
            switches.push_back(i);
	else
            priorities[turnlong[i]["SpeedPriority"].toInt()].push_back(i);
    }

    qDebug() << "Number of switches: " << switches.size();

    foreach(int player, switches) {
	analyzeChoice(player);
    }
    foreach(int player, switches) {
	callEntryEffects(player);
    }

    qDebug() << "Finished the switches";

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;

    for (it = priorities.begin(); it != priorities.end(); ++it) {
	/* There's another priority system: Ability stall, and Item lagging tail */
        std::map<int, std::vector<int>, std::greater<int> > secondPriorities;

        foreach (int player, it->second) {
            qDebug() << "Stating calling TurnOrder for " << id(player) << "(" << player << ")" ;
            callaeffects(player,player, "TurnOrder"); //Stall
            callieffects(player,player, "TurnOrder"); //Lagging tail & ...
            calleffects(player,player,"TurnOrder"); // A berry does that
            qDebug() << "Turn order is " << turnlong[player]["TurnOrder"].toInt();
            secondPriorities[turnlong[player]["TurnOrder"].toInt()].push_back(player);
	}

        for(std::map<int, std::vector<int> >::iterator it = secondPriorities.begin(); it != secondPriorities.end(); ++it) {
            foreach(int p, it->second) {
                analyzeChoice(p);
            }
        }
    }
    qDebug() << "End of Analyzing choices";
}

void BattleSituation::notifySub(int player, bool sub)
{
    notify(All, Substitute, player, sub);
}

void BattleSituation::battleChoiceReceived(int id, const BattleChoice &b)
{
    int player = spot(id);

    if (hasChoice[player] == false) {
        /* If at least one of the two player still hasn't moved, and the cancel is valid, we allow the cancel */
        if (b.cancelled() && couldMove[player] && (hasChoice[player] || hasChoice[rev(player)])) {
            hasChoice[player] = true;
            notify(player, CancelMove, player);
        } else {
            //INVALID BEHAVIOR
        }
    } else {
	if (!b.match(options[player])) {
	    //INVALID BEHAVIOR
	} else {
	    /* Routine checks */
	    if (b.poke()) {
		if (b.numSwitch == currentPoke(player) || poke(player, b.numSwitch).num() == 0 || poke(player, b.numSwitch).ko()) {
		    // INVALID BEHAVIOR
		    return;
		}
	    }
	    /* One player has chosen their solution, so there's one less wait */
	    choice[player] = b;
            hasChoice[player] = false;
            /* If everyone has chosen their solution, we carry on */
            if (!hasChoice[player] && !hasChoice[rev(player)]) {
                sem.release(1);
            }
	}
    }
}

void BattleSituation::battleChat(int id, const QString &str)
{
    notify(All, BattleChat, spot(id), str);
}

void BattleSituation::spectatingChat(int id, const QString &str)
{
    notify(All, SpectatorChat, id, qint32(id), str);
}


/* Battle functions! Yeah! */

void BattleSituation::sendPoke(int player, int pok)
{
    koedPokes.remove(player);
    changeCurrentPoke(player, pok);

    notify(player, SendOut, player, ypoke(player, pok));
    notify(AllButPlayer, SendOut, player, opoke(player, pok));

    /* reset temporary variables */
    pokelong[player].clear();
    /* Give new values to what needed */
    pokelong[player]["Num"] = poke(player).num();
    pokelong[player]["Weight"] = PokemonInfo::Weight(poke(player).num());
    pokelong[player]["Type1"] = PokemonInfo::Type1(poke(player).num());
    pokelong[player]["Type2"] = PokemonInfo::Type2(poke(player).num());
    pokelong[player]["Ability"] = poke(player).ability();
    for (int i = 0; i < 4; i++) {
	pokelong[player]["Move" + QString::number(i)] = poke(player).move(i).num();
    }

    for (int i = 1; i <= 6; i++)
	pokelong[player][QString("Stat%1").arg(i)] = poke(player).normalStat(i);
    pokelong[player]["Level"] = poke(player).level();
    turnlong[player]["CantGetToMove"] = true;
}

void BattleSituation::callEntryEffects(int player)
{
    ItemEffect::setup(poke(player).item(),player,*this);
    acquireAbility(player, poke(player).ability());
    calleffects(player, player, "UponSwitchIn");
    callzeffects(player, player, "UponSwitchIn");
    calleffects(player, player, "AfterSwitchIn");
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

void BattleSituation::callieffects(int source, int target, const QString &name)
{
    //Klutz
    if (!pokelong[source].value("Embargoed").toBool() && !hasWorkingAbility(source, 46))
	ItemEffect::activate(name, poke(source).item(), source, target, *this);
}

void BattleSituation::callaeffects(int source, int target, const QString &name)
{
    if (hasWorkingAbility(source, ability(source)))
        AbilityEffect::activate(name, ability(source), source, target, *this);
}

void BattleSituation::sendBack(int player)
{
    notify(All, SendBack, player);

    /* Just calling pursuit directly here, forgive me for this */
    int opp = rev(player);
    if (!koed(opp) && turnlong[opp].value("Attack").toInt() == 305 && !turnlong[opp]["HasMoved"].toBool()) {
	turnlong[opp]["Power"] = turnlong[opp]["Power"].toInt() * 2;
	analyzeChoice(opp);

	if (koed(player)) {
	    Mechanics::removeFunction(turnlong[player],"UponSwitchIn","BatonPass");
	}
    }

    if (!koed(player)) {
        callaeffects(player,player,"UponSwitchOut");
	changeCurrentPoke(player, -1);
    }
}

bool BattleSituation::testAccuracy(int player, int target)
{
    int acc = turnlong[player]["Accuracy"].toInt();

    if (acc == 0) {
	return true;
    }

    if (pokelong[target].value("LockedOnEnd").toInt() >= turn() && pokelong[player].contains("LockedOn") && pokelong[player].value("LockedOn") == target) {
	return true;
    }

    if (pokelong[player].value("BerryLock").toBool()) {
        pokelong[player].remove("BerryLock");
        return true;
    }

    //OHKO
    if (MoveInfo::isOHKO(turnlong[player]["MoveChosen"].toInt())) {
        bool ret = (true_rand() % 100) < 30;
        if (!ret) {
            notify(All, Miss, player);
        }
        return ret;
    }

    //No Guard
    if (hasWorkingAbility(player, 61) || hasWorkingAbility(target, 61)) {
        return true;
    }

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

    if (true_rand() % 100 < acc) {
	return true;
    } else {
	notify(All, Miss, player);
	calleffects(player,target,"MissAttack");
	return false;
    }
}

void BattleSituation::testCritical(int player, int target)
{
    /* Shell armor, Battle Armor */
    if (hasWorkingAbility(target, 8) || hasWorkingAbility(target, 85) || teamzone[target].value("LuckyChantCount").toInt() > 0) {
	return;
    }

    int randnum = true_rand() % 48;
    int minch;
    int craise = turnlong[player]["CriticalRaise"].toInt();

    if (hasWorkingAbility(player, 105)) { /* Super Luck */
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
                poke(player).sleepCount() -= 1 + hasWorkingAbility(player, 21);
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
            if (!hasWorkingAbility(player, 52) && true_rand() % 4 == 0) {
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
        if (hasWorkingAbility(player, 99)) {
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
    if (hasWorkingAbility(target, 41)) {
        return;
    }

    int rate = turnlong[player]["FlinchRate"].toInt();
    int randnum = true_rand() % 100;
    /* Serene Grace */
    if (hasWorkingAbility(player, 82)) {
        randnum /= 2;
    }

    if (randnum % 100 < rate) {
	turnlong[target]["Flinched"] = true;
    }

    if (hasWorkingItem(player, 87) && turnlong[player]["KingRock"].toBool()) /* King's rock */
    {
        if (true_rand() % 100 < 10) {
	    turnlong[target]["Flinched"] = true;
	}
    }
}

bool BattleSituation::testFail(int player)
{
    if (turnlong[player]["Failed"].toBool() == true) {
	if (turnlong[player]["FailingMessage"].toBool() == true) {
	    notify(All, Failed, player);
	}
	return true;
    }
    return false;
}

int BattleSituation::attacker() {
    if (!battlelong.contains("Attacker")) {
        return -1;
    } else {
        return battlelong["Attacker"].toInt();
    }
}

int BattleSituation::attacked() {
    if (!battlelong.contains("Attacked")) {
        return -1;
    } else {
        return battlelong["Attacked"].toInt();
    }
}

void BattleSituation::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    battlelong["Attacker"] = player;

    int attack;
    QList<int> targetList;

    if (specialOccurence) {
	attack = move;
    } else {
	attack = this->move(player,move);
	pokelong[player]["MoveSlot"] = move;
	pokelong[player][QString("Move%1Used").arg(move)] = true;
    }

    qDebug() << "move " << MoveInfo::Name(attack) << " used by " << id(player);

    turnlong[player]["HasMoved"] = true;

    calleffects(player,player,"EvenWhenCantMove");

    if (!testStatus(player)) {
        goto end;
    }

    turnlong[player]["HasPassedStatus"] = true;

    //Just for truant
    callaeffects(player, player, "DetermineAttackPossible");
    if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
        goto end;
    }

    callpeffects(player, player, "DetermineAttackPossible");
    if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
        goto end;
    }

    turnlong[player]["MoveChosen"] = attack;

    callbeffects(player,player,"MovePossible");
    if (turnlong[player]["ImpossibleToMove"].toBool()) {
        goto end;
    }

    if (!specialOccurence) {
	calleffects(player, player, "MovePossible");
	if (turnlong[player]["ImpossibleToMove"].toBool()) {
            goto end;
	}
	if (!isMovePossible(player, move)) {
            goto end;
	}
	callieffects(player,player, "RegMoveSettings");
    }

    if (attack != 394) { //Struggle
        pokelong[player]["LastMoveUsed"] = attack;
        inc(pokelong[player]["MovesUsed"]);
    }

    calleffects(player, player, "MoveSettings");

    if (tellPlayers && !turnlong[player].contains("TellPlayers")) {
        notify(All, UseAttack, player, qint16(attack));
    }

    if (!specialOccurence && !turnlong[player].contains("NoChoice")) {
        //Pressure
        losePP(player, move, 1 + (hasWorkingAbility(rev(player), 70) && !koed(rev(player)) && turnlong[player]["Power"].toInt() > 0));
    }
    
    switch(turnlong[player]["PossibleTargets"].toInt()) {
	case Move::None: targetList.push_back(player); break;
	case Move::User: targetList.push_back(player); break;
	case Move::All: targetList.push_back(player); targetList.push_back(rev(player)); break;
	default: targetList.push_back(rev(player));
    }

    if (targetList.size() == 1 && targetList[0] == rev(player) && koed(rev(player))) {
	notify(All, NoOpponent, player);
        goto end;
    }

    callaeffects(player,player, "BeforeTargetList");
    callieffects(player, player, "BeforeTargetList");
    calleffects(player, player, "BeforeTargetList");

    foreach(int target, targetList) {
        battlelong["Attacked"] = target;
	if (player != target && !specialOccurence && !amn.contains(attack)) {
	    pokelong[target]["MirrorMoveMemory"] = attack;
	}

	turnlong[player]["Failed"] = false;
	turnlong[player]["FailingMessage"] = true;
	if (target != -1 && koed(target)) {
	    continue;
	}
	if (target != player && !testAccuracy(player, target)) {
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
            continue;
        }
	if (turnlong[player]["Power"].toInt() > 0)
        {
	    int type = turnlong[player]["Type"].toInt(); /* move type */
	    int typeadv[] = {getType(target, 1), getType(target, 2)};
	    int typepok[] = {getType(player, 1), getType(player, 2)};
	    int typeffs[] = {TypeInfo::Eff(type, typeadv[0]),TypeInfo::Eff(type, typeadv[1])};
	    int typemod = 1;
	    bool fly = type == Move::Ground && !isFlying(target);

	    for (int i = 0; i < 2; i++) {
                if (typeffs[i] != 0 || ((!fly || typepok[i] != Pokemon::Flying) && !pokelong[target].value(QString::number(typeadv[i])+"Sleuthed").toBool()
                    && (!hasType(target, Pokemon::Ghost) || !hasWorkingAbility(player,82)))) //Scrappy
		    typemod *= typeffs[i];
	    }

	    if (type == Move::Ground && isFlying(target)) {
		typemod = 0;
	    }

	    int stab = 2 + (type==typepok[0] || type==typepok[1]);

	    turnlong[player]["Stab"] = stab;
	    turnlong[player]["TypeMod"] = typemod; /* is attack effective? or not? etc. */

	    calleffects(player, target, "BeforeCalculatingDamage");

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

	    if (target != player && !testAccuracy(player, target)) {
		calleffects(player,target,"AttackSomehowFailed");
		continue;
	    }

	    calleffects(player, target, "BeforeHitting");

            int num = repeatNum(player, turnlong[player]);
	    bool hit = num > 1;

	    for (int i = 0; i < num && !koed(target); i++) {

		if (hit) {
		    notify(All, Hit, target);
		}

                turnlong[target]["HadSubstitute"] = false;
		bool sub = hasSubstitute(target);
                turnlong[target]["HadSubstitute"] = sub;

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
		if (!specialOccurence && attack != 247 && attack != 358 && attack != 394 && attack != 432) {
		    pokelong[player]["LastMoveSuccessfullyUsed"] = attack;
		    pokelong[player]["LastMoveSuccessfullyUsedTurn"] = turn();
		}

		if (!sub && !koed(target))
		    testFlinch(player, target);
		/* Removing substitute... */
		turnlong[player]["HadSubstitute"] = false;
	    }

	    if (turnlong[player]["Power"].toInt() > 1)
		notify(All, Effective, target, quint8(typemod));

	    if (!koed(player))
		calleffects(player, target, "AfterAttackSuccessful");
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

	    calleffects(player, target, "BeforeHitting");

	    applyMoveStatMods(player, target);
	    calleffects(player, target, "UponAttackSuccessful");

	    battlelong["LastMoveSuccesfullyUsed"] = attack;

	    /* this is put after calleffects to avoid endless sleepTalk/copycat for example */
	    if (!specialOccurence && attack != 247 && attack != 358 && attack != 394 && attack != 432) {
		    pokelong[player]["LastMoveSuccessfullyUsed"] = attack;
		    pokelong[player]["LastMoveSuccessfullyUsedTurn"] = turn();
	    }
	    if (!koed(player))
		calleffects(player, target, "AfterAttackSuccessful");
	}
	if (turnlong[player]["Type"].toInt() == Move::Fire && poke(target).status() == Pokemon::Frozen) {
	    notify(All, StatusMessage, target, qint8(FreeFrozen));
	    healStatus(target, Pokemon::Frozen);
	}
	pokelong[target]["LastAttackToHit"] = attack;
    }

    end:
    battlelong.remove("Attacker");
    battlelong.remove("Attacked");
}

bool BattleSituation::hasMove(int player, int move) {
    for (int i = 0; i < 4; i++) {
	if (this->move(player, i) == move) {
	    return true;
	}
    }
    return false;
}

bool BattleSituation::hasWorkingAbility(int player, int ab)
{
    if (battlelong.contains("Attacker")) {
        int attacker = battlelong["Attacker"].toInt();
        //Mold Breaker
        if (attacker == rev(player) && hasWorkingAbility(attacker, 57)) {
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

bool BattleSituation::hasWorkingItem(int player, int it)
{
    //Klutz
    return poke(player).item() == it && !pokelong[player].value("Embargoed").toBool() && !hasWorkingAbility(player, 46);
}

int BattleSituation::move(int player, int slot)
{
    return pokelong[player]["Move"+QString::number(slot)].toInt();
}

void BattleSituation::inflictRecoil(int source, int target)
{
    //Rockhead
    if (koed(source) || hasWorkingAbility(source,76))
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
    if (hasWorkingAbility(player,82)) {
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
	    sendMoveMessage(128, 2, player,0,target,turnlong[player]["Attack"].toInt());
	    continue;
	}

        //Shield Dust
        if (!self && hasWorkingAbility(targeted, 86)) {
            continue;
        }

	/* There maybe different type of changes, aka status & mod in move 'Flatter' */
        QStringList changes = effect.mid(1).split('/');
	
	foreach (QString effect, changes)
	{
	    /* Now the kind of change itself */
            bool statusChange = effect.midRef(0,3) == "[S]"; /* otherwise it's stat change */

	    if(!self && !statusChange && teamzone[targeted].value("MistCount").toInt() > 0) {
		sendMoveMessage(86, 2, player,Pokemon::Ice,target,turnlong[player]["Attack"].toInt());
		continue;
	    }

	    if(!self && statusChange && teamzone[targeted].value("SafeGuardCount").toInt() > 0) {
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

void BattleSituation::inflictConfused(int player)
{
    //OwnTempo
    if (!pokelong[player]["Confused"].toBool() && !hasWorkingAbility(player,65)) {
	pokelong[player]["Confused"] = true;
        pokelong[player]["ConfusedCount"] = (true_rand() % 4) + 1;
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
    case Pokemon::Paralysed: return !hasWorkingAbility(player, 50);
    case Pokemon::Asleep: return !hasWorkingAbility(player, 42) && !hasWorkingAbility(player, 118);
    case Pokemon::Burnt: return !hasType(player, Pokemon::Fire) && !hasWorkingAbility(player, 121);
    case Pokemon::DeeplyPoisoned:
    case Pokemon::Poisoned: return !hasType(player, Pokemon::Poison) && !hasType(player, Pokemon::Steel) && !hasWorkingAbility(player, 40);
    case Pokemon::Frozen: return !isWeatherWorking(Sunny) && !hasType(player, Pokemon::Ice) && !hasWorkingAbility(player, 53);
    default:
        return false;
    }
}

void BattleSituation::inflictStatus(int player, int status, int attacker)
{
    if (poke(player).status() != Pokemon::Fine) {
        if (this->attacker() == attacker && turnlong[attacker]["Power"].toInt() == 0)
            notify(All, AlreadyStatusMessage, player);
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
                if (sleepClause() && currentForcedSleepPoke[player] != -1) {
                    notifyClause(ChallengeInfo::SleepClause, true);
                    return;
                } else {
                    currentForcedSleepPoke[player] = currentPoke(player);
                }
            } else if (status == Pokemon::Frozen)
            {
                if (clauses() & ChallengeInfo::FreezeClause) {
                    for (int i = 0; i < 6; i++) {
                        if (poke(player,i).status() == Pokemon::Frozen) {
                            notifyClause(ChallengeInfo::FreezeClause);
                            return;
                        }
                    }
                }
            }
        }
        changeStatus(player, status);
        if (attacker != player && status != Pokemon::Asleep && status != Pokemon::Frozen && poke(attacker).status() == Pokemon::Fine && canGetStatus(attacker,status)
            && hasWorkingAbility(player, 108)) //Synchronize
        {
            sendAbMessage(61,0,player,attacker);
            inflictStatus(attacker,status, player);
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
	notify(All, WeatherMessage, Player1, qint8(ContinueWeather), qint8(weather));
    } else {
	notify(All, WeatherMessage, Player1, qint8(StartWeather), qint8(weather));
	battlelong["WeatherCount"] = turns;
	battlelong["Weather"] = weather;
        for (int i = Player1; i <= Player2; i++) {
            if (!koed(i)) {
                callaeffects(i,i,"WeatherChange");
            }
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
	    for (int i = Player1; i <= Player2; i++) {
		if (koed(i))
		    continue;
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
    //Air lock & Cloud nine
    if (hasWorkingAbility(Player1, 3) || hasWorkingAbility(Player1, 12) || hasWorkingAbility(Player2, 3) || hasWorkingAbility(Player2, 12)) {
        return false;
    }
    return this->weather() == weather;
}

bool BattleSituation::isSeductionPossible(int seductor, int naiveone) {
    //Oblivious
    return !hasWorkingAbility(naiveone,63) && poke(seductor).gender() != Pokemon::Neutral && poke(naiveone).gender() != Pokemon::Neutral && poke(seductor).gender() != poke(naiveone).gender();
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
        if (hasWorkingAbility(player, 59)) {
	    types[0] = pokelong[player]["ItemArg"].toInt();
	    types[1] = Pokemon::Curse;
	}
    }

    if (types[slot-1] == Pokemon::Flying && turnlong[player].value("Roosted").toBool())
    {
	return Pokemon::Curse;
    }

    return types[slot-1];
}

bool BattleSituation::isFlying(int player)
{
    /* Item 212 is iron ball, ability is levitate */
    return !battlelong.value("Gravity").toBool() && !hasWorkingItem(player, 212) && !pokelong[player].value("Rooted").toBool() &&
	    (hasWorkingAbility(player, 48) ||  hasType(player, Pokemon::Flying) || pokelong[player].value("MagnetRiseCount").toInt() > 0);
}

bool BattleSituation::hasSubstitute(int player)
{
    return !koed(player) && (pokelong[player].value("Substitute").toBool() || turnlong[player].value("HadSubstitute").toBool());
}

void BattleSituation::changeStatus(int player, int status)
{
    if (poke(player).status() == status) {
	return;
    }

    //Sleep clause
    if (status != Pokemon::Asleep && currentForcedSleepPoke[player] == currentPoke(player)) {
        currentForcedSleepPoke[player] = -1;
    }

    notify(All, StatusChange, player, qint8(status));
    notify(All, AbsStatusChange, player, qint8(currentPoke(player)), qint8(status));
    poke(player).status() = status;
    if (status == Pokemon::Asleep) {
        poke(player).sleepCount() = (true_rand() % 5) +1;
    }
    if (status == Pokemon::DeeplyPoisoned) {
	pokelong[player]["ToxicCount"] = 0;
    }
    callpeffects(player, player,"AfterStatusChange");
    callieffects(player, player,"AfterStatusChange");
}

void BattleSituation::changeStatus(int team, int poke, int status)
{
    if (poke == currentPoke(team)) {
	changeStatus(team, status);
    } else {
	this->poke(team, poke).status() = status;
        notify(All, AbsStatusChange, team, qint8(poke), qint8(status));
        //Sleep clause
        if (status != Pokemon::Asleep && currentForcedSleepPoke[team] == poke) {
            currentForcedSleepPoke[team] = -1;
        }
    }
}

void BattleSituation::gainStatMod(int player, int stat, int bonus)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost < 6) {
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
            turnlong[player]["StatModded"] = Attack;
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
    return !turnlong[defender].contains(QString("StatModFrom%1DPrevented").arg(attacker)) &&
            turnlong[attacker]["Power"].toInt() == 0;
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

    if (attackused == 119 || attackused == 346) /* explosion / selfdestruct */
	def/=2;

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = true_rand() % (255-217) + 217;
    int ch = 1 + (crit * (1+hasWorkingAbility(p,90))); //Sniper
    int power = move["Power"].toInt();
    int type = move["Type"].toInt();

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
                       (poke.status() == Pokemon::Burnt && move["Category"].toInt() == Move::Physical && !hasWorkingAbility(p,31))
                       ? PokeFraction(1,2) : PokeFraction(1,1)
                       );
    /* Light screen / Reflect */
    if (!crit && teamzone[t].value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
	damage /= 2;
    }
    if (isWeatherWorking(Sunny)) {
	if (type == Move::Fire) {
	    damage = damage * 3 /2;
	} else if (type == Move::Water) {
	    damage /= 2;
	}
    } else if (isWeatherWorking(Move::Water)) {
	if (type == Move::Water) {
	    damage = damage * 3/2;
	} else if (type == Move::Fire) {
	    damage /= 2;
	}
    }
    //FlashFire
    if (type == Move::Fire && pokelong[p].contains("FlashFired") && hasWorkingAbility(p, 25)) {
        damage = damage * 3 / 2;
    }
    damage = (damage+2)*ch;
    callieffects(p,t,"Mod2Modifier");
    damage = damage*(10+move["ItemMod2Modifier"].toInt())/10/*Mod2*/;
    damage = damage *randnum*100/255/100*stab/2*typemod/4;

    /* Mod 3 */
    // FILTER / SOLID ROCK
    if (typemod > 4 && (hasWorkingAbility(t,23) || hasWorkingAbility(t,94))) {
        damage = damage * 3 / 4;
    }

    /* Expert belt */
    damage = damage * ((typemod > 4 && hasWorkingItem(p, 8))? 6 : 5)/5;

    /* Berries of the foe */
    callieffects(t, p, "Mod3Items");

    damage = damage * (10 + turnlong[p].value("Mod3Berry").toInt())/ 10;

    return damage;
}

int BattleSituation::repeatNum(int player, context &move)
{
    int min = 1+move["RepeatMin"].toInt();
    int max = 1+move["RepeatMax"].toInt();

    //Skill link
    if (hasWorkingAbility(player, 88)) {
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
    if (hasWorkingAbility(player, 52) && !straightattack) {
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

	if (hp <= 0 && ((straightattack && turnlong[player].contains("CannotBeKoedBy") && turnlong[player]["CannotBeKoedBy"].toInt() == source)
											  || turnlong[player].value("CannotBeKoed").toBool())) {
	    damage = poke(player).lifePoints() - 1;
	    hp = 1;
	}

	if (straightattack) {
	    notify(player, StraightDamage,player, qint16(damage));
	    notify(AllButPlayer, StraightDamage,player, qint16(damage*100/poke(player).totalLifePoints()));
	}

	if (hp <= 0) {
	    koPoke(player, source, straightattack);
	} else {
	    changeHp(player, hp);
	}
    }


    if (straightattack) {
	turnlong[source]["DamageInflicted"] = damage;
	if (!sub) {
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
    changePP(player,slot,std::min(5, int(poke(player).move(slot).PP())));
    notify(player, ChangeTempPoke, player, quint8(TempMove), quint8(slot), quint16(move));
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
    poke(player).item() = 0;
}

void BattleSituation::eatBerry(int player) {
    sendItemMessage(8000,player,0, 0, poke(player).item());
    disposeItem(player);
}

void BattleSituation::acqItem(int player, int item) {
    poke(player).item() = item;
    ItemEffect::setup(poke(player).item(),player,*this);
    callieffects(player,player,"AfterSetup");
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

    notify(player, ChangeHp, player, quint16(newHp));
    notify(AllButPlayer, ChangeHp, player, quint16(newHp == 0 ? 0 : std::max(int(poke(player).lifePercent()),1))); /* percentage calculus */

    callieffects(player, player, "AfterHPChange");
}

void BattleSituation::koPoke(int player, int source, bool straightattack)
{
    qDebug() << "koed poke from player: " << id(player);
    qDebug() << "Poke name: " << poke(player).nick() << " " << currentPoke(player);

    if (poke(player).ko()) {
	return;
    }

    changeHp(player, 0);
    changeStatus(player,Pokemon::Koed);

    notify(All, Ko, player);
    koedPokes.insert(player);

    if (straightattack && player!=source) {
	callpeffects(player, source, "AfterKoedByStraightAttack");
    }
}

void BattleSituation::requestSwitchIns()
{
    testWin();

    int count = koedPokes.size();

    if (count == 0) {
        return;
    }

    qDebug() << "Request a switch in : " << count;

    notifyInfos();

    foreach(int p, koedPokes) {
        requestChoice(p, false);
    }

    sem.acquire(1);

    testquit();

    QSet<int> copy = koedPokes;

    foreach(int p, copy) {
        analyzeChoice(p);
    }

    foreach(int p, copy) {
	callEntryEffects(p);
    }

    qDebug() << "Recursive call to requestSwitchIn";
    /* Recursive call */
    requestSwitchIns();
}

void BattleSituation::requestSwitch(int player)
{
    testWin();

    if (countAlive(player) - !koed(player) == 0) {
        //No poke to switch in, so we won't request a choice & such;
        return;
    }

    notifyInfos();

    options[player] = BattleChoices::SwitchOnly();

    requestChoice(player,true,true);
    analyzeChoice(player);
    callEntryEffects(player);
}

int BattleSituation::countAlive(int player) const
{
    int count = 0;
    for (int i = 0; i < 6; i++) {
	if (poke(player, i).num() != 0 && !poke(player, i).ko()) {
	    count += 1;
	}
    }
    return count;
}

void BattleSituation::testWin()
{
    int c1 = countAlive(Player1);
    int c2 = countAlive(Player2);

    if (c1*c2==0) {
        finished() = true;
        if (c1 + c2 == 0) {
            notify(All, BattleEnd, Player1, qint8(Tie));
            emit battleFinished(Tie, id(Player1), id(Player2));
        } else if (c1 == 0) {
            notify(All, BattleEnd, Player2, qint8(Win));
            emit battleFinished(Win, id(Player2), id(Player1));
        } else {
            notify(All, BattleEnd, Player1, qint8(Win));
            emit battleFinished(Win, id(Player1), id(Player2));
        }
        /* The battle is finished so we stop the thread */
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

    notify(player, ChangePP, player, quint8(move), poke(player).move(move).PP());
}

void BattleSituation::losePP(int player, int move, int loss)
{
    int PP = poke(player).move(move).PP();

    PP = std::max(PP-loss, 0);
    changePP(player, move, PP);

    callieffects(player, player, "AfterPPLoss");
}

void BattleSituation::gainPP(int player, int move, int gain)
{
    int PP = poke(player).move(move).PP();

    PP = std::min(PP+gain, int(poke(player).move(move).totalPP()));
    changePP(player, move, PP);

    notify(player, ChangePP, player, quint8(move), poke(player).move(move).PP());
}

int BattleSituation::getStat(int player, int stat)
{
    QString q = "Stat"+QString::number(stat);
    turnlong[player].remove(q+"AbilityModifier");
    turnlong[player].remove(q+"ItemModifier");
    callieffects(player, player, "StatModifier");
    callaeffects(player, player, "StatModifier");
    int ret = pokelong[player][q].toInt()*getStatBoost(player, stat)*(20+turnlong[player][q+"AbilityModifier"].toInt())/20
              *(20+turnlong[player][q+"ItemModifier"].toInt())/20;

    if (stat == Speed && teamzone[player].value("TailWindCount").toInt() > 0){
        ret *= 2;
    }

    //QuickFeet
    if (stat == Speed && poke(player).status() == Pokemon::Paralysed && !hasWorkingAbility(player, 72)) {
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



void BattleSituation::sendItemMessage(int move, int src, int part, int foe, int berry)
{
    if (foe ==-1)
	notify(All, ItemMessage, src, quint16(move), uchar(part));
    else if (berry == -1)
	notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe));
    else
        notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe), qint16(berry));
}

void BattleSituation::sendBerryMessage(int move, int src, int part, int foe, int berry)
{
    sendItemMessage(move+8000,src,part,foe,berry);
}


void BattleSituation::fail(int player, int move, int part, int type)
{
    turnlong[player]["FailingMessage"] = false;
    turnlong[player]["Failed"] = true;
    sendMoveMessage(move, part, player, type, rev(player),turnlong[player]["MoveChosen"].toInt());
}

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = pokelong[player][QString("Boost%1").arg(stat)].toInt();

    //simple
    if (hasWorkingAbility(player,87)) {
        boost *= 2;
    }

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */
    int attacker = this->attacker();
    int attacked = this->attacked();

    if (attacker != -1 && attacked != -1) {
        //Unaware
        if (attacker != player && attacked == player && hasWorkingAbility(attacker, 116) && (stat == SpDefense || stat == Defense)) {
            boost = 0;
        } else if (attacker == player && attacked != player && hasWorkingAbility(attacked, 116) && (stat == SpAttack || stat == Attack)) {
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

void BattleSituation::emitCommand(int player, int players, const QByteArray &toSend)
{
    if (players == All) {
        emit battleInfo(qint32(id(Player1)), toSend);
        emit battleInfo(qint32(id(Player2)), toSend);

        foreach(int id, spectators) {
            emit battleInfo(qint32(id), toSend);
        }
    } else if (players == AllButPlayer) {
        emit battleInfo(qint32(id(rev(player))), toSend);

        foreach(int id, spectators) {
            emit battleInfo(qint32(id), toSend);
        }
    } else {
        emit battleInfo(qint32(id(players)), toSend);
    }
}

BattleDynamicInfo BattleSituation::constructInfo(int player)
{
    BattleDynamicInfo ret;

    for (int i = 0; i < 7; i++) {
        ret.boosts[i] = pokelong[player]["Boost" + QString::number(i+1)].toInt();
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
    if (teamzone[player].contains("StealthRock")) {
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

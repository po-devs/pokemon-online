#include "battle.h"
#include "player.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"
#include "items.h"
#include <ctime> /* for random numbers, time(NULL) needed */
#include <map>
#include <algorithm>

BattleSituation::BattleSituation(Player &p1, Player &p2)
	:team1(p1.team()), team2(p2.team())
{
    myid[0] = p1.id();
    myid[1] = p2.id();
    mycurrentpoke[0] = -1;
    mycurrentpoke[1] = -1;
}

BattleSituation::~BattleSituation()
{
    /* releases the thread */
    {
	/* So the thread will quit immediately after being released */
	quit = true;
	/* Should be enough lol */
	sem.release(1000);
	/* In the case the thread has not quited yet (anyway should quit in like 1 nano second) */
	wait();
    }
}

void BattleSituation::start()
{
    quit = false; /* doin' that cuz if any battle command is called why quit is set to true disasters happen */

    /* Beginning of the battle! */
    sendPoke(Player1, 0);
    sendPoke(Player2, 0);
    callEntryEffects(Player1);
    callEntryEffects(Player2);

    haveChoice[0] = false;
    haveChoice[1] = false;
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

int BattleSituation::id(int spot) const
{
    return myid[spot];
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
    analyzeChoices();
}

void BattleSituation::endTurn()
{
    qDebug() << "Start of the end of the turn";

    if (!koed(Player1))
	callieffects(Player1, Player1, "EndTurn");
    if (!koed(Player2))
	callieffects(Player2, Player2, "EndTurn");

    callpeffects(Player1, Player2, "EndTurn");
    callpeffects(Player2, Player1, "EndTurn");

    endTurnStatus();

    requestSwitchIns();
    qDebug() << "End of the turn";
}

void BattleSituation::endTurnStatus()
{
    for (int player = Player1; player <= Player2; player++)
    {
	switch(poke(player).status())
	{
	    case Pokemon::Burnt:
		notify(All, StatusMessage, player, qint8(HurtBurn));
		inflictDamage(player, poke(player).totalLifePoints()/8, player);
		break;
	    case Pokemon::DeeplyPoisoned:
		notify(All, StatusMessage, player, qint8(HurtPoison));
		inflictDamage(player, poke(player).totalLifePoints()*(pokelong[player]["ToxicCount"].toInt()+1)/16, player);
		pokelong[player]["ToxicCount"] = std::min(pokelong[player]["ToxicCount"].toInt()+1, 14);
		break;
	    case Pokemon::Poisoned:
		notify(All, StatusMessage, player, qint8(HurtPoison));
		inflictDamage(player, poke(player).totalLifePoints()/8, player);
		break;
	}
    }
}

void BattleSituation::testquit()
{
    if (quit)
	throw QuitException();
}

bool BattleSituation::requestChoice(int player, bool acquire, bool custom)
{
    if (turnlong[player].contains("NoChoice") && !koed(player)) {
	return false;
    }

    haveChoice[player] = true;
    if (!custom)
	options[player] = createChoice(player);

    notify(player, OfferChoice, player, options[player]);

    if (acquire)
	sem.acquire(1); /* Lock until a choice is received */

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
    return true;
}

void BattleSituation::requestChoices()
{
    /* Gets the number of choices to be done */
    int count = int(requestChoice(Player1, false)) + requestChoice(Player2, false);

    /* Lock until BOTH choices are received */
    sem.acquire(count);

    //test to see if the quit was requested by system or if choice was received
    testquit();

    notify(All, BeginTurn, All, turn());

    /* Now all the players gonna do is analyzeChoice(int player) */
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
    for (int i = 0; i < 4; i++) {
	if (!isMovePossible(player,i)) {
	    ret.attackAllowed[i] = false;
	}
    }

    if (poke(player).item() != 30) /* Shed Shell */
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
    }

    return ret;
}

bool BattleSituation::isMovePossible(int player, int move)
{
    return (poke(player).move(move).PP() > 0 && (turnlong[player]["Move" + QString::number(move) + "Blocked"].toBool() == false));
}

void BattleSituation::analyzeChoice(int player)
{
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice[player].attack()) {
	if (!koed(player)) {
	    if (turnlong[player].contains("NoChoice"))
		useAttack(player, choice[player].numSwitch, false, true);
	    else
		useAttack(player, choice[player].numSwitch);
	}
    } else {
        if (!koed(player)) { /* if the pokemon isn't ko, it IS sent back */
	    sendBack(player);
	}
	sendPoke(player, choice[player].numSwitch);
    }
}

void BattleSituation::analyzeChoices()
{
    /* If there's no choice then the effects are already taken care of */
    if (!turnlong[Player1].contains("NoChoice") && choice[Player1].attack())
	MoveEffect::setup(poke(Player1).move(choice[Player1].numSwitch), Player1, Player2, *this);
    if (!turnlong[Player2].contains("NoChoice") && choice[Player2].attack())
	MoveEffect::setup(poke(Player2).move(choice[Player2].numSwitch), Player2, Player1, *this);

    std::multimap<int, int, std::greater<int> > priorities;
    QSet<int> switches;

    for (int i = Player1; i <= Player2; i++) {
	if (choice[i].poke())
	    switches.insert(i);
	else
	    priorities.insert(std::pair<int, int>(turnlong[i]["SpeedPriority"].toInt(), i));
    }

    foreach(int player, switches) {
	analyzeChoice(player);
    }
    foreach(int player, switches) {
	callEntryEffects(player);
    }
    requestSwitchIns();

    std::multimap<int, int, std::greater<int> >::const_iterator it;
    std::multimap<int, int, std::greater<int> >::const_iterator itEnd;

    for (it = priorities.begin(); it != priorities.end(); ) {
	itEnd = priorities.upper_bound(it->first);

	/* There's another priority system: Ability stall, and Item lagging tail */
	std::multimap<int, int, std::greater<int> > secondPriorities;
	for (; it != itEnd; ++it) {
	    int player = it->second;
	    callieffects(player,player, "TurnOrder");
	    secondPriorities.insert(std::pair<int,int>(turnlong[player]["TurnOrder"].toInt(), player));
	}

	std::multimap<int, int, std::greater<int> >::const_iterator it2;
	std::multimap<int, int, std::greater<int> >::const_iterator it2End;

	for(it2 = secondPriorities.begin(); it2 != secondPriorities.end();) {
	    it2End = secondPriorities.upper_bound(it2->first);

	    /* At last the speed comparison... */
	    std::map<int, int, std::greater<int> > speeds;
	    for (; it2 != it2End; ++it2) {
		speeds.insert(std::pair<int,int>(getStat(it2->second, Speed), it2->second));
	    }

	    std::multimap<int, int, std::greater<int> >::const_iterator it3;
	    for(it3 = speeds.begin(); it3 != speeds.end();)
	    {
		std::multimap<int, int, std::greater<int> >::const_iterator it3End = speeds.upper_bound(it3->first);
		std::vector<int> heap;
		for (; it3 != it3End; ++it3) {
		    heap.push_back(it3->second);
		}
		std::random_shuffle(heap.begin(), heap.end());
		foreach(int player, heap) {
		    analyzeChoice(player);
		}
	    }
	}
    }
}

void BattleSituation::battleChoiceReceived(int id, const BattleChoice &b)
{
    int player = spot(id);

    if (haveChoice[player] == false) {
	//INVALID BEHAVIOR
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
	    haveChoice[player] = false;
	    sem.release(1);
	}
    }
}

void BattleSituation::battleChat(int id, const QString &str)
{
    notify(Player1, BattleChat, spot(id), str);
    notify(Player2, BattleChat, spot(id), str);
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
    pokelong[player]["Type1"] = PokemonInfo::Type1(poke(player).num());
    pokelong[player]["Type2"] = PokemonInfo::Type2(poke(player).num());
    for (int i = 1; i <= 6; i++)
	pokelong[player][QString("Stat%1").arg(i)] = poke(player).normalStat(i);
    pokelong[player]["Level"] = poke(player).level();

    ItemEffect::setup(poke(player).item(),player,*this);
}

void BattleSituation::callEntryEffects(int player)
{
    calleffects(player, player, "UponSwitchIn");
    callzeffects(player, player, "UponSwitchIn");
}

void BattleSituation::calleffects(int source, int target, const QString &name)
{

    if (turnlong[source].contains(name)) {
	qDebug() << "Calling effects for " << name ;
	QSet<QString> &effects = *turnlong[source][name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
	    qDebug() << "Called by " << effect;
	    turnlong[source]["EffectBlocked"] = false;
	    turnlong[source]["EffectActivated"] = effect;
	    callpeffects(source, target, "BlockTurnEffects");
	    if (turnlong[source]["EffectBlocked"].toBool() == true) {
		continue;
	    }

	    MoveMechanics::function f = turnlong[source][name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callpeffects(int source, int target, const QString &name)
{
    if (pokelong[source].contains(name)) {
	qDebug() << "Calling pokemon long effects for " << name;
	QSet<QString> &effects = *pokelong[source][name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
	    qDebug() << "Called by " << effect;
	    MoveMechanics::function f = pokelong[source][name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callbeffects(int source, int target, const QString &name)
{
    if (battlelong.contains(name)) {
	QSet<QString> &effects = *battlelong[name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
	    MoveMechanics::function f = battlelong[name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callzeffects(int source, int target, const QString &name)
{
    if (teamzone[source].contains(name)) {
	QSet<QString> &effects = *teamzone[source][name].value<QSharedPointer<QSet<QString> > >();

	foreach(QString effect, effects) {
	    MoveMechanics::function f = teamzone[source][name + "_" + effect].value<MoveMechanics::function>();

	    f(source, target, *this);
	}
    }
}

void BattleSituation::callieffects(int source, int target, const QString &name)
{
    ItemEffect::activate(name, poke(source).item(), source, target, *this);
}

void BattleSituation::sendBack(int player)
{
    changeCurrentPoke(player, -1);

    notify(All, SendBack, player);
}

bool BattleSituation::testAccuracy(int player, int target)
{
    int acc = turnlong[player]["Accuracy"].toInt();

    if (acc == 0) {
	return true;
    }

    callieffects(player,target,"StatModifier");
    callieffects(target,player,"StatModifier");
    /* no *=: remember, we're working with fractions & int, changing the order might screw up by 1 % or so
	due to the ever rounding down to make an int */
    acc = acc * getStatBoost(player, 7) * getStatBoost(target, 6)
	    * (20+turnlong[player]["Stat7ItemModifier"].toInt())/20
	    * (20-turnlong[target]["Stat6ItemModifier"].toInt())/20;

    if (rand() % 100 < acc) {
	return true;
    } else {
	notify(All, Miss, player);
	return false;
    }
}

void BattleSituation::testCritical(int player, int target)
{
    (void) target; /* will be used for ability & lucky chant */

    int randnum = rand() % 48;
    int minch;
    switch(turnlong[player]["CriticalRaise"].toInt()) {
	case 0: minch = 3; break;
	case 1: minch = 6; break;
	case 2: minch = 12; break;
	case 3: minch = 16; break;
	case 4: default: minch = 24; break;
    }

    bool critical = randnum<minch;

    turnlong[player]["CriticalHit"] = critical;

    if (critical) {
	notify(All, CriticalHit, player, quint8(turnlong[player]["TypeMod"].toInt()));
    }
}

bool BattleSituation::testStatus(int player)
{
    switch (poke(player).status()) {
	case Pokemon::Asleep:
	{
	    if (poke(player).sleepCount() > 0) {
		poke(player).sleepCount() -= 1;
		notify(All, StatusMessage, player, qint8(FeelAsleep));
		return false;
	    }
	    healStatus(player, Pokemon::Asleep);
	    notify(All, StatusMessage, player, qint8(FreeAsleep));
	    break;
	}
	case Pokemon::Paralysed:
	{
	    if (rand() % 4 == 0) {
		notify(All, StatusMessage, player, qint8(PrevParalysed));
		return false;
	    }
	    break;
	}
	case Pokemon::Frozen:
	{
	    if (rand() % 255 > 51)
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
	return false;
    }
    if (pokelong[player]["Confused"].toBool()) {
	if (pokelong[player]["ConfusedCount"].toInt() > 0) {
	    inc(pokelong[player]["ConfusedCount"], -1);

	    notify(All, StatusMessage, player, qint8(FeelConfusion));

	    if (rand() % 2 == 0) {
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
}

void BattleSituation::testFlinch(int player, int target)
{
    int rate = turnlong[player]["FlinchRate"].toInt();

    if (rand() % 100 < rate) {
	turnlong[target]["Flinched"] = true;
    }

    if (poke(player).item() == 87 && turnlong[player]["KingRock"].toBool()) /* King's rock */
    {
	if (rand() % 100 < 10) {
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

void BattleSituation::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    int attack;

    if (specialOccurence) {
	attack = move;
    } else {
	attack = poke(player).move(move).num();
	pokelong[player]["MoveSlot"] = move;
    }

    turnlong[player]["HasMoved"] = true;

    if (!testStatus(player)) {
	return;
    }
    callpeffects(player, player, "DetermineAttackPossible");
    if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
	return;
    }

    turnlong[player]["MoveChosen"] = attack;

    if (!specialOccurence) {
	callpeffects(player, player, "MovePossible");
	if (turnlong[player]["ImpossibleToMove"].toBool() == true) {
	    return;
	}
	callpeffects(player, player, "MovesPossible");
	if (!isMovePossible(player, move)) {
	    return;
	}
	callieffects(player,player, "RegMoveSettings");
    }

    if (tellPlayers && !turnlong[player].contains("TellPlayers")) {
	notify(All, UseAttack, player, qint16(attack));
	qDebug() << poke(player).nick() << " used " << MoveInfo::Name(attack);
    }

    pokelong[player]["LastMoveUsed"] = attack;
    inc(pokelong[player]["MovesUsed"]);

    if (!specialOccurence)
	losePP(player, move, 1);

    calleffects(player, player, "MoveSettings");

    QList<int> targetList;

    switch(turnlong[player]["PossibleTargets"].toInt()) {
	case Move::None: targetList.push_back(player); break;
	case Move::User: targetList.push_back(player); break;
	case Move::All: targetList.push_back(player); targetList.push_back(rev(player)); break;
	default: targetList.push_back(rev(player));
    }

    if (targetList.size() == 1 && targetList[0] == rev(player) && koed(rev(player))) {
	notify(All, NoOpponent, player);
	return;
    }

    callieffects(player, player, "BeforeTargetList");

    foreach(int target, targetList) {
	turnlong[player]["Failed"] = false;
	turnlong[player]["FailingMessage"] = true;
	if (target != -1 && koed(target)) {
	    continue;
	}
	if (target != player && !testAccuracy(player, target)) {
	    continue;
	}
	qDebug() << "Accuracy test passed";
	qDebug() << "Accuracy: " << turnlong[player]["Power"].toInt();
	qDebug() << "Power: " << turnlong[player]["Power"].toInt();
	if (turnlong[player]["Power"].toInt() > 0)
	{
            qDebug() << "Going offensive";
	    int type = turnlong[player]["Type"].toInt(); /* move type */
	    int typeadv[] = {getType(target, 1), getType(target, 2)};
	    int typemod = TypeInfo::Eff(type, typeadv[0]) * TypeInfo::Eff(type, typeadv[1]);

	    int typepok[] = {getType(player,1), getType(player,2)};
	    int stab = 2 + (type==typepok[0] || type==typepok[1]);

	    turnlong[player]["Stab"] = stab;
	    turnlong[player]["TypeMod"] = typemod; /* is attack effective? or not? etc. */

	    if (typemod == 0) {
		/* If it's ineffective we just say it */
		notify(All, Effective, target, quint8(typemod));
		continue;
	    }

	    callpeffects(player, target, "DetermineAttackFailure");
	    if (testFail(player))
		continue;
	    callbeffects(player, target, "DetermineGeneralAttackFailure");
	    if (testFail(player))
		continue;
	    calleffects(player, target, "DetermineAttackFailure");
	    if (testFail(player))
		continue;

	    int num = repeatNum(turnlong[player]);
	    bool hit = num > 1;

	    for (int i = 0; i < num; i++) {

		if (hit) {
		    notify(All, Hit, target);
		}

		testCritical(player, target);

		calleffects(player, target, "BeforeCalculatingDamage");

		bool sub = hasSubstitute(player);
		turnlong[player]["HadSubstitute"] = sub;

		qDebug() << "Power"<< turnlong[player]["Power"].toInt();
		if (turnlong[player]["Power"].toInt() > 1) {
		    int damage = calculateDamage(player, target);
		    inflictDamage(target, damage, player, true);
		} else {
		    qDebug() << "The other way";
		    calleffects(player, target, "CustomAttackingDamage");
		}
		calleffects(player, target, "UponAttackSuccessful");

		if (turnlong[player]["PhysicalContact"].toBool()) {
		    if (!sub)
			callieffects(target, player, "UponPhysicalAssault");
		}
		/* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
		applyMoveStatMods(player, target);

		battlelong["LastMoveSuccesfullyUsed"] = attack;

		if (koed(target))
		    ;
		else
		    if (!sub)
			testFlinch(player, target);
		/* Removing substitute... */
		turnlong[player]["HadSubstitute"] = false;
	    }

	    notify(All, Effective, target, quint8(typemod));

	    calleffects(player, target, "AfterAttackSuccessful");
	} else {
            qDebug() << "Going tricky";
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
	    /* this is put after calleffects to avoid endless sleepTalk/copycat for example */
	    battlelong["LastMoveSuccesfullyUsed"] = attack;
	    calleffects(player, target, "AfterAttackSuccessful");
	}
	pokelong[target]["LastAttackToHit"] = attack;
    }
}

bool BattleSituation::hasWorkingAbility(int player, int ab)
{
    return poke(player).ability() == ab;
}

void BattleSituation::inflictRecoil(int source, int target)
{
    if (koed(source))
        return;

    int recoil = turnlong[source]["Recoil"].toInt();

    if (recoil == 0)
        return;

    notify(All, Recoil, source);
    inflictDamage(source, turnlong[target]["DamageTakenByAttack"].toInt()/recoil, source);
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    bool sub = hasSubstitute(player);

    QString effect = turnlong[player]["StatEffect"].value<QString>();

    /* First we check if there's even an effect... */
    if (effect.length() == 0) {
	return;
    }

    qDebug() << "Threre's an effect to apply";

    /* Then we check if the effect hits */
    int randnum = rand() % 100;
    int maxtogo = turnlong[player]["EffectRate"].toInt();

    if (maxtogo != 0 && randnum > maxtogo) {
	return;
    }

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
	}

	/* There maybe different type of changes, aka status & mod in move 'Flatter' */
	QStringList changes = effect.split('/');
	
	foreach (QString effect, changes)
	{
	    /* Now the kind of change itself */
	    bool statusChange = effect.midRef(1,3) == "[S]"; /* otherwise it's stat change */
    
	    QStringList possibilities = effect.mid(4).split('^');
    
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
    
	    QStringList mychoice = possibilities[rand()%possibilities.size()].split('&');
    
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
			    inflictStatus(targeted, status);
			}
		    }
		} else /* StatMod change */
		{
		    int stat = strtol(ptr, &ptr, 10);
		    int mod = strtol(ptr+1, NULL, 10);
		    char sep = *ptr;
    
		    if (sep == '+') {
			gainStatMod(targeted, stat, mod);
		    } else if (sep == '-') {
			loseStatMod(targeted, stat, mod);
		    } else {
			changeStatMod(targeted, stat, mod);
		    }
		}
	    }
	}
    }
}

void BattleSituation::healConfused(int player)
{
    pokelong[player]["Confused"] = false;
}

void BattleSituation::inflictConfused(int player)
{
    if (!pokelong[player]["Confused"].toBool()) {
	pokelong[player]["Confused"] = true;
	pokelong[player]["ConfusedCount"] = (rand() % 4) + 1;
	notify(All, StatusChange, player, qint8(-1));
    }
}

void BattleSituation::healStatus(int player, int status)
{
    if (poke(player).status() == status) {
	changeStatus(player, Pokemon::Fine);
    }
}

void BattleSituation::inflictStatus(int player, int status)
{
    if (poke(player).status() == Pokemon::Fine) {
	if (status == Pokemon::Poisoned || status == Pokemon::DeeplyPoisoned) {
	    if (!hasType(player, Pokemon::Poison) && !hasType(player, Pokemon::Steel)) {
		changeStatus(player, status);
	    }
	} else if (status == Pokemon::Burnt) {
	    if (!hasType(player, Pokemon::Fire)) {
		changeStatus(player, status);
	    }
	} else if (status == Pokemon::Frozen) {
	    if (!hasType(player, Pokemon::Ice)) {
		changeStatus(player, status);
	    }
	} else {
	    changeStatus(player, status);
	}
    }
}

bool BattleSituation::hasType(int player, int type)
{
    return getType(player,1) == type  || getType(player,2) == type;
}

int BattleSituation::getType(int player, int slot)
{
    int types[] = {pokelong[player]["Type1"].toInt(),pokelong[player]["Type2"].toInt()};

    if (ItemInfo::isPlate(poke(player).item())) {
	if (types[1] != Pokemon::Curse) {
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
    return hasType(player, Pokemon::Flying);
}

bool BattleSituation::hasSubstitute(int player)
{
    return !koed(player) && (pokelong[player].value("Substitute").toBool() || pokelong[player].value("HadSubstitute").toBool());
}

void BattleSituation::changeStatus(int player, int status)
{
    notify(All, StatusChange, player, qint8(status));
    poke(player).status() = status;
    if (status == Pokemon::Asleep) {
	poke(player).sleepCount() = (rand() % 5) +1;
    }
    if (status == Pokemon::DeeplyPoisoned) {
	pokelong[player]["ToxicCount"] = 0;
    }
}

void BattleSituation::changeStatus(int team, int poke, int status)
{
    if (poke == currentPoke(team)) {
	changeStatus(team, status);
    } else {
	this->poke(team, poke).status() = status;
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

void BattleSituation::loseStatMod(int player, int stat, int malus)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost > -6) {
	notify(All, StatChange, player, qint8(stat), qint8(-malus));
	changeStatMod(player, stat, std::max(boost-malus, -6));
    }
}

void BattleSituation::changeStatMod(int player, int stat, int newstat)
{
    QString path = tr("Boost%1").arg(stat);
    pokelong[player][path] = newstat;
}

int BattleSituation::calculateDamage(int p, int t)
{
    context &player = pokelong[p];
    context &move = turnlong[p];
    PokeBattle &poke = this->poke(p);

    int level = player["Level"].toInt();
    int attack, def;

    if (move["Category"].toInt() == Move::Physical) {
	attack = getStat(p, Attack);
	def = getStat(t, Defense);
    } else {
	attack = getStat(p, SpAttack);
	def = getStat(t, SpDefense);
    }

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = rand() % (255-217) + 217;
    int ch = 1 + move["CriticalHit"].toBool();
    int power = move["Power"].toInt();

    callieffects(p,t,"BasePowerModifier");
    power = power * (10+move["BasePowerItemModifier"].toInt())/10;

    int damage = ((((level * 2 / 5) + 2) * power * attack / 50) / def);
    damage = damage * ((poke.status() == Pokemon::Burnt && move["Category"].toInt() == Move::Physical) ? PokeFraction(1,2) : PokeFraction(1,1));
    damage = (damage+2)*ch;
    callieffects(p,t,"Mod2Modifier");
    damage = damage*(10+move["ItemMod2Modifier"].toInt())/10/*Mod2*/;
    damage = damage *randnum*100/255/100*stab/2*typemod/4;

    damage = damage * ((poke.item() == 7 && typemod > 4)? 6 : 5)/5 /* mod3 */;

    return damage;
}

PokeFraction BattleSituation::getMod1(int player, int)
{
    if (poke(player).status() == Pokemon::Burnt && turnlong[player]["Category"].toInt() == Move::Physical) {
	return PokeFraction(1, 2);
    } else {
	return PokeFraction(1, 1);
    }
}

int BattleSituation::repeatNum(context &move)
{
    int min = 1+move["RepeatMin"].toInt();
    int max = 1+move["RepeatMax"].toInt();

    if (min == max) {
	return min;
    } else {
	return min + (rand() % (max-min));
    }
}

void BattleSituation::inflictDamage(int player, int damage, int source, bool straightattack)
{
    if (koed(player)) {
	return;
    }

    if (straightattack)
	callieffects(player, source, "BeforeTakingDamage");

    qDebug() << "Damage inflicted (first v: " << damage << ") by " << source << " from " << player;
    if (damage == 0) {
	damage = 1;
    }

    bool sub = hasSubstitute(player);

    if (sub && player != source && straightattack) {
	inflictSubDamage(player, damage, source);
    } else {
	damage = std::min(int(poke(player).lifePoints()), damage);

	int hp  = poke(player).lifePoints() - damage;

	if (hp <= 0 && straightattack && turnlong[player].contains("CannotBeKoedBy") && turnlong[player]["CannotBeKoedBy"].toInt() == source) {
	    damage = poke(player).lifePoints() - 1;
	    hp = 1;
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
	    if (!sub) {
		callieffects(player, source, "UponOffensiveDamageReceived");
		calleffects(player, source, "UponOffensiveDamageReceived");
	    }
	}
    }

    if (!sub)
	turnlong[player]["DamageTaken"] = damage;
}

void BattleSituation::inflictSubDamage(int player, int damage, int source)
{
    int life = pokelong[player]["SubstituteLife"].toInt();

    if (life <= damage) {
	pokelong[player]["Substitute"] = false;
	turnlong[source]["DamageInflicted"] = life;
	sendMoveMessage(128, 1, player);
    } else {
	pokelong[player]["SubstituteLife"] = life-damage;
	turnlong[source]["DamageInflicted"] = damage;
    }
}

void BattleSituation::disposeItem(int  player) {
    poke(player).item() = 0;
}

void BattleSituation::acqItem(int player, int item) {
    poke(player).item() = item;
    ItemEffect::setup(poke(player).item(),player,*this);
}

void BattleSituation::healLife(int player, int healing)
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

void BattleSituation::changeHp(int player, int newHp)
{
    if (newHp == poke(player).lifePoints()) {
	/* no change, so don't bother */
	return;
    }
    poke(player).lifePoints() = newHp;

    notify(player, ChangeHp, player, quint16(newHp));
    notify(AllButPlayer, ChangeHp, player, newHp==0?quint16(0):quint16(std::max(1,poke(player).lifePoints()*100/poke(player).totalLifePoints()))); /* percentage calculus */
}

void BattleSituation::koPoke(int player, int source, bool straightattack)
{
    if (poke(player).ko()) {
	return;
    }

    qDebug() << "koPoke, player: " << player;
    changeHp(player, 0);
    qDebug() << "Changed Hp to 0";

    notify(All, Ko, player);
    qDebug() << "Notified players";
    koedPokes.insert(player);
    qDebug() << "Inserted it in the list";

    if (straightattack) {
	callpeffects(player, source, "AfterKoedByStraightAttack");
    }
}

void BattleSituation::requestSwitchIns()
{
    qDebug() << "Requesting switchin";
    int count = koedPokes.size();

    if (count == 0) {
        return;
    }

    qDebug() << "Before foreach loop (count is " << count << ")";
    foreach(int p, koedPokes) {
	qDebug() << "p is " << p;
        requestChoice(p, false);
    }
    qDebug() << "After foreach";

    sem.acquire(count);

    testquit();

    QSet<int> copy = koedPokes;

    foreach(int p, copy) {
        analyzeChoice(p);
    }

    foreach(int p, copy) {
	callEntryEffects(p);
    }

    /* Recursive call */
    requestSwitchIns();
}

void BattleSituation::requestSwitch(int player)
{
    int pokealive = countAlive(player) - (!poke(player).ko());

    if (pokealive == 0) {
	return;
    }

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

void BattleSituation::changeCurrentPoke(int player, int poke)
{
    mycurrentpoke[player] = poke;
}

void BattleSituation::changePP(int player, int move, int PP)
{
    poke(player).move(move).PP() = PP;
}

void BattleSituation::losePP(int player, int move, int loss)
{
    int PP = poke(player).move(move).PP();

    PP = std::max(PP-loss, 0);
    changePP(player, move, PP);

    notify(player, ChangePP, player, quint8(move), poke(player).move(move).PP());
}

int BattleSituation::getStat(int player, int stat)
{
    QString q = "Stat"+QString::number(stat);
    callieffects(player, player, "StatModifier");
    int ret = pokelong[player][q].toInt()*getStatBoost(player, stat)*(20+turnlong[player][q+"AbilityModifier"].toInt())/20*(20+turnlong[player][q+"ItemModifier"].toInt())/20;

    if (stat == Speed && poke(player).status() == Pokemon::Paralysed) {
	ret = ret * 3 / 4;
    }

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

void BattleSituation::sendItemMessage(int move, int src, int part, int foe)
{
    if (foe ==-1)
	notify(All, ItemMessage, src, quint16(move), uchar(part));
    else
	notify(All, ItemMessage, src, quint16(move), uchar(part), qint8(foe));
}

void BattleSituation::fail(int player, int move, int part, int type)
{
    turnlong[player]["FailingMessage"] = false;
    turnlong[player]["Failed"] = true;
    sendMoveMessage(move, part, player, type, rev(player));
}

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = pokelong[player][tr("Boost%1").arg(stat)].toInt();

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */
    if (stat <= 5) {
        return PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else if (stat == 7) {
        return PokeFraction(std::max(3+boost, 3), std::max(3-boost, 3));
    } else {
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

void BattleSituation::emitCommand(int player, int players, const QByteArray &tosend)
{
    if (players == All) {
	emit battleInfo(id(Player1), tosend);
	emit battleInfo(id(Player2), tosend);
    } else if (players == AllButPlayer) {
	emit battleInfo(id(rev(player)), tosend);
    } else {
	emit battleInfo(id(players), tosend);
    }
}

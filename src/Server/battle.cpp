#include "battle.h"
#include "player.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "moves.h"

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

    haveChoice[0] = false;
    haveChoice[1] = false;
    turn = 0;

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
    ++turn;
    /* Resetting temporary variables */
    turnlong[0].clear();
    turnlong[1].clear();

    notify(All, BeginTurn, All, turn);

    requestChoices();
    analyzeChoices();
}

void BattleSituation::endTurn()
{
    endTurnStatus();
}

void BattleSituation::endTurnStatus()
{
    for (int player = Player1; player <= Player2; player++)
    {
	switch(poke(player).status())
	{
	    case Pokemon::Burnt:
		notify(All, StatusMessage, player, qint8(HurtBurn));
		inflictDamage(player, poke(player).lifePoints()/8, player);
		break;
	    case Pokemon::DeeplyPoisoned:
		notify(All, StatusMessage, player, qint8(HurtPoison));
		inflictDamage(player, poke(player).lifePoints()*pokelong[player]["ToxicCount"].toInt()/16, player);
		inc(pokelong[player]["ToxicCount"], 1);
		break;
	    case Pokemon::Poisoned:
		notify(All, StatusMessage, player, qint8(HurtPoison));
		inflictDamage(player, poke(player).lifePoints()/8, player);
		break;
	}
    }
}

void BattleSituation::testquit()
{
    if (quit)
	throw QuitException();
}

void BattleSituation::requestChoice(int player, bool acquire)
{
    haveChoice[player] = true;
    options[player] = createChoice(player);

    notify(player, OfferChoice, player, options[player]);

    if (acquire)
	sem.acquire(1); /* Lock until a choice is received */

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
}

void BattleSituation::requestChoices()
{
    requestChoice(Player1, false);
    requestChoice(Player2, false);

    /* Lock until BOTH choices are received */
    sem.acquire(2);

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
}

bool BattleSituation::koed(int player) const
{
    return currentPoke(player) == -1 || poke(player).lifePoints() == 0;
}

BattleChoices BattleSituation::createChoice(int player) const
{
    /* First let's see for attacks... */
    if (koed(player)) {
	return BattleChoices::SwitchOnly();
    }

    BattleChoices ret;

    /* attacks ok, lets see which ones then */
    for (int i = 0; i < 4; i++) {
	if (poke(player).move(i).num() == 0 || poke(player).move(i).PP() == 0) {
	    ret.attackAllowed[i] = false;
	}
    }

    return ret;
}

void BattleSituation::analyzeChoice(int player)
{
    /* It's already verified that the choice is valid, by battleChoiceReceived, called in a different thread */
    if (choice[player].attack()) {
	useAttack(player, choice[player].numSwitch);
    } else {
        if (!koed(player)) { /* if the pokemon isn't ko, it IS sent back */
	    sendBack(player);
	}
	sendPoke(player, choice[player].numSwitch);
    }
}

void BattleSituation::analyzeChoices()
{
    if (choice[Player1].attack())
	merge(turnlong[Player1], MoveEffect(poke(Player1).move(choice[Player1].numSwitch).num()));
    if (choice[Player2].attack())
	merge(turnlong[Player2], MoveEffect(poke(Player2).move(choice[Player2].numSwitch).num()));

    if (choice[Player1].attack() && choice[Player2].attack()) {
	int first, second;

	if (turnlong[Player1]["SpeedPriority"].toInt() > turnlong[Player2]["SpeedPriority"].toInt()) {
	    first = Player1;
	} else if (turnlong[Player1]["SpeedPriority"].toInt() < turnlong[Player2]["SpeedPriority"].toInt()) {
	    first = Player2;
	} else {
	    first = getStat(Player1, Speed) > getStat(Player2, Speed) ? Player1 : Player2;
	}

	second = rev(first);

	analyzeChoice(first);
        if (turnlong[second]["CancelChoice"].toBool() != true && turnlong[second]["AttackKoed"].toBool() != true)
	    analyzeChoice(second);

	return;
    }
    if (choice[Player1].attack()) {
	analyzeChoice(Player2);
	analyzeChoice(Player1);
    } else {
	analyzeChoice(Player1);
	analyzeChoice(Player2);
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

/* Battle functions! Yeah! */

void BattleSituation::sendPoke(int player, int pok)
{
    changeCurrentPoke(player, pok);

    notify(player, SendOut, player, ypoke(player, pok));
    notify(AllButPlayer, SendOut, player, opoke(player, pok));

    /* reset temporary variables */
    pokelong[player].clear();
    /* Give new values to what needed */
    pokelong[player]["Type1"] = PokemonInfo::Type1(poke(player).num());
    pokelong[player]["Type2"] = PokemonInfo::Type2(poke(player).num());
    for (int i = 1; i <= 6; i++)
	pokelong[player][tr("Stat%1").arg(i)] = poke(player).normalStat(i);
    for (int i = 1; i <= 6; i++)
	pokelong[player][tr("Boost%1").arg(i)] = 0; /* No boost when a poke switches in, right? */
    pokelong[player][tr("Level")] = poke(player).level();
}

void BattleSituation::sendBack(int player)
{
    changeCurrentPoke(player, -1);

    notify(All, SendBack, player);
}

bool BattleSituation::testAccuracy(int player, int target)
{
    int acc = turnlong[player]["Accuracy"].toInt() * getStatBoost(player, 6) * getStatBoost(target, 7);

    if (acc == 0 || rand() % 100 < acc) {
	return true;
    } else {
	notify(All, Miss, player);
	return false;
    }
}

void BattleSituation::testCritical(int player, int)
{
    int randnum = rand() % 16;
    int minch = 1*(1+turnlong[player]["CriticalRaise"].toInt());
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
	    if (rand() % 255 > 25)
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

void BattleSituation::useAttack(int player, int move)
{
    int attack = poke(player).move(move).num();
    int target = rev(player);

    if (!testStatus(player)) {
	return;
    }

    notify(All, UseAttack, player, qint16(attack));

    losePP(player, move, 1);

    if (!testAccuracy(player, target)) {
	return;
    }
    if (turnlong[player]["Power"].toInt() > 0)
    {
	int type = turnlong[player]["Type"].toInt(); /* move type */
	int typeadv[] = {pokelong[target]["Type1"].toInt(), pokelong[target]["Type2"].toInt()};
	int typemod = TypeInfo::Eff(type, typeadv[0]) * TypeInfo::Eff(type, typeadv[1]);

	int typepok[] = {pokelong[player]["Type1"].toInt(), pokelong[player]["Type2"].toInt()};
	int stab = 2 + (type==typepok[0] || type==typepok[1]);

	turnlong[player]["Stab"] = stab;
	turnlong[player]["TypeMod"] = typemod; /* is attack effective? or not? etc. */

	if (typemod == 0) {
	    /* If it's ineffective we just say it */
	    notify(All, Effective, target, quint8(typemod));
	    return;
	}

	int num = repeatNum(turnlong[player]);
	bool hit = num > 1;

	for (int i = 0; i < num; i++) {
	    if (hit) {
		notify(All, Hit, target);
	    }

	    testCritical(player, target);

	    int damage = calculateDamage(player, target);
            inflictDamage(target, damage, player);

            /* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
            applyMoveStatMods(player, target);

	    if (turnlong[target]["AttackKoed"].toBool())
		break;
	}

        notify(All, Effective, target, quint8(typemod));

        requestSwitchIns();
    } else {
        applyMoveStatMods(player, target);
    }
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    QString effect = turnlong[player]["StatEffect"].value<QString>();

    /* First we check if there's even an effect... */
    if (effect.length() == 0) {
	return;
    }

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

	/* If the effect is on the opponent, and the opponent is Koed, we don't do nothing */
        if (!self && koed(target) == true) {
            continue;
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
	if (status == Pokemon::Poisoned || Pokemon::DeeplyPoisoned) {
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
    return pokelong[player]["Type1"].toInt() == type  || pokelong[player]["Type2"].toInt() == type;
}

void BattleSituation::changeStatus(int player, int status)
{
    notify(All, StatusChange, player, qint8(status));
    poke(player).status() = status;
    if (status == Pokemon::Asleep) {
	poke(player).sleepCount() = (rand() % 5) +1;
    }
    if (status == Pokemon::DeeplyPoisoned) {
	pokelong[player]["ToxicCount"] = 1;
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

int BattleSituation::calculateDamage(int _player, int _target)
{
    context &player = pokelong[_player];
    context &move = turnlong[_player];

    int level = player["Level"].toInt();
    int attack, def;

    if (move["Category"].toInt() == Move::Physical) {
	attack = getStat(_player, Attack);
	def = getStat(_target, Defense);
    } else {
	attack = getStat(_player, SpAttack);
	def = getStat(_target, SpDefense);
    }

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = rand() % (255-217) + 217;
    int ch = 1 + move["CriticalHit"].toBool();
    int power = move["Power"].toInt();

    PokeFraction mod1 = getMod1(_player, _target);

    int damage = (((((((level * 2 / 5) + 2) * power * attack / 50) / def) * mod1) + 2) * ch * 1 /*Mod2*/ * randnum * 100 / 255 / 100) * stab / 2 * typemod / 4 * 1 /* Mod3 */;

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
    int min = move["RepeatMin"].toInt();
    int max = move["RepeatMax"].toInt();

    if (min == max) {
	return min;
    } else {
	return min + (rand() % (max-min));
    }
}

void BattleSituation::inflictDamage(int player, int damage, int source)
{
    if (damage == 0) {
	damage = 1;
    }

    int hp  = poke(player).lifePoints() - damage;

    if (hp <= 0) {
	koPoke(player, source);
    } else {
	changeHp(player, hp);
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
    notify(AllButPlayer, ChangeHp, player, quint16(poke(player).lifePoints()*100/poke(player).totalLifePoints())); /* percentage calculus */
}

void BattleSituation::koPoke(int player, int source)
{
    changeHp(player, 0);

    notify(All, Ko, player);
    koedPokes.insert(player);

    if (source!=player) {
	turnlong[player]["AttackKoed"] = true; /* the attack the poke should have is not anymore */
	turnlong[player]["CancelChoice"] = true; /* the attack the poke should have is not anymore */
    }
}

void BattleSituation::requestSwitchIns()
{
    if (koedPokes.size() == 0) {
        return;
    }
    foreach(int p, koedPokes) {
        requestChoice(p, false);
    }
    int count = koedPokes.size();

    sem.acquire(count);

    testquit();

    foreach(int p, koedPokes) {
        analyzeChoice(p);
    }

    koedPokes.clear();
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
    int ret = pokelong[player][tr("Stat%1").arg(stat)].toInt()*getStatBoost(player, stat);

    if (stat == Speed && poke(player).status() == Pokemon::Paralysed) {
	ret = ret * 3 / 4;
    }

    return ret;
}

PokeFraction BattleSituation::getStatBoost(int player, int stat)
{
    int boost = pokelong[player][tr("Boost%1").arg(stat)].toInt();

    /* Boost is 1 if boost == 0,
       (2+boost)/2 if boost > 0;
       2/(2+boost) otherwise */
    if (stat <= 5) {
        return PokeFraction(std::max(2+boost, 2), std::max(2-boost, 2));
    } else if (stat == 6) {
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

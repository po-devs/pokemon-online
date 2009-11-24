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
    notify(Player1, BeginTurn, You, turn);
    notify(Player2, BeginTurn, You, turn);
    requestChoices();
    analyzeChoices();
}

void BattleSituation::endTurn()
{
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

    notify(player, OfferChoice, You, options[player]);

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
	if (currentPoke(player) != -1) { /* if the pokemon isn't ko, it IS sent back */
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
	    first = pokelong[Player1]["Stat3"].toInt() > pokelong[Player2]["Stat3"].toInt() ? Player1 : Player2;
	}

	second = rev(first);

	analyzeChoice(first);
	if (turnlong[second]["CancelChoice"].toBool() != true)
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

    notify(player, SendOut, You, ypoke(player, pok));
    notify(rev(player), SendOut, Opp, opoke(player, pok));

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

    notify(player, SendBack, You);
    notify(rev(player), SendBack, Opp);
}

void BattleSituation::useAttack(int player, int move)
{
    int attack = poke(player).move(move).num();

    notify(player, UseAttack, You, qint16(attack));
    notify(rev(player), UseAttack, Opp, qint16(attack));

    losePP(player, move, 1);

    if (turnlong[player]["Power"].toInt() > 0)
    {
	int randnum = rand() % 100 + 1;
	if (randnum > turnlong[player]["Accuracy"].toInt())
	{
	    notify(Player1, Miss, You);
	    notify(Player2, Miss, You);
	} else
	{
	    int target = rev(player);

	    int type = turnlong[player]["Type"].toInt(); /* move type */
	    int typeadv[] = {pokelong[target]["Type1"].toInt(), pokelong[target]["Type2"].toInt()};
	    int typemod = TypeInfo::Eff(type, typeadv[0]) * TypeInfo::Eff(type, typeadv[1]);

	    int typepok[] = {pokelong[player]["Type1"].toInt(), pokelong[player]["Type2"].toInt()};
	    int stab = 2 + (type==typepok[0] || type==typepok[1]);

	    turnlong[player]["Stab"] = stab;
	    turnlong[player]["TypeMod"] = typemod; /* is attack effective? or not? etc. */

	    if (typemod == 0) {
		/* If it's ineffective we just say it */
		notify(Player1, Effective, You, quint8(typemod));
		notify(Player2, Effective, You, quint8(typemod));
	    } else {
		int num = repeatNum(turnlong[player]);
		bool hit = num > 1;

		for (int i = 0; i < num; i++) {
		    if (hit) {
			notify(player, Hit, You);
			notify(target, Hit, Opp);
		    }

		    int randnum = rand() % 16;
		    int minch = 1*(1+turnlong[player]["CriticalRaise"].toInt());

		    turnlong[player]["CriticalHit"] = (randnum<minch);

		    if (turnlong[player]["CriticalHit"].toBool()) {
			notify(Player1, CriticalHit, You);
			notify(Player2, CriticalHit, You);
		    }

		    int damage = calculateDamage(turnlong[player], pokelong[player], pokelong[target]);

		    inflictDamage(target, damage, true);
		    if (turnlong[target]["AttackKoed"].toBool())
			break;
		}

		if (!turnlong[target]["AttackKoed"].toBool()) {
		    notify(Player1, Effective, You, quint8(typemod));
		    notify(Player2, Effective, You, quint8(typemod));
		}

		/* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
		applyMoveStatMods(player, target);
	    }
	}
    }
}

void BattleSituation::applyMoveStatMods(int player, int target)
{
    QString effect = turnlong[player]["StatEffect"].toString();

    /* First we check if there's even an effect... */
    if (effect.length() == 0) {
	return;
    }

    qDebug("Effect!");

    /* Then we check if the effect hits */
    int randnum = rand() % 100;
    int mintogo = turnlong[player]["StatEffect"].toInt();

    if (mintogo != 0 && randnum >= mintogo) {
	return;
    }

    qDebug("Not Cancelled!");

    /* Splits effects between the opponent & self */
    QStringList effects = effect.split('|');

    foreach(QString effect, effects)
    {
	/* Now we parse the effect.. */
	bool self = (effect[0] == 'S');
	int targeted = self? player : target;

	/* If the effect is on the opponent, and the opponent is Koed, we don't do nothing */
	if (!self && turnlong[target]["AttackKoed"].toBool() == true) {
	    return;
	}

	/* There maybe different type of changes, aka status & mod in move 'Flatter' */
	QStringList changes = effect.split('/');
	
	foreach (QString effect, changes)
	{
	    /* Now the kind of change itself */
	    bool statusChange = effect.leftRef(3) == "[S]"; /* otherwise it's stat change */
    
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
			    healStatus(player, status);
			} else {
			    inflictStatus(player, status);
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
    poke(player).confused() = false;
}

void BattleSituation::inflictConfused(int player)
{
    if (!poke(player).confused()) {
	poke(player).confused() = true;
	notify(player, StatusChange, You, qint8(-1));
	notify(rev(player), StatusChange, Opp, qint8(-1));
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
	changeStatus(player, status);
    }
}

void BattleSituation::changeStatus(int player, int status)
{
    notify(player, StatusChange, You, qint8(status));
    notify(rev(player), StatusChange, Opp, qint8(status));
    poke(player).status() = status;
}

void BattleSituation::gainStatMod(int player, int stat, int bonus)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost < 6) {
	notify(player, StatChange, You, qint8(stat), qint8(bonus));
	notify(rev(player), StatChange, Opp, qint8(stat), qint8(bonus));
	changeStatMod(player, stat, std::min(boost+bonus, 6));
    }
}

void BattleSituation::loseStatMod(int player, int stat, int malus)
{
    QString path = tr("Boost%1").arg(stat);
    int boost = pokelong[player][path].toInt();
    if (boost > -6) {
	notify(player, StatChange, You, qint8(stat), qint8(-malus));
	notify(rev(player), StatChange, Opp, qint8(stat), qint8(-malus));
	changeStatMod(player, stat, std::max(boost-malus, -6));
    }
}

void BattleSituation::changeStatMod(int player, int stat, int newstat)
{
    QString path = tr("Boost%1").arg(stat);
    pokelong[player][path] = newstat;
}

int BattleSituation::calculateDamage(context &move, context &player, context &target)
{
    int level = player["Level"].toInt();
    int attack, def;

    if (move["Category"].toInt() == Move::Physical) {
	attack = player["Stat1"].toInt();
	def = target["Stat2"].toInt();
    } else {
	attack = player["Stat4"].toInt();
	def = target["Stat5"].toInt();
    }

    int stab = move["Stab"].toInt();
    int typemod = move["TypeMod"].toInt();
    int randnum = rand() % (255-217) + 217;
    int ch = 1 + move["CriticalHit"].toBool();
    int power = move["Power"].toInt();

    int damage = (((((((level * 2 / 5) + 2) * power * attack / 50) / def) * /*Mod1*/ 1) + 2) * ch * 1 /*Mod2*/ * randnum * 100 / 255 / 100) * stab / 2 * typemod / 4 * 1 /* Mod3 */;

    /* The minimum damage is always 1... */
    if (damage == 0) {
	damage = 1;
    }

    return damage;
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

void BattleSituation::inflictDamage(int player, int damage, bool attacking)
{
    int hp  = poke(player).lifePoints() - damage;

    if (hp <= 0) {
	koPoke(player, attacking);
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

    notify(player, ChangeHp, You, quint16(newHp));
    notify(rev(player), ChangeHp, Opp, quint16(poke(player).lifePoints()*100/poke(player).totalLifePoints())); /* percentage calculus */
}

void BattleSituation::koPoke(int player, bool attacking)
{
    changeHp(player, 0);

    if (attacking) {
	notify(player, Effective, You, qint8(turnlong[player]["TypeMod"].toInt()));
	notify(rev(player), Effective, You, qint8(turnlong[player]["TypeMod"].toInt()));
    }

    notify(player, Ko, You);
    notify(rev(player), Ko, Opp);

    changeCurrentPoke(player, -1);

    requestChoice(player); /* Asks the player to switch a poke in! */
    analyzeChoice(player);

    if (attacking) {
	turnlong[player]["AttackKoed"] = true; /* the attack the poke should have is not anymore */
	turnlong[player]["CancelChoice"] = true; /* the attack the poke should have is not anymore */
    }
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

    notify(player, ChangePP, You, quint8(move), poke(player).move(move).PP());
}

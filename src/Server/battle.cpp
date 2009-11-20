#include "battle.h"
#include "player.h"

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
	sendBack(player);
	sendPoke(player, choice[player].numSwitch);
    }
}

void BattleSituation::analyzeChoices()
{
    if (choice[Player1].attack() && choice[Player2].attack()) {
	if (poke(Player1).normalStat(Speed) > poke(Player2).normalStat(Speed)) {
	    analyzeChoice(Player1);
	    analyzeChoice(Player2);
	} else {
	    analyzeChoice(Player2);
	    analyzeChoice(Player1);
	}
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
	    sem.release(1);
	}
    }
}

/* Battle functions! Yeah! */

void BattleSituation::sendPoke(int player, int poke)
{
    changeCurrentPoke(player, poke);

    notify(player, SendOut, You, ypoke(player, poke));
    notify(rev(player), SendOut, Opp, opoke(player, poke));
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

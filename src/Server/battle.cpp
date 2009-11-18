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
    /* Beginning of the battle! */
    sendPoke(Player1, 0);
    sendPoke(Player2, 0);

    quit = false;
    haveChoice[0] = false;
    haveChoice[1] = false;

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
    requestChoices();
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
    options[player] = BattleChoices();

    notify(player, OfferChoice, You, options[player]);

    if (acquire)
	sem.acquire(1);

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
}

void BattleSituation::requestChoices()
{
    requestChoice(Player1, false);
    requestChoice(Player2, false);

    /* Calling sem.acquire(1) twice wouldn't have the same effect */
    sem.acquire(2);

    //test to see if the quit was requested by system or if choice was received
    testquit();

    /* Now all the players gonna do is analyzeChoice(int player) */
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

void BattleSituation::changeCurrentPoke(int player, int poke)
{
    mycurrentpoke[player] = poke;
}

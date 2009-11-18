#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class Player;

class BattleSituation : public QThread
{
    Q_OBJECT
public:
    enum {
	Player1,
	Player2
    };
    static const bool You = true;
    static const bool Opp = false;

    BattleSituation(Player &p1, Player &p2);
    ~BattleSituation();

    const TeamBattle &pubteam(int id);
    /* returns 0 or 1, or -1 if that player is not involved */
    int spot(int id) const;
    /* The other player */
    int rev(int spot) const;
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    int id(int spot) const;

    /*
	Below Player is either 1 or 0, aka the spot of the id.
	Use the functions above to make conversions
    */
    TeamBattle &team(int player);
    const TeamBattle &team(int player) const;
    PokeBattle &poke(int player);
    const PokeBattle &poke(int player) const;
    PokeBattle &poke(int player, int poke);
    const PokeBattle &poke(int player, int poke) const;
    int currentPoke(int player) const;
    void changeCurrentPoke(int player, int poke);

    /* Starts the battle -- use the time before to connect signals / slots */
    void start();
    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();
    /* requests choice of action from the player */
    void requestChoice(int player, bool acq = true /*private arg used by RequestChoices */);
    void requestChoices(); /* request from both players */

    /* Commands for the battle situation */
    void beginTurn();
    void endTurn();
    void sendPoke(int player, int poke);
    /* Sends a poke back to his pokeball (not koed) */
    void sendBack(int player);
    /* Does not do extra operations,just a setter */
    void changeHp(int player, int newHp);
    void removePoke(int player);
    void koPoke(int player);
    /* Does not do extra operations,just a setter */
    void changeStatMod(int player, int stat, int newstatmod);
    void gainStatMod(int player, int stat, int bonus);
    void loseStatMod(int player, int stat, int malus);
    /* Does not do extra operations,just a setter */
    void changeStatus(int player, int poke, int status);
    void healStatus(int player, int poke);
    void healConfused(int player);
    void healLife(int player, int healing);
    void inflictStatus(int player, int Status);
    void inflictConfused(int player);
    void inflictDamage(int player, int damage);

    /* conversion for sending a message */
    quint8 ypoke(int, int i) const { return i; } /* aka 'your poke', or what you need to know if it's your poke */
    ShallowBattlePoke opoke(int play, int i) const { return ShallowBattlePoke(poke(play, i));} /* aka 'opp poke', or what you need to know if it's your opponent's poke */

    /* Send a message to the outworld */
    enum BattleCommand
    {
	SendOut,
	OfferChoice,
	BeginTurn
    };

    /* Here C++0x would make it so much better looking with variadic templates! */
    template<class T>
    void notify(int player, int command, bool who, const T& param);
public slots:
    void battleChoiceReceived(int id, const BattleChoice &b);
signals:
    void battleInfo(int id, const QByteArray &info);
private:
    /* To interrupt the thread when needed */
    QSemaphore sem;
    /* To notify the thread to quit */
    bool quit;
    /* if quit==true, throws QuitException */
    void testquit();

    /* What choice we allow the players to have */
    BattleChoices options[2];
    BattleChoice choice[2];
    bool haveChoice[2];

    TeamBattle team1, team2;
    int mycurrentpoke[2]; /* -1 for koed */
    int myid[2];
    int turn;
public:
    struct QuitException {};
};


template<class T>
void BattleSituation::notify(int player, int command, bool who, const T& param)
{
    /* Doing that cuz we never know */
    testquit();

    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << who << param;

    emit battleInfo(id(player), tosend);
}

#endif // BATTLE_H

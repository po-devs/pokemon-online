#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class Player;

class BattleSituation : public QObject
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
	OfferChoice
    };

    /* Here C++0x would make it so much better looking with variadic templates! */
    template<class T>
    void notify(int player, int command, bool who, const T& param);
signals:
    void battleInfo(int id, const QByteArray &info);
private:

    TeamBattle team1, team2;
    int mycurrentpoke[2];
    int myid[2];
};

struct BattleChoice
{
    /* Sets everything to true */
    BattleChoice();
    void disableSwitch();
    void disableAttack(int attack);
    void disableAttacks();

    bool switchAllowed;
    bool attacksAllowed[4];

    static BattleChoice SwitchOnly();
};

QDataStream & operator >> (QDataStream &in, BattleChoice &po);
QDataStream & operator << (QDataStream &out, const BattleChoice &po);

template<class T>
void BattleSituation::notify(int player, int command, bool who, const T& param)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << who << param;

    emit battleInfo(id(player), tosend);
}

#endif // BATTLE_H

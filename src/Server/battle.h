#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class Player;

class BattleSituation : public QObject
{
    Q_OBJECT
public:
    BattleSituation(Player &p1, Player &p2);

    const TeamBattle &pubteam(int id);
    /* returns 0 or 1, or -1 if that player is not involved */
    int spot(int id) const;
    TeamBattle &team(int spot);
    const TeamBattle &team(int spot) const;
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    int id(int spot) const;
private:

    TeamBattle team1, team2;
    int myid[2];
};

#endif // BATTLE_H

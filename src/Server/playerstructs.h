#ifndef PLAYERSTRUCTS_H
#define PLAYERSTRUCTS_H

#include "../PokemonInfo/battlestructs.h"

struct TeamHolder
{
    TeamHolder(){}
    TeamHolder(const Team &t) : team(t), waiting(true) {

    }

    TeamBattle team;
    bool waiting;
};

struct TeamsHolder
{
    void init(const QList<Team> &teams) {
        teams.clear();
        for (int i = 0; i < teams.size(); i++) {
            this->teams.push_back(std::move(TeamHolder(teams[i])));
        }
    }

    QList<TeamHolder> teams;
};

#endif // PLAYERSTRUCTS_H

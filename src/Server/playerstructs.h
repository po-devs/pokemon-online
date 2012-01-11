#ifndef PLAYERSTRUCTS_H
#define PLAYERSTRUCTS_H

#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/networkstructs.h"

struct TeamHolder
{
    TeamHolder(){}
    TeamHolder(PersonalTeam &t) : team(t), waiting(true) {

    }

    TeamBattle team;
    bool waiting;
};

struct TeamsHolder
{
    void init(QList<PersonalTeam> &teams) {
        teams.clear();
        for (int i = 0; i < teams.size(); i++) {
            this->teams.push_back(std::move(TeamHolder(teams[i])));
        }
    }

    TeamBattle &team(int i) {return teams[i].team;}
    const TeamBattle &team(int i) const {return teams[i].team;}

    QList<TeamHolder> teams;
};

#endif // PLAYERSTRUCTS_H

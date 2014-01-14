#ifndef PLAYERSTRUCTS_H
#define PLAYERSTRUCTS_H

#include <PokemonInfo/battlestructs.h>
#include <PokemonInfo/networkstructs.h>

struct TeamsHolder
{
    void init(QList<PersonalTeam> &teams) {
        this->teams.clear();
        for (int i = 0; i < teams.size(); i++) {
            this->teams.push_back(TeamBattle(teams[i]));
        }
    }

    void init() {
        teams.clear();
        teams.push_back(TeamBattle());
    }

    TeamBattle &team(int i) {return teams[i];}
    const TeamBattle &team(int i) const {return teams[i];}
    int count() const {return teams.count();}

    QList<TeamBattle> teams;
};

#endif // PLAYERSTRUCTS_H

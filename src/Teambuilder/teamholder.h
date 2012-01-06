#ifndef TEAMHOLDER_H
#define TEAMHOLDER_H

#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/networkstructs.h"
#include "teamholderinterface.h"

class TeamHolder : public TeamHolderInterface
{
    PROPERTY(TrainerInfo, info);
    PROPERTY(QString, name);
    PROPERTY(Team, team);
};

#endif // TEAMHOLDER_H

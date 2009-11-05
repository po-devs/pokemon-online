#ifndef SERVER_H
#define SERVER_H

#include "../PokemonInfo/pokemonstructs.h"
#include "analyze.h"

/* a single player */
class Player : public QObject
{
    Q_OBJECT
public:
    Player();

    TrainerTeam &team();
signals:

private:
    TrainerTeam myteam;
    Analyzer myrelay;
};

#endif // SERVER_H

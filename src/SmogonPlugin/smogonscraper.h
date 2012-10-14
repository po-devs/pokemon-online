#ifndef SMOGONSCRAPER_H
#define SMOGONSCRAPER_H

#include <string>

#include <QtCore>

#include <QtNetwork>

#include <QDomDocument>
#include "../PokemonInfo/pokemoninfo.h"


using namespace std;

/*
 * This build object is used to represent a pokemon build from Smogon
 * The use of arrays are in many cases due to the fact that Smogon gives you
 *   multiple choices of what Item or move you want to use.
 */
struct BuildObject
{
    string buildName;
    string item[];
    string nature[];
    int EVList[6];
    string moves[4][];
    string description;
};


/*
 * The SmogonScraper class is simply a class that wraps up the functionality:
 *   connecting and scraping the pokemon's data from Smogon
 *   parsing that data into BuildObjects
 *   returning those builds to the user
 */
class SmogonScraper
{

private:
    string scrapePage(Pokemon::gen gen, PokeTeam pokeName);
    BuildObject parsePage(string page);
    /*Note, it might be better to have this function just return all the builds*/
public:
    SmogonScraper::SmogonScraper();
    static BuildObject* get(Pokemon::gen gen, PokeTeam pokeName);
};


#endif // SMOGONSCRAPER_H

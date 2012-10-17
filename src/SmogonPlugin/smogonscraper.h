#ifndef SMOGONSCRAPER_H
#define SMOGONSCRAPER_H

#include <string>

#include <QtCore>
#include <QtNetwork>
#include <QObject>

#include <QDomDocument>
#include "../PokemonInfo/pokemoninfo.h"
#include "pokemontab.h"
#include "SmogonBuild.h"

/*
 * The SmogonScraper class is simply a class that wraps up the functionality:
 *   connecting and scraping the pokemon's data from Smogon
 *   parsing that data into BuildObjects
 *   returning those builds to the user
 */
class SmogonScraper : public QObject {
    Q_OBJECT

public:
    SmogonScraper(PokemonTab* srcTab);
    void lookup(Pokemon::gen gen, PokeTeam pokeName);

public slots:
    void reciever(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
    PokemonTab* uiTab;
    SmogonBuild* parsePage(QString webPage);

};


#endif // SMOGONSCRAPER_H

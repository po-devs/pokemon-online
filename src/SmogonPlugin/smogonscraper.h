#ifndef SMOGONSCRAPER_H
#define SMOGONSCRAPER_H

#include <string>

#include <QtCore>
#include <QtNetwork>
#include <QObject>

#include <QDomDocument>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/enums.h"
#include "pokemontab.h"
#include "smogonbuild.h"

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
    ~SmogonScraper();
    void lookup(PokeTeam pokeName);

public slots:
    void reciever(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
    PokemonTab* uiTab;
    QString currGen;
    QList<smogonbuild>* parsePage(QString webPage);
    QList<QString> *getHtmlBuilds(QString webPage, int numBuilds);

    /*Helper functions for parsing the page*/
    QString getBuildName(QString htmlBuild);
    QList<QString> *getNature(QString htmlBuild);
    QList<QString> *getItem(QString htmlBuild);
    QList<int> *getEVs(QString htmlBuild);
    QList<QString> *getAbility(QString htmlBuild);
    QList<QString> *getMove(QString htmlBuild, int moevNum);
    QString getDescription(QString htmlBuild);

    /*Helper functions for the helper functions*/
    QString getContents(QString htmlBuild, int start);
    QString getEVString(QString htmlBuild);

};


#endif // SMOGONSCRAPER_H

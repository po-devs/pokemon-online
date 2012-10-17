#include "smogonscraper.h"

/*
 *Since the only method that is ever going to be accessed is static, we don't
 *  need to instantiate anything for new instances. In fact we probably don't
 *  need a constructor.
 */


SmogonScraper::SmogonScraper(PokemonTab* srcTab)
{
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(reciever(QNetworkReply*)));

    uiTab = srcTab;
}

void SmogonScraper::lookup(Pokemon::gen gen, PokeTeam pokeName)
{
    //There will have to be some parsing of the data to create a "good" url
    QString webPage = QString("http://www.smogon.com/bw/pokemon/chansey");

    manager->get(QNetworkRequest(QUrl(webPage)));
}

SmogonBuild* SmogonScraper::parsePage(QString webPage)
{
    //Dumb Algorithm that gets the data on the 241st line
    QStringList ls = webPage.split("\n");
    QString oneLine = ls.at(419);

    SmogonBuild* builds = new SmogonBuild[1];
    builds[0].description = oneLine;
    return builds;
}

void SmogonScraper::reciever(QNetworkReply* reply)
{
    QString webPage = QString(reply->readAll());
    SmogonBuild *builds = parsePage(webPage);
    uiTab->update_ui(builds);
}

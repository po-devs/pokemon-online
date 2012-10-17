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

void SmogonScraper::lookup(Pokemon::gen gen, PokeTeam p)
{
    //TODO: add approximation for edge cases such as yellow version
    QString genString = "";
    switch(gen.num)
    {
    case 1:genString = "rb";
    case 2:genString = "gs";
    case 3:genString = "rs";
    case 4:genString = "dp";
    case 5:genString = "bw";
    }

    QString name = PokemonInfo::Name(p.num());


    QString webPage = QString("http://www.smogon.com/"+genString+"/pokemon/"+name);

    manager->get(QNetworkRequest(QUrl(webPage)));
}

SmogonBuild* SmogonScraper::parsePage(QString webPage)
{
    //Dumb Algorithm that gets the data on the 241st line
    QStringList ls = webPage.split("\n");
    QString oneLine= ls.at(9);

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

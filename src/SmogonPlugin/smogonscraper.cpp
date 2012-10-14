#include "smogonscraper.h"

/*
 *Since the only method that is ever going to be accessed is static, we don't
 *  need to instantiate anything for new instances. In fact we probably don't
 *  need a constructor.
 */
SmogonScraper::SmogonScraper()
{
}

static BuildObject* SmogonScraper::get(Pokemon::gen gen, PokeTeam pokeName)
{
    QString webPage = QString(scrapePage(gen, pokeName));
    int numberOfBuilds = 0;
    BuildObject currBuild;
    BuildObject* fullList = new BuildObject[10];//Definatly less then 10 builds
    while((currBuild = parsePage(webPage)) != NULL)
    {
        fullList[numberOfBuilds] = currBuild;
        numberOfBuilds++;
    }

    BuildObject* retList = new BuildObject[numberOfBuilds];
    for(int i=0;i<numberOfBuilds;i++)
    {
        retList[i] = fullList[i];
    }
    return retList;
}

/*I believe this will get the target data*/
static string SmogonScraper::scrapePage(Pokemon::gen gen, PokeTeam pokeName)
{
    /*TODO: actually parse the inputs so we can get real data*/
    string url = "http://www.smogon.com/bw/pokemon/chansey";

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    QNetworkRequest request;

    request.setUrl(QUrl(QString(url)));
    request.setRawHeader("User-Agent", "Smogon Plugin data request");

    QNetworkReply *reply = manager.get(request);
    return reply->readAll();
}

static BuildObject SmogonScraper::parsePage(QString page)
{
    /*QDomDocument myDoc;
    myDoc.setContent(QString(page));
    QDomNodeList tdList = myDoc.elementsByTagName("td");
    for(int i = 0;i<tdList.length;i++)
    {
        QDomNode temp = tdList.at(i);
    }*/


    //Dumb Algorithm that gets the data on the 241st line
    QStringList ls = page.split("\n");
    QString oneLine = ls.at(420);

    BuildObject retObj;
    retObj.description = oneLine;
    return retObj;
}

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

SmogonScraper::~SmogonScraper()
{
    delete manager;
}

void SmogonScraper::lookup(PokeTeam p)
{
    //TODO: add approximation for edge cases such as yellow version
    currGen = "";
    switch(p.gen().num)
    {
    case 1:currGen = "rb";break;
    case 2:currGen = "gs";break;
    case 3:currGen = "rs";break;
    case 4:currGen = "dp";break;
    case 5:currGen = "bw";break;
    default:currGen = "bw";/*TODO: modify this*/
    }

    QString name = PokemonInfo::Name(p.num());


    QString webPage = QString("http://www.smogon.com/"+currGen+"/pokemon/"+name);

    manager->get(QNetworkRequest(QUrl(webPage)));
}

/*
 *  Takes the full Html page from the web and parses it to find the information relating to
 *      the SmogonBuild Object, creating all SmogonBuild objects that it finds on the page.
 */
QList<smogonbuild>* SmogonScraper::parsePage(QString webPage)
{
    int numBuilds = webPage.count("class=\"name\"");
    QList<smogonbuild> *buildsList = new QList<smogonbuild>();

    QList<QString> *htmlBuilds = getHtmlBuilds(webPage, numBuilds);
    for(int i = 0; i<numBuilds; i++)
    {
        QString htmlBuild = htmlBuilds->at(i);
        smogonbuild *tempBuild = new smogonbuild();
        tempBuild->buildName = getBuildName(htmlBuild);
        tempBuild->item = getItem(htmlBuild);
        tempBuild->ability = getAbility(htmlBuild);
        tempBuild->nature = getNature(htmlBuild);

        int movesParsed = 0;
        tempBuild->move1 = getMove(htmlBuild, movesParsed);
        movesParsed += tempBuild->move1->size();
        tempBuild->move2 = getMove(htmlBuild,movesParsed);
        movesParsed += tempBuild->move2->length();
        tempBuild->move3 = getMove(htmlBuild,movesParsed);
        movesParsed += tempBuild->move3->length();
        tempBuild->move4 = getMove(htmlBuild,movesParsed);

        tempBuild->EVList = getEVs(htmlBuild);
        tempBuild->description = getDescription(htmlBuild);
        buildsList->push_back(*tempBuild);

        //tempBuild->printBuild();
    }

    return buildsList;
}

QString SmogonScraper::getBuildName(QString htmlBuild)
{
    QString buildName;
    buildName = htmlBuild.left(htmlBuild.indexOf("<"));
    return buildName;
}

QList<QString> *SmogonScraper::getNature(QString htmlBuild)
{
    QList<QString> *lsString = new QList<QString>();
    QString natureString = this->currGen+"/natures/";
    int numNatures = htmlBuild.count(natureString);
    for(int i=0; i<numNatures; i++)
    {
        int start = htmlBuild.indexOf(natureString);
        QString nature = this->getContents(htmlBuild, start);
        lsString->push_back(nature);
        htmlBuild = htmlBuild.right(htmlBuild.length()-start-5);
    }
    return lsString;
}

QList<QString> *SmogonScraper::getItem(QString htmlBuild)
{
    QList<QString> *lsString = new QList<QString>();
    QString itemString = this->currGen+"/items/";
    int numItems = htmlBuild.count(itemString);
    for(int i=0; i<numItems; i++)
    {
        int start = htmlBuild.indexOf(itemString);
        QString item = this->getContents(htmlBuild, start);
        lsString->push_back(item);
        htmlBuild = htmlBuild.right(htmlBuild.length()-start-5);
    }
    return lsString;
}

/*
 * The EVs are ordered as follows (HP, Att, Def, SpA, SpD, Spe)
 */
QList<int> *SmogonScraper::getEVs(QString htmlBuild)
{
    QList<int> *lsInt = new QList<int>();
    /*Initiate all of the values to zero*/
    lsInt->push_back(0);lsInt->push_back(0);lsInt->push_back(0);
    lsInt->push_back(0);lsInt->push_back(0);lsInt->push_back(0);

    QString evString = getEVString(htmlBuild);
    evString = evString.remove(QChar('\n'));
    QStringList stringList = evString.split(" ");
    QString contString = "/";
    for(int i=0; contString == "/"; i++)
    {
        QString stat = stringList.at(stringList.length()-1-3*i);
        QString value = stringList.at(stringList.length()-2-3*i);

        if(stat =="HP"){
            lsInt->replace(0,value.toInt());
        }
        else if(stat == "Atk"){
            lsInt->replace(1,value.toInt());
        }
        else if(stat == "Def"){
            lsInt->replace(2,value.toInt());
        }
        else if(stat == "SpA"){
            lsInt->replace(3,value.toInt());
        }
        else if(stat == "SpD"){
            lsInt->replace(4,value.toInt());
        }
        else if(stat == "Spe"){
            lsInt->replace(5,value.toInt());
        }

        contString = stringList.at(stringList.length()-3-3*i);
    }

    /*Parse the EV String*/
    return lsInt;
}

QList<QString> *SmogonScraper::getAbility(QString htmlBuild)
{
    QList<QString> *lsString = new QList<QString>();
    QString abilityString = this->currGen+"/abilities/";
    int numAbilities = htmlBuild.count(abilityString);
    for(int i=0; i<numAbilities; i++)
    {
        int start = htmlBuild.indexOf(abilityString);
        QString ability = this->getContents(htmlBuild, start);
        lsString->push_back(ability);
        htmlBuild = htmlBuild.right(htmlBuild.length()-start-5);
    }
    return lsString;
}

QList<QString> *SmogonScraper::getMove(QString htmlBuild, int movesParsed)
{
    QList<QString> *lsString = new QList<QString>();
    QString moveString = this->currGen+"/moves/";
    int start = htmlBuild.indexOf(moveString);
    //Pass over the moves that have already been added to the list
    for(int i=0;i<movesParsed;i++)
    {
        QString tempStr = htmlBuild.right(htmlBuild.length() - start-1);
        start += tempStr.indexOf(moveString)+1;
    }
    bool movesLeft = true;
    bool noBreak = true;
    while(movesLeft && noBreak)
    {
        QString move = getContents(htmlBuild, start);
        lsString->push_back(move);
        int nextMove = htmlBuild.right(htmlBuild.length()-start-1).indexOf(moveString);
        movesLeft = (nextMove!=-1);
        if(movesLeft)
            noBreak = htmlBuild.mid(start, nextMove+1).indexOf("<br />") == -1;
        start += nextMove+1;
    }
    return lsString;
}

QString SmogonScraper::getDescription(QString htmlBuild)
{
    int start = htmlBuild.indexOf("<p>");
    QString retString = getContents(htmlBuild, start);
    return retString;
}

/*
 *  Given the html representation of a SmogonBuild and a position in the tag
 *      before the information that you want, this function returns the contents of
 *      said tag.
 */
QString SmogonScraper::getContents(QString htmlBuild, int start)
{
    while(htmlBuild.at(start) != QChar('>'))
    {
        start++;
        if(htmlBuild.length() == start)
            return NULL;
    }
    start++;
    QString retString = htmlBuild.right(htmlBuild.length()-start);
    retString = retString.left(retString.indexOf("<"));
    return retString;
}

/*This function, given the htmlBuild finds the Smogon style EV string and returns it*/
QString SmogonScraper::getEVString(QString htmlBuild)
{
    int start = htmlBuild.lastIndexOf(this->currGen + "/moves/");

    QString tempLoc = htmlBuild.right(htmlBuild.length() - start);

    start = tempLoc.lastIndexOf("<td>");

    QString EVString = getContents(tempLoc, start);
    return EVString;
}

/*
 *  takes the html webpage and returns a list of strings that encapsulate all the data for a
 *      specific build.
 */
QList<QString> *SmogonScraper::getHtmlBuilds(QString webPage, int numBuilds)
{
    QList<QString> *retList = new QList<QString>();

    for(int i=0;i<numBuilds;i++)
    {
        int begin = webPage.indexOf("class=\"name\"><h2>");
        //17, the length of the above string
        webPage = webPage.right(webPage.length()-begin-17);
        int end = webPage.indexOf("/div");
        QString subsection = webPage.left(end);
        retList->push_back(subsection);
    }
    return retList;
}

void SmogonScraper::reciever(QNetworkReply* reply)
{

    QString webPage = QString(reply->readAll());

    QList<smogonbuild> *builds = parsePage(webPage);

    if(builds->size() == 0)
        builds = NULL;

    uiTab->createInitialUi(builds);
}

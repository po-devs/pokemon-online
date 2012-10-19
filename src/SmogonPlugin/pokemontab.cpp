#include "pokemontab.h"
#include "smogonscraper.h"

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Create a text box with scraped information */
    mainLayout = new QVBoxLayout;

    SmogonScraper *scraper = new SmogonScraper(this);
    scraper->lookup(m_gen, p);

    QLabel *simpleText = new QLabel("No Builds");
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}

void PokemonTab::createInitialUi(QList<SmogonBuild> *builds)
{
    PokemonTab::allBuilds = builds;
    //TODO: add call to initial ui setup

    /*For debugging purposes*/
    QString labelTxt="";
    if(builds == NULL)
        labelTxt = "Nothing found";
    else
    {
        labelTxt += builds->at(0).move1->at(0).toAscii().data();
        labelTxt += "~";
        labelTxt += builds->at(0).move2->at(0).toAscii().data();
        labelTxt += "~";
        labelTxt += builds->at(0).move3->at(0).toAscii().data();
        labelTxt += "~";
        labelTxt += builds->at(0).move4->at(0).toAscii().data();
    }
    QLabel *simpleText = new QLabel(labelTxt);
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}

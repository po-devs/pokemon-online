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
        foreach(SmogonBuild build, *builds)
        {
            printf("\n\n\n\n");
            build.printBuild();
        }
    }
    QLabel *simpleText = new QLabel(labelTxt);
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}



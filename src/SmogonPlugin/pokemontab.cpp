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

void PokemonTab::update_ui(SmogonBuild *builds)
{
    QLabel *simpleText = new QLabel(builds[0].description);
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}

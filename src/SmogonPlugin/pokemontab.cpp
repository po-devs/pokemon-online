#include "pokemontab.h"
#include "smogonscraper.h"
#include <QtGui>

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Create a text box with scraped information */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    BuildObject* builds = SmogonScraper.get(m_gen, p);

    QLabel *simpleText = new QLabel(builds[0].description);
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}

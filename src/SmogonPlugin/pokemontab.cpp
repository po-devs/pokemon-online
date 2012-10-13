#include "pokemontab.h"
#include <QtGui>

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Create a text box with scraped information */
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QLabel *simpleText = new QLabel("Scraper Info Goes here");
    mainLayout->addWidget(simpleText);
    setLayout(mainLayout);
}

#include "pokemontab.h"
#include <QtGui>

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Create a text box with scraped information */
    QGridLayout *mainLayout = new QGridLayout;

    /* Fonts */
    QFont title_font("Arial", 12, QFont::Bold);

    /* Choose Build */
    QLabel *build_title = new QLabel("Build:");
    build_title -> setFont(title_font);

    QComboBox *build_chooser = new QComboBox();
    build_chooser -> addItem("");
    build_chooser -> addItem("Solar Power");
    build_chooser -> addItem("Choice");

    /* Display the Pokemon Image */
    QLabel *pokePic = new QLabel;
    pokePic -> setPixmap(p.picture());

    /* Item */ 
    QLabel *item_title = new QLabel("Item:");
    item_title -> setFont(title_font);

    QComboBox *item_chooser = new QComboBox();
    item_chooser -> addItem("");
    item_chooser -> addItem("Choice Scarf");
    item_chooser -> addItem("Choice Specs");

    /* Abilty */
    QLabel *ability_title = new QLabel("Ability:");
    ability_title -> setFont(title_font);
    
    /* Nature */
    QLabel *nature_title = new QLabel("Nature:");
    nature_title -> setFont(title_font);

    /* EV */
    QLabel *ev_title = new QLabel("EV:");
    ev_title -> setFont(title_font);

    /* Moves */
    QLabel *moves_title = new QLabel("Moves:");
    moves_title -> setFont(title_font);

    /* Desctiption */
    QLabel *description_title = new QLabel("Description:");
    description_title -> setFont(title_font);

    QLabel *description = new QLabel("test\n");

    /* Add the widgets to the main layout */
    mainLayout->addWidget(build_title,0,0);
    mainLayout->addWidget(build_chooser,1,0);
    mainLayout->addWidget(pokePic,1,2);
    mainLayout->addWidget(item_title,2,0);
    mainLayout->addWidget(ability_title,3,0);
    mainLayout->addWidget(nature_title,4,0);
    mainLayout->addWidget(ev_title,5,0);
    mainLayout->addWidget(moves_title,6,0);
    mainLayout->addWidget(description_title,7,0);
    mainLayout->addWidget(description,8,0);

    setLayout(mainLayout);
}

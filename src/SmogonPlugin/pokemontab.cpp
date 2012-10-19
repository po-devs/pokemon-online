#include "pokemontab.h"
#include <QtGui>

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Save off the original team */
    originalPokemon = p;

    /* Create a text box with scraped information */
    QGridLayout *mainLayout = new QGridLayout;

    /* Fonts */
    QFont title_font("Arial", 12, QFont::Bold);
    

    /* Display the Pokemon Image */
    QLabel *pokePic = new QLabel;
    pokePic -> setPixmap(p.picture());

    /* Choose Build */
    QLabel *build_title = new QLabel("Build:");
    build_title -> setFont(title_font);

    build_chooser = new QComboBox();
    build_chooser -> addItem("");
    build_chooser -> addItem("Solar Power");
    build_chooser -> addItem("Choice");

    /* Item */ 
    QLabel *item_title = new QLabel("Item:");
    item_title -> setFont(title_font);

    item_chooser = new QComboBox();
    item_chooser -> addItem("");
    item_chooser -> addItem("Choice Scarf");
    item_chooser -> addItem("Choice Specs");

    /* Abilty */
    QLabel *ability_title = new QLabel("Ability:");
    ability_title -> setFont(title_font);

    ability_chooser = new QComboBox();
    ability_chooser -> addItem("");
    ability_chooser -> addItem("Overgrow");
    
    /* Nature */
    QLabel *nature_title = new QLabel("Nature:");
    nature_title -> setFont(title_font);

    nature_chooser = new QComboBox();
    nature_chooser -> addItem("");
    nature_chooser -> addItem("Adamant"); 

    /* EV */
    QLabel *ev_title = new QLabel("EV:");
    ev_title -> setFont(title_font);

    ev_chooser = new QComboBox();
    ev_chooser -> addItem("");
    ev_chooser -> addItem("252 HP / 4 SpA / 252 Spe"); 

    /* Moves */
    QLabel *moves_title = new QLabel("Moves:");
    moves_title -> setFont(title_font);

    move1_chooser = new QComboBox();
    move1_chooser -> addItem("");
    move1_chooser -> addItem("Flamethrower"); 

    move2_chooser = new QComboBox();
    move2_chooser -> addItem("");
    move2_chooser -> addItem("Iron Defense"); 

    move3_chooser = new QComboBox();
    move3_chooser -> addItem("");
    move3_chooser -> addItem("Bide"); 

    move4_chooser = new QComboBox();
    move4_chooser -> addItem("");
    move4_chooser -> addItem("Wrap"); 


    /* Description */
    QLabel *description_title = new QLabel("Description:");
    description_title -> setFont(title_font);

    description = new QLabel("(Some long description here)");

    int space = 10;

    /* Add the widgets to the main layout */
    mainLayout->addWidget(pokePic,0,0);
    mainLayout->addWidget(build_title,1,0);
    mainLayout->addWidget(build_chooser,1,1);
    mainLayout->addItem(new QSpacerItem(0, 50, QSizePolicy::Minimum, QSizePolicy::Expanding),2,0);

    mainLayout->addWidget(item_title,3,0); 
    mainLayout->addWidget(item_chooser,4,0);
    

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),5,0);

    mainLayout->addWidget(ability_title,6,0);
    mainLayout->addWidget(ability_chooser,7,0);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),8,0);

    mainLayout->addWidget(nature_title,9,0);
    mainLayout->addWidget(nature_chooser,10,0);
    

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),11,0);

    mainLayout->addWidget(ev_title,12,0);
    mainLayout->addWidget(ev_chooser,13,0); 

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),14,0);

    mainLayout->addWidget(moves_title,15,0);
    mainLayout->addWidget(move1_chooser,16,0);
    mainLayout->addWidget(move2_chooser,17,0);
    mainLayout->addWidget(move3_chooser,18,0);
    mainLayout->addWidget(move4_chooser,19,0);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),20,0);
    
    mainLayout->addWidget(description_title,21,0);
    mainLayout->addWidget(description,22,0);

    setLayout(mainLayout);
}

/* Given the selected options in the current UI, return a PokeTeam
 * representation of the build */
PokeTeam *PokemonTab::getPokeTeam(){
    PokeTeam *createdBuild = new PokeTeam;

    createdBuild -> setNum(originalPokemon.num());
    createdBuild -> setGen(originalPokemon.gen());

    #if 0
    /* Set held Item */
    PokeTeam.item() = createBuild.Item::number();

    /* Set Ability */
    PokeTeam.ability() = createBuild.Ability::number();
    
    /* Set Nature */
    PokeTeam.nature() = createBuild.Nature::number();
    
    /* Set EVs */
    PokemonTeam.setEV();
 
    /* Set Moves */
    for(int i=0; i<4; i++)
        createBuild.addMove(MoveInfo::number(), false);

    #endif

    return createdBuild;
}

/* Given the selected options in the current UI */
void PokemonTab::updateUI(){

    /* Update Item */
    /* Update Ability */
    /* Update Nature */
    /* Update EVs */
    /* Update Moves */
    /* Update Description */
}

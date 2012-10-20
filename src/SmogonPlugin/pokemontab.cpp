#include "pokemontab.h"
#include <QtGui>
#include "smogonscraper.h"

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
    /*build_chooser -> addItem("Solar Power");
    build_chooser -> addItem("Choice");
    */

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
    description -> setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    description -> setMinimumSize(300, 400);

    int space = 10;

    /* Add the widgets to the main layout */
    mainLayout->addWidget(pokePic,0,0);
    mainLayout->addWidget(build_title,1,0);
    mainLayout->addWidget(build_chooser,2,0);
    mainLayout->addItem(new QSpacerItem(0, 50, QSizePolicy::Minimum, QSizePolicy::Expanding),3,0);

    mainLayout->addWidget(item_title,4,0); 
    mainLayout->addWidget(item_chooser,5,0);
    

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),6,0);

    mainLayout->addWidget(ability_title,7,0);
    mainLayout->addWidget(ability_chooser,8,0);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),9,0);

    mainLayout->addWidget(nature_title,10,0);
    mainLayout->addWidget(nature_chooser,11,0);
    

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),12,0);

    mainLayout->addWidget(ev_title,13,0);
    mainLayout->addWidget(ev_chooser,14,0); 

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),15,0);

    mainLayout->addWidget(moves_title,16,0);
    mainLayout->addWidget(move1_chooser,17,0);
    mainLayout->addWidget(move2_chooser,18,0);
    mainLayout->addWidget(move3_chooser,19,0);
    mainLayout->addWidget(move4_chooser,20,0);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding),21,0);
    
    mainLayout->addWidget(description_title,22,0);
    mainLayout->addWidget(description,23,0);

    /* Make the column stretchable */
    mainLayout -> setColumnStretch( 0, 1 ) ;

    /* Create a new scraper */
    SmogonScraper *scraper = new SmogonScraper(this);
    scraper->lookup(m_gen, p);

    /* Make the UI update when a differnt build is chosen */
    connect(build_chooser, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
 
    setLayout(mainLayout);
}

PokemonTab::~PokemonTab()
{
    delete build_chooser;
    delete item_chooser;
    delete ability_chooser;
    delete nature_chooser;
    delete ev_chooser;
    delete move1_chooser;
    delete move2_chooser;
    delete move3_chooser;
    delete move4_chooser;
    delete description;
    delete allBuilds;
}

void PokemonTab::createInitialUi(QList<SmogonBuild> *builds)
{
    PokemonTab::allBuilds = builds;
    
    if(builds == NULL)
        return;
    else
    {
        foreach(SmogonBuild build, *builds)
        {
            build.printBuild();
            build_chooser -> addItem(build.buildName);
        }
    }
   
   updateUI();
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

    /* Get the current build */
    int buildNum = build_chooser -> currentIndex() - 1;
    
    /* Default build, clear everything */
    if(buildNum <0){
        item_chooser -> clear();
        ability_chooser -> clear();
        nature_chooser -> clear();
        move1_chooser -> clear();
        move2_chooser -> clear();
        move3_chooser -> clear();
        move4_chooser -> clear();
        description -> clear();
        return;
    }

    SmogonBuild currentBuild = allBuilds -> at(buildNum);

    /* Update item chooser based on current build */
    item_chooser -> clear();
    foreach(QString item , *currentBuild.item){
        item_chooser -> addItem(item);
    }

    /* Update Ability */
    ability_chooser -> clear();
    foreach(QString ability , *currentBuild.ability){
        ability_chooser -> addItem(ability);
    }

    /* Update Nature */
    nature_chooser -> clear();
    foreach(QString nature , *currentBuild.nature){
        nature_chooser -> addItem(nature);
    }

    /* Update Moves */
    move1_chooser -> clear();
    foreach(QString move1 , *currentBuild.move1){
        move1_chooser -> addItem(move1);
    }

    move2_chooser -> clear();
    foreach(QString move2 , *currentBuild.move2){
        move2_chooser -> addItem(move2);
    }
    
    move3_chooser -> clear();
    foreach(QString move3 , *currentBuild.move3){
        move3_chooser -> addItem(move3);
    }
    
    move4_chooser -> clear();
    foreach(QString move4 , *currentBuild.move4){
        move4_chooser -> addItem(move4);
    }
    
    /* Update Description */
    description -> setText((allBuilds -> at(buildNum)).description);
    description -> adjustSize();

    QCoreApplication::processEvents();
}



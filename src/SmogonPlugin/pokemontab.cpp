#include "pokemontab.h"
#include <QtGui>
#include "smogonscraper.h"

PokemonTab::PokemonTab(PokeTeam p, Pokemon::gen m_gen, QWidget *parent) 
    : QWidget(parent)
{
    /* Save off the original team */
    originalPokemon = p;

    /* Create a text box with scraped information */
    //QGridLayout *mainLayout = new QGridLayout;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    
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

    /* Item */ 
    QLabel *item_title = new QLabel("Item:");
    item_title -> setFont(title_font);
    item_chooser = new QComboBox();

    /* Abilty */
    QLabel *ability_title = new QLabel("Ability:");
    ability_title -> setFont(title_font);
    ability_chooser = new QComboBox();
    
    /* Nature */
    QLabel *nature_title = new QLabel("Nature:");
    nature_title -> setFont(title_font);
    nature_chooser = new QComboBox();

    /* EV */
    QLabel *ev_title = new QLabel("EV:");
    ev_title -> setFont(title_font);
    ev_chooser = new QComboBox();

    /* Moves */
    QLabel *moves_title = new QLabel("Moves:");
    moves_title -> setFont(title_font);
    move1_chooser = new QComboBox();
    move2_chooser = new QComboBox();
    move3_chooser = new QComboBox();
    move4_chooser = new QComboBox();


    /* Description */
    QLabel *description_title = new QLabel("Description:");
    description_title -> setFont(title_font);

    description = new QLabel();
    description -> setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    description -> setMinimumSize(400, 400);
    description -> setWordWrap(true);
    description -> setAlignment(Qt::AlignTop);   
    description -> setAlignment(Qt::AlignLeft);   
 
    /* Spacer between sections */
    int space = 10;

    mainLayout->addWidget(pokePic);
    mainLayout->addWidget(build_title);
    mainLayout->addWidget(build_chooser);
    mainLayout->addItem(new QSpacerItem(0, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mainLayout->addWidget(item_title); 
    mainLayout->addWidget(item_chooser);
    

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mainLayout->addWidget(ability_title);
    mainLayout->addWidget(ability_chooser);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mainLayout->addWidget(nature_title);
    mainLayout->addWidget(nature_chooser);
    
    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mainLayout->addWidget(ev_title);
    mainLayout->addWidget(ev_chooser); 

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mainLayout->addWidget(moves_title);
    mainLayout->addWidget(move1_chooser);
    mainLayout->addWidget(move2_chooser);
    mainLayout->addWidget(move3_chooser);
    mainLayout->addWidget(move4_chooser);

    mainLayout->addItem(new QSpacerItem(0, space, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    mainLayout->addWidget(description_title);
    mainLayout->addWidget(description);

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

void PokemonTab::createInitialUi(QList<smogonbuild> *builds)
{
    PokemonTab::allBuilds = builds;
    
    if(builds == NULL)
        return;
    else
    {
        foreach(smogonbuild build, *builds)
        {
#ifndef QT5
            build.printBuild();
#endif
            build_chooser -> addItem(build.buildName);
        }
    }
   
   updateUI();
}

/* Given the selected options in the current UI, return a PokeTeam
 * representation of the build */
PokeTeam *PokemonTab::getPokeTeam(){
    
    /* If no build selected, return null */
    int buildNum = build_chooser -> currentIndex() - 1;
    if(buildNum<0)
        return NULL;
 
    /* Build that we are generating */
    PokeTeam *createdBuild = new PokeTeam;

    createdBuild -> setNum(originalPokemon.num());
    createdBuild -> setGen(originalPokemon.gen());

    /* Set held Item */
    createdBuild -> item() = ItemInfo::Number(item_chooser -> currentText());

    /* Set Ability */
    createdBuild -> ability() = AbilityInfo::Number(ability_chooser -> currentText());
    
    /* Set Nature */
    createdBuild -> nature() = NatureInfo::Number(nature_chooser -> currentText()); 
    /* Set EVs */
    smogonbuild currentBuild = allBuilds -> at(buildNum);
    QList<int> *EVList = currentBuild.EVList;
    for(int i = 0; i<6; i++)
        createdBuild -> setEV(i, EVList -> at(i));
 
    /* Set Move 1 */
    QString moveName = move1_chooser->currentText();
    QString hiddenPowerType = "None";
    if(moveName.split(" ").at(0) == "Hidden" && moveName.split(" ").at(1) == "Power")
    {
        hiddenPowerType = moveName.split(" ").at(2);
        moveName = "Hidden Power";
    }
    createdBuild -> addMove(MoveInfo::Number(moveName), false);

    /* Set Move 2 */
    moveName = move2_chooser->currentText();
    if(moveName.split(" ").at(0) == "Hidden" && moveName.split(" ").at(1) == "Power")
    {
        hiddenPowerType = moveName.split(" ").at(2);
        moveName = "Hidden Power";
    }
    createdBuild -> addMove(MoveInfo::Number(moveName), false);

    /* Set Move 3 */
    moveName = move3_chooser->currentText();
    if(moveName.split(" ").at(0) == "Hidden" && moveName.split(" ").at(1) == "Power")
    {
        hiddenPowerType = moveName.split(" ").at(2);
        moveName = "Hidden Power";
    }
    createdBuild -> addMove(MoveInfo::Number(moveName), false);

    /* Set Move 4 */
    moveName = move4_chooser->currentText();
    if(moveName.split(" ").at(0) == "Hidden" && moveName.split(" ").at(1) == "Power")
    {
        hiddenPowerType = moveName.split(" ").at(2);
        moveName = "Hidden Power";
    }
    createdBuild -> addMove(MoveInfo::Number(moveName), false);

    /* Set DVs if we need to */
    if(hiddenPowerType != "None")
    {
        QStringList dvs = getDVList(hiddenPowerType);
        for(int i = 0; i< dvs.size(); i++){
            createdBuild -> setDV(i, dvs[i].toInt());
        }
    }

    return createdBuild;
}

/* Given the selected options in the current UI */
void PokemonTab::updateUI(){

    /* Get the current build */
    int buildNum = build_chooser -> currentIndex() - 1;
    
    /* No selected build, clear everything */
    if(buildNum <0){
        item_chooser    -> clear();
        ability_chooser -> clear();
        nature_chooser  -> clear();
        ev_chooser      -> clear();
        move1_chooser   -> clear();
        move2_chooser   -> clear();
        move3_chooser   -> clear();
        move4_chooser   -> clear();
        description -> setText("No build selected...");
        description -> adjustSize();
        return;
    }

    smogonbuild currentBuild = allBuilds -> at(buildNum);

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

    /* Update EV */
    ev_chooser -> clear();
    ev_chooser -> addItem(currentBuild.EVListToString());

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




QStringList PokemonTab::getDVList(QString typeString)
{
    QStringList retList;

    int type = 0;
    if(typeString == "Fighting")
        type = 1;
    else if(typeString == "Flying")
        type = 2;
    else if(typeString == "Poison")
        type = 3;
    else if(typeString == "Ground")
        type = 4;
    else if(typeString == "Rock")
        type = 5;
    else if(typeString == "Bug")
        type = 6;
    else if(typeString == "Ghost")
        type = 7;
    else if(typeString == "Steel")
        type = 8;
    else if(typeString == "Fire")
        type = 9;
    else if(typeString == "Water")
        type = 10;
    else if(typeString == "Grass")
        type = 11;
    else if(typeString == "Electric")
        type = 12;
    else if(typeString == "Psychic")
        type = 13;
    else if(typeString == "Ice")
        type = 14;
    else if(typeString == "Dragon")
        type = 15;
    else if(typeString == "Dark")
        type = 16;
    else if(typeString == "Curse")
        type = 17;

    QList<QStringList> possibilities = HiddenPowerInfo::PossibilitiesForType(type);
    retList = possibilities.at(0);

    return retList;
}

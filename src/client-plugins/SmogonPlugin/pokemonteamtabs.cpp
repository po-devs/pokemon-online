#include "pokemonteamtabs.h"
#include <QtGui>

PokemonTeamTabs::PokemonTeamTabs(QString saveFilePath) 
    : QTabWidget()
{
    pokemonTabs = new QList<PokemonTab*>();
    filePath = saveFilePath;
}

void PokemonTeamTabs::addPokeTab(PokemonTab* page, const QString & label){
    /* Add a pointer to the tab locally */
    pokemonTabs -> append(page);

    /* Adds the page to the view */   
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea -> setWidget(page);
    addTab(scrollArea, label);
}

Team* PokemonTeamTabs::getTeam(){
    Team* ret = new Team();
    ret -> setName("SmogonTeam");

    for(int i = 0; i<6; i++){
        PokemonTab* tab = pokemonTabs->value(i);
        if(tab) {
            PokeTeam* currentPoke = tab->getPokeTeam();
            if (currentPoke) {
                ret -> poke(i) = *currentPoke;
            }
        }
    }
    printf("Saving to file...");
    
    ret -> saveToFile(filePath);

    return ret;
}

#include "dockinterface.h"
#include "../Teambuilder/teambuilder.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Teambuilder/advanced.h"
#include "../Teambuilder/mainwindow.h"

//class dockAdvanced
DockAdvanced::DockAdvanced(TeamBuilder * builder):
	QDockWidget(tr("Advanced Pokemons"),builder),m_builder(builder),lastStackIndex(-1)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    //parametre(s) interne(s)
    this->setObjectName("dockAdvanced");
    this->setWindowIcon(QIcon("db/icon.png"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    //affichage des interfaces Advanced de chaque pokemon
    AdvancedPokemons_gestionnaire = new QStackedWidget(this);
    connect(AdvancedPokemons_gestionnaire,SIGNAL(currentChanged(int)),this,SLOT(stackAdvancedChanged()));

    for(int i= 0; i<6;i++)
    {
	PokeTeam * p = &m_builder->getTeam()->poke(i);
	TB_Advanced * stack = new TB_Advanced(p);
	AdvancedPokemons_gestionnaire->addWidget(stack);
	TB_PokemonBody *body = builder->pokebody(i);

	connect(stack, SIGNAL(levelChanged()), body, SLOT(updateLevel()));
	connect(stack, SIGNAL(imageChanged()), body, SLOT(updateImage()));
	connect(stack, SIGNAL(genderChanged()), body, SLOT(updateGender()));
	connect(stack, SIGNAL(genderChanged()), body, SLOT(updateImage()));
	connect(stack, SIGNAL(statChanged()), body, SLOT(updateEVs()));
    }

    this->setWidget(AdvancedPokemons_gestionnaire);
}

DockAdvanced::~DockAdvanced()
{
}

void DockAdvanced::setCurrentPokemon(int index)
{
    lastStackIndex = AdvancedPokemons_gestionnaire->currentIndex();
    AdvancedPokemons_gestionnaire->setCurrentIndex(index);
}

void DockAdvanced::setPokemonNum(int indexStack,int pokeNum)
{
    std::cout <<"indexStack::"<<indexStack<<" pokeNum:"<<pokeNum << std::endl;
    QWidget * w = AdvancedPokemons_gestionnaire->widget(indexStack);
    AdvancedPokemons_gestionnaire->removeWidget(w);
    delete w;
    TB_Advanced * adv = new TB_Advanced(&m_builder->getTeam()->poke(indexStack));
    AdvancedPokemons_gestionnaire->insertWidget(indexStack,adv);
    if(pokeNum ==0)
    {
        adv->setHidden(true);
    }
    AdvancedPokemons_gestionnaire->setCurrentIndex(indexStack);
}

void DockAdvanced::stackAdvancedChanged()
{
    emit updateDataOfBody(lastStackIndex);
}

#include "dockinterface.h"
#include "../Teambuilder/teambuilder.h"
#include <QStackedWidget>
#include "../Pokemoninfo/pokemonstructs.h"
#include "../Teambuilder/advanced.h"
#include "../TeamBuilder/mainwindow.h"

//class dockAdvanced
dockAdvanced::dockAdvanced(TeamBuilder * builder,QWidget * parent):
        QDockWidget(tr("Advanced Pokemons"),parent),m_builder(builder),lastStackIndex(-1)
{
    //parametre(s) interne(s)
    this->setObjectName("dockAdvanced");
    this->setWindowIcon(QIcon("db/icon.png"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    //affichage des interfaces Advanced de chaque pokemon
    AdvancedPokemons_gestionnaire = new QStackedWidget(this);
    connect(AdvancedPokemons_gestionnaire,SIGNAL(currentChanged(int)),this,SLOT(stackAdvancedChanged(int)));
    MainWindow * m = qobject_cast<MainWindow *>(this->parent());
    if(m!=0)
    {
        for(int i= 0; i<6;i++)
        {
            PokeTeam * p = &m_builder->getTeam()->poke(i);
            TB_Advanced * stack = new TB_Advanced(p);
            AdvancedPokemons_gestionnaire->addWidget(stack);
        }
        //connect(this,SIGNAL(displayInterface(int)),AdvancedPokemons_gestionnaire,SLOT(setCurrentIndex(int)));
    }
    else
    {
        QMessageBox::warning(0,tr("error dockAvanced"),tr("Can't create tabs for dockAdvanced"));
    }
    this->setWidget(AdvancedPokemons_gestionnaire);
}

dockAdvanced::~dockAdvanced()
{
}

void dockAdvanced::setCurrentPokemon(int index)
{
    lastStackIndex = AdvancedPokemons_gestionnaire->currentIndex();
    AdvancedPokemons_gestionnaire->setCurrentIndex(index);
}

void dockAdvanced::setPokemonNum(int indexStack,int pokeNum)
{
    qDebug() <<"indexStack::"<<indexStack<<" pokeNum:"<<pokeNum;
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

void dockAdvanced::stackAdvancedChanged(int stackIndex)
{
    emit updateDataOfBody(lastStackIndex);
}

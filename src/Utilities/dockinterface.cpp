#include "dockinterface.h"
#include "../Teambuilder/teambuilder.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Teambuilder/advanced.h"
#include "../Teambuilder/mainwindow.h"

//class dockAdvanced
DockAdvanced::DockAdvanced(TB_TeamBody * builder):
        QDockWidget(tr("Advanced Options"),builder), m_builder(builder)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(QIcon("db/icon.png"));
    setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    QLabel *bg = new QLabel();
    bg->setPixmap(QPixmap("db/Teambuilder/Advanced/AdvBG.png"));
    bg->setParent(this);
    bg->setFixedSize(bg->pixmap()->size());
    setFixedSize(bg->size());

    AdvStack = new QStackedWidget(bg);
    AdvStack->setFixedSize(311,495);

    for(int i= 0; i<6;i++)
    {
        PokeTeam * p = &m_builder->trainerTeam()->team().poke(i);
        TB_Advanced * stack = new TB_Advanced(p);
        AdvStack->addWidget(stack);
        TB_PokemonBody *body = builder->pokeBody[i];

        body->connectWithAdvanced(stack);
    }

    this->setWidget(AdvStack);
}

DockAdvanced::~DockAdvanced()
{
}

void DockAdvanced::setCurrentPokemon(int index)
{
    qDebug() << index;
    AdvStack->setCurrentIndex(index);
}

void DockAdvanced::setPokemonNum(int indexStack,int pokeNum)
{
    QWidget * w = AdvStack->widget(indexStack);
    AdvStack->removeWidget(w);
    delete w;
    TB_Advanced * adv = new TB_Advanced(&m_builder->trainerTeam()->team().poke(indexStack));
    AdvStack->insertWidget(indexStack,adv);
    if(pokeNum ==0)
    {
        adv->setHidden(true);
    }
    AdvStack->setCurrentIndex(indexStack);
    m_builder->pokeBody[indexStack]->connectWithAdvanced(adv);
}

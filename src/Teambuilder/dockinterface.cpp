#include "dockinterface.h"
#include "../Teambuilder/teambuilder.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Teambuilder/advanced.h"
#include "../Teambuilder/mainwindow.h"

//class dockAdvanced
DockAdvanced::DockAdvanced(TB_TeamBody * builder): m_builder(builder)
{
    setAttribute(Qt::WA_DeleteOnClose);

#if not defined(Q_OS_MACX)
    setWindowIcon(QIcon("db/icon.png"));
#endif
    setWindowTitle(tr("Advanced Options"));

    for(int i= 0; i<6;i++)
    {
        PokeTeam * p = &m_builder->trainerTeam()->team().poke(i);
        TB_Advanced * stack = new TB_Advanced(p);
        addWidget(stack);
        TB_PokemonBody *body = builder->pokeBody[i];

        body->connectWithAdvanced(stack);
    }
}

DockAdvanced::~DockAdvanced()
{
}

void DockAdvanced::setCurrentPokemon(int index)
{
    setCurrentIndex(index);
}

void DockAdvanced::changeGeneration(int)
{
    int index = currentIndex();
    for (int i = 0; i < 6; i++) {
        setPokemonNum(i, m_builder->trainerTeam()->team().poke(i).num());
    }
    setCurrentPokemon(index);
}

void DockAdvanced::setPokemonNum(int indexStack, Pokemon::uniqueId pokeNum)
{
    QWidget * w = widget(indexStack);
    removeWidget(w);
    delete w;
    TB_Advanced * adv = new TB_Advanced(&m_builder->trainerTeam()->team().poke(indexStack));
    insertWidget(indexStack,adv);
    if(pokeNum.toPokeRef() == Pokemon::NoPoke)
    {
        adv->setHidden(true);
    }
    setCurrentIndex(indexStack);
    m_builder->pokeBody[indexStack]->connectWithAdvanced(adv);
}

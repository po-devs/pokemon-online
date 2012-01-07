#include "dockinterface.h"
#include "teambuilder_old.h"
#include "../../PokemonInfo/pokemonstructs.h"
#include "advanced.h"
#include "../../Teambuilder/mainwindow.h"
#include "teambody.h"
#include "pokebody.h"
#include "teamholder.h"

//class dockAdvanced
DockAdvanced::DockAdvanced(TB_TeamBody * builder): m_builder(builder)
{
    std::fill(advanced, advanced+6, (QWidget*)NULL);

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

void DockAdvanced::createAdvanced(int i)
{
    PokeTeam * p = &m_builder->trainerTeam()->team().poke(i);
    TB_Advanced * stack = new TB_Advanced(p);
    addWidget(stack);
    TB_PokemonBody *body = m_builder->pokeBody[i];

    body->connectWithAdvanced(stack);

    advanced[i] = stack;
}

DockAdvanced::~DockAdvanced()
{
}

void DockAdvanced::setCurrentPokemon(int index)
{
    if (advanced[index] == NULL) {
        createAdvanced(index);
    }

    setCurrentWidget(advanced[index]);
}

void DockAdvanced::changeGeneration(int)
{
    int index = currentIndex();
    for (int i = 0; i < 6; i++) {
        updatePokemonNum(i);
    }
    setCurrentIndex(index);
}

void DockAdvanced::updatePokemonNum(int indexStack)
{
    QWidget * w = advanced[indexStack];
    removeWidget(w);
    delete w;

    createAdvanced(indexStack);
    setCurrentWidget(advanced[indexStack]);
}

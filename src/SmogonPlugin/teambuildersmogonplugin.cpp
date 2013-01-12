#include <QLayout>
#include <QToolButton>
#include "../Utilities/qverticalscrollarea.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "pokemontab.h"
#include "teambuildersmogonplugin.h"


TeambuilderSmogonPlugin::TeambuilderSmogonPlugin(TeambuilderInterface *tb)
{
    teambuilder = tb;
}

TeambuilderSmogonPlugin::~TeambuilderSmogonPlugin()
{
    if (poke) {
        delete poke.data()->topLevelWidget();
        poke = NULL;
    }
}

QHash<QString, TeambuilderPlugin::Hook> TeambuilderSmogonPlugin::getHooks()
{
    QHash<QString, Hook> ret;

    ret.insert("addPokeEditButton(QLayout*,PokeTeam*)", (Hook)(&TeambuilderSmogonPlugin::addPokeEditButton));

    return ret;
}

void TeambuilderSmogonPlugin::addPokeEditButton(QLayout *l, PokeTeam *p)
{
    QToolButton *button = new QToolButton();
    button->setIcon(PokemonInfo::Icon(Pokemon::Koffing));
    button->setToolTip(tr("Choose smogon build..."));
    l->addWidget(button);
    button->setProperty("pokemon", intptr_t(p));
    button->setIconSize(QSize(32,32));

    connect(button, SIGNAL(clicked()), SLOT(openSmogonWindow()));
}

void TeambuilderSmogonPlugin::openSmogonWindow()
{
    PokeTeam *p = (PokeTeam*)sender()->property("pokemon").value<intptr_t>();

    QVerticalScrollArea *scrollArea = new QVerticalScrollArea();

    poke = new PokemonTab(*p);
    scrollArea -> setWidget(poke);
    scrollArea->show();
    scrollArea->setAttribute(Qt::WA_DeleteOnClose);

//    /* Update teambuilder when poke is changed */
//    connect(poke.data(), SIGNAL(destroyed()), teambuilder, SLOT(updateCurrentTeamAndNotify()));
}

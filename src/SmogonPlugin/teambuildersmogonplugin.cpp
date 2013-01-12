#include <QLayout>
#include <QToolButton>
#include "../Utilities/qverticalscrollarea.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Teambuilder/teambuilderinterface.h"
#include "pokemontab.h"
#include "teambuildersmogonplugin.h"
#include "smogonsinglepokedialog.h"

TeambuilderSmogonPlugin::TeambuilderSmogonPlugin(TeambuilderInterface *tb)
{
    teambuilder = tb;
    pokeTeam = NULL;
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
    button->setToolTip(tr("Choose Smogon build..."));
    l->addWidget(button);
    button->setProperty("pokemon", intptr_t(p));
    button->setIconSize(QSize(32,32));
    button->setObjectName("smogon"); //for cssers...

    connect(button, SIGNAL(clicked()), SLOT(openSmogonWindow()));
}

void TeambuilderSmogonPlugin::openSmogonWindow()
{
    PokeTeam *p = pokeTeam = (PokeTeam*)sender()->property("pokemon").value<intptr_t>();

    SmogonSinglePokeDialog *d = new SmogonSinglePokeDialog();
    d->setPokemon(p);
    d->show();
    d->setAttribute(Qt::WA_DeleteOnClose);

    poke = d->getPokemonTab();

    connect(d, SIGNAL(accepted()), SLOT(updatePokemon()));
}

void TeambuilderSmogonPlugin::updatePokemon()
{
    *pokeTeam = *poke.data()->getPokeTeam();
    teambuilder->updateCurrentTeamAndNotify();
}

#ifndef TEAMBUILDERSMOGONPLUGIN_H
#define TEAMBUILDERSMOGONPLUGIN_H

#include <QPointer>
#include "../Teambuilder/plugininterface.h"

class QLayout;
class PokeTeam;
class PokemonTab;

class TeambuilderSmogonPlugin : public TeambuilderPlugin
{
    Q_OBJECT
public:
    TeambuilderSmogonPlugin(TeambuilderInterface *tb);
    ~TeambuilderSmogonPlugin();

    QHash<QString, TeambuilderPlugin::Hook> getHooks();

    /* Adds our button to the poke edit layout */
    void addPokeEditButton(QLayout*,PokeTeam*);
private slots:
    void openSmogonWindow();
    void updatePokemon();
private:
    QPointer<PokemonTab> poke;

    TeambuilderInterface *teambuilder;
    PokeTeam *pokeTeam;
};

#endif // TEAMBUILDERSMOGONPLUGIN_H

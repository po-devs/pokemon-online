#include "smogonplugin.h"
#include "../Teambuilder/engineinterface.h"
#include "../Teambuilder/Teambuilder/teamholderinterface.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include <QLabel>
#include <QPalette>
#include <QTabWidget>

ClientPlugin* createPluginClass(MainEngineInterface *interface)
{
    return new SmogonPlugin(interface);
}


SmogonPlugin::SmogonPlugin(MainEngineInterface *interface) : interface(interface)
{
}

bool SmogonPlugin::hasConfigurationWidget() const
{
    return true;
}

QString SmogonPlugin::pluginName() const
{
    return "Get Builds from Smogon";
}

QWidget *SmogonPlugin::getConfigurationWidget()
{

    QTabWidget* ret = new QTabWidget;
    
    /* Given the interface, get the pokemon team */
    Team team = interface->trainerTeam()->team();
    Pokemon::gen m_gen = team.gen();
    
    /* Insert multiple tabs */
    for(int i = 0; i<6; i++){
        PokeTeam current_poke = team.poke(i);
        ret->addTab(new QWidget(ret), PokemonInfo::Name(current_poke.num()));
    }

    return ret;
}


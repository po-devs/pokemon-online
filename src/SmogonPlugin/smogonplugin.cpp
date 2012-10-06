#include "smogonplugin.h"
#include "../Teambuilder/engineinterface.h"
#include "../Teambuilder/Teambuilder/teamholderinterface.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/otherwidgets.h"
#ifdef _WIN32
#include "../../SpecialIncludes/zlib.h"
#include "../../SpecialIncludes/qrencode.h"
#else
#include <zlib.h>
#include <qrencode.h>
#endif
#include <QLabel>
#include <QBitmap>
#include <QPalette>


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
    return "Get builds from Smogon";
}

QWidget *SmogonPlugin:getConfigurationWidget()
{

}


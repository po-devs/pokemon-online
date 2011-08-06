#include "qrcodeplugin.h"
#include "../Teambuilder/engineinterface.h"
#include "../PokemonInfo/pokemonstructs.h"
#include <QLabel>

ClientPlugin* createPluginClass(MainEngineInterface *interface)
{
    return new QRCodePlugin(interface);
}

QRCodePlugin::QRCodePlugin(MainEngineInterface *interface) : interface(interface)
{
}

bool QRCodePlugin::hasConfigurationWidget() const
{
    return true;
}

QString QRCodePlugin::pluginName() const
{
    return "Export team to QRCode";
}

QWidget *QRCodePlugin::getConfigurationWidget()
{
    TrainerTeam *team = interface->trainerTeam();
    QString name = team->team().poke(0).nickname();
    return new QLabel(QObject::tr("Your first pokemon is named %1").arg(name));
}

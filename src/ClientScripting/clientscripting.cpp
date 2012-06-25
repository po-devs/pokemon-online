#include "clientscripting.h"
#include "../Teambuilder/engineinterface.h"
#include <QtCore>

ClientPlugin* createPluginClass(MainEngineInterface*)
{
    return new ClientScripting();
}

ClientScripting::ClientScripting()
{
}

bool ClientScripting::hasConfigurationWidget() const
{
    return true;
}

QString ClientScripting::pluginName() const
{
    return QObject::tr("Scripts");
}

QWidget *ClientScripting::getConfigurationWidget()
{
    return NULL;
}

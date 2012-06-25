#include "clientscripting.h"
#include "../Teambuilder/engineinterface.h"
#include "scriptwindow.h"
#include "scriptengine.h"

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
    return QObject::tr("Script Window");
}

QWidget *ClientScripting::getConfigurationWidget()
{
    return new ScriptWindow();
}

OnlineClientPlugin* ClientScripting::getOnlinePlugin(ClientInterface *c)
{
    return new ScriptEngine(c);
}

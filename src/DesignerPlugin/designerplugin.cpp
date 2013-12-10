#include "designerplugin.h"
#include "designerwidget.h"

ClientPlugin* createPluginClass(MainEngineInterface *client)
{
    return new DesignerPlugin(client);
}

DesignerPlugin::DesignerPlugin(MainEngineInterface* client) : client(client)
{
}

QString DesignerPlugin::pluginName() const
{
    return QObject::tr("Designer Plugin");
}

bool DesignerPlugin::hasConfigurationWidget() const
{
    return true;
}

QWidget *DesignerPlugin::getConfigurationWidget()
{
    return new DesignerWidget(this);
}

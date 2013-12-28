#include "settingsplugin.h"
#include "settingsdialog.h"

ClientPlugin* createClientPlugin(MainEngineInterface *)
{
    return new SettingsPlugin();
}

SettingsPlugin::SettingsPlugin()
{
}

QString SettingsPlugin::pluginName() const
{
    return QObject::tr("Edit Settings");
}

bool SettingsPlugin::hasConfigurationWidget() const
{
    return true;
}

QWidget *SettingsPlugin::getConfigurationWidget()
{
    return new SettingsDialog();
}

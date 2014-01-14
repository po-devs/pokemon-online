#ifndef SETTINGSPLUGIN_H
#define SETTINGSPLUGIN_H

#include "SettingsPlugin_global.h"
#include "../Teambuilder/plugininterface.h"

extern "C" {
SETTINGSPLUGINSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class SETTINGSPLUGINSHARED_EXPORT SettingsPlugin : public ClientPlugin {
public:
    SettingsPlugin();

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    client_plugin_version()
};

#endif // SETTINGSPLUGIN_H

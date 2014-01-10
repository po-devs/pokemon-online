#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "ThemeManager_global.h"

#include "../Teambuilder/plugininterface.h"

extern "C" {
THEMEMANAGERSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class THEMEMANAGERSHARED_EXPORT ThemeManagerPlugin : public ClientPlugin {
public:
    ThemeManagerPlugin();

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    client_plugin_version()
};


#endif // THEMEMANAGER_H

#ifndef DESIGNERPLUGIN_H
#define DESIGNERPLUGIN_H

#include "DesignerPlugin_global.h"
#include "../Teambuilder/plugininterface.h"

extern "C" {
DESIGNERPLUGINSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class DESIGNERPLUGINSHARED_EXPORT DesignerPlugin : public ClientPlugin {
    friend class DesignerWidget;
public:
    DesignerPlugin(MainEngineInterface *client);

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    client_plugin_version()

private:
    MainEngineInterface *client;
};

#endif // DESIGNERPLUGIN_H

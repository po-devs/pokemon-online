#ifndef SMOGONPLUGIN_H
#define SMOGONPLUGIN_H

#include "SmogonPlugin_global.h"
#include "../Teambuilder/plugininterface.h"

extern "C" {
SMOGONPLUGINSHARED_EXPORT ClientPlugin *createPluginClass(MainEngineInterface*);
}

class SMOGONPLUGINSHARED_EXPORT SmogonPlugin : public QObject, public ClientPlugin {
public:
    SmogonPlugin(MainEngineInterface *interface);

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    TeambuilderPlugin *getTeambuilderPlugin(TeambuilderInterface *);
private:
    MainEngineInterface *interface;
};

#endif // SMOGONPLUGIN_H

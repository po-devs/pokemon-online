#ifndef CSSCHANGER_H
#define CSSCHANGER_H

#include "CSSChanger_global.h"

#include "../Teambuilder/plugininterface.h"

struct ThemeAccessor;

extern "C" {
CSSCHANGERSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class CSSCHANGERSHARED_EXPORT CSSPlugin : public ClientPlugin {
public:
    CSSPlugin(ThemeAccessor*);

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    client_plugin_version()
private:
    ThemeAccessor *theme;
};


#endif // CSSCHANGER_H

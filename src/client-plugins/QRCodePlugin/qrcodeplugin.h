#ifndef QRCODEPLUGIN_H
#define QRCODEPLUGIN_H

#include "QRCodePlugin_global.h"
#include "../Teambuilder/plugininterface.h"

extern "C" {
QRCODEPLUGINSHARED_EXPORT ClientPlugin *createPluginClass(MainEngineInterface*);
}

class QRCODEPLUGINSHARED_EXPORT QRCodePlugin : public ClientPlugin {
public:
    QRCodePlugin(MainEngineInterface *_interface);

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;
private:
    MainEngineInterface *_interface;
};

#endif // QRCODEPLUGIN_H

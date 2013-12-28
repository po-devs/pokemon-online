#ifndef CLIENTSCRIPTING_H
#define CLIENTSCRIPTING_H

#include "ClientScripting_global.h"
#include "../Teambuilder/plugininterface.h"
#include <QPointer>

class ScriptEngine;

extern "C" {
CLIENTSCRIPTINGSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class CLIENTSCRIPTINGSHARED_EXPORT ClientScripting : public ClientPlugin {
public:
    ClientScripting();

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();

    bool hasConfigurationWidget() const;

    OnlineClientPlugin *getOnlinePlugin(ClientInterface *);
private:
    QPointer<ScriptEngine> engine;
};


#endif // CLIENTSCRIPTING_H

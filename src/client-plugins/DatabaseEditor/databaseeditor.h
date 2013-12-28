#ifndef DATABASEEDITOR_H
#define DATABASEEDITOR_H

#include "databaseeditor_global.h"
#include "../Teambuilder/engineinterface.h"
#include "../Teambuilder/plugininterface.h"

extern "C" {
DATABASEEDITORSHARED_EXPORT ClientPlugin *createClientPlugin(MainEngineInterface*);
}

class DATABASEEDITORSHARED_EXPORT DatabaseEditor : public ClientPlugin
{

public:
    DatabaseEditor(MainEngineInterface *client);

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    QString pluginName() const;

    bool hasConfigurationWidget() const;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    QWidget * getConfigurationWidget();
};

#endif // DATABASEEDITOR_H

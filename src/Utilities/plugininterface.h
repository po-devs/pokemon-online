#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QString>
#include <QHash>

/* An interface Server Plugins must follow.
   Remember that plugins are still very experimental and that this file is going to be subject to
   a lot of changes... */

class QWidget;

class Plugin
{
public:
    virtual ~Plugin(){}

    static const int pluginVersion = 0;

    /* The name of the option the plugin would take in the menu bar.
       Also appears as the name of the plugin */
    virtual QString pluginName() const = 0;

    /* A widget that would be used to configure the plugin in the menu bar.
       Return NULL if you don't want one (default behavior) */
    virtual QWidget * getConfigurationWidget() {
        return NULL;
    }

    virtual bool hasConfigurationWidget() const {
        return false;
    }

    /* Some plugins may not be ready to be deleted instantly */
    virtual bool isReadyForDeletion() const {
        return true;
    }

    /* The version for that particular plugin class */
    virtual int version() const {
        return pluginVersion;
    }
};

/* Each plugin will have to have a function like that named
   createPluginClass, that creates a ServerEngine (or a derived
    class) through new and returns it. */
typedef Plugin *(*PluginInstanceFunction) ();

/* Will be used like that:

class MyPlugin : public Plugin
{
...
}

extern "C" {
Plugin *createPlugin();
};

....

Plugin *createPlugin() {
    return new MyPlugin();
}

*/

#endif // PLUGININTERFACE_H

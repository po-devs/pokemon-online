#ifndef BATTLESERVERPLUGINMANAGER_H
#define BATTLESERVERPLUGINMANAGER_H

#include "../Utilities/pluginmanager.h"

#include "plugininterface.h"

class BattlePlugin;
class BattleInterface;

namespace cross {
    class DynamicLibrary;
}

class BattleServerPluginManager : public PluginManager
{
public:
    BattleServerPluginManager();

    QList<BattlePlugin*> getBattlePlugins(BattleInterface *);

    BattleServerPlugin *plugin(int index) const;
protected:
    /* What settings file to use? */
    virtual QSettings& settings();
    /* How to instanciate the plugin? */
    virtual BattleServerPlugin* instanciatePlugin(void *function);
    /* What is the name of the function in the library to create the plugin? */
    virtual const char* instantiatingFunctionName() const {return "createBattleServerPlugin";}
    /* The version the plugins need to have */
    virtual int version() const;
private:
    QSettings m_settings;
};

#endif // BATTLESERVERPLUGINMANAGER_H

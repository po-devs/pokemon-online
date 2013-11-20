#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include "pluginmanager.h"
#include "plugininterface.h"
#include "../Utilities/CrossDynamicLib.h"

/* What settings file to use? */
virtual QSettings settings() = 0;
/* How to instanciate the plugin? */
virtual Plugin* instanciatePlugin(void *function);
/* What is the name of the function in the library to create the plugin? */
virtual const char* instantiatingFunctionName() const {return "createBattleServerPlugin";}
/* The version the plugins need to have */
virtual int version() const;

BattleServerPluginManager::BattleServerPluginManager() : m_settings("config_battleServer", QSettings::IniFormat)
{
    loadPlugins();
}

QSettings &BattleServerPluginManager::settings()
{
    return m_settings;
}

BattleServerPlugin *BattleServerPluginManager::instanciatePlugin(void *function)
{
    return dynamic_cast<BattleServerPlugin*>(((BattleServerPluginInstanceFunction)function)());
}

BattleServerPlugin *BattleServerPluginManager::plugin(index)
{
    return dynamic_cast<BattleServerPlugin*>(PluginManager::plugin(index));
}

QList<BattlePlugin*> BattleServerPluginManager::getBattlePlugins(BattleInterface *b)
{
    cleanPlugins();

    QList<BattlePlugin*> ret;

    for (int i = 0 ; i < count(); i++) {
        BattlePlugin *p = plugin(i)->getBattlePlugin(b);

        if (p) {
            ret.push_back(p);
        }
    }

    return ret;
}

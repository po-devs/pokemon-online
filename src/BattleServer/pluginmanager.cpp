#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>

#include "pluginmanager.h"
#include "plugininterface.h"
#include "../Utilities/CrossDynamicLib.h"

BattleServerPluginManager::BattleServerPluginManager() : m_settings("config_battleServer", QSettings::IniFormat)
{
    loadPlugins();
}

QSettings &BattleServerPluginManager::settings()
{
    return m_settings;
}

BattleServerPlugin *BattleServerPluginManager::plugin(int index) const
{
    return dynamic_cast<BattleServerPlugin*>(this->PluginManager::plugin(index));
}

BattleServerPlugin *BattleServerPluginManager::instanciatePlugin(void *function)
{
    return dynamic_cast<BattleServerPlugin*>(((BattleServerPluginInstanceFunction)function)());
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

#include "pluginmanager.h"
#include "../Utilities/CrossDynamicLib.h"

PluginManager::PluginManager()
{
    QSettings s;

    QStringList plugins = s.value("plugins").toStringList();

    foreach(QString plugin, plugins) {
        if (libraries.contains(plugin))
            continue;
        try {
            cross::DynamicLibrary *l = new cross::DynamicLibrary(plugin.toAscii().constData());
            libraries.insert(plugin, l);
        } catch (const std::exception &e) {
            qDebug() << "Error when loading plugin " << plugin <<  ": " << e.what();
        }
    }
}

PluginManager::~PluginManager()
{
    foreach(cross::DynamicLibrary *l, libraries) {
        delete l;
    }
    libraries.clear();
}

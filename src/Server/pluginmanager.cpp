#include "pluginmanager.h"
#include "plugininterface.h"
#include "../Utilities/CrossDynamicLib.h"

PluginManager::PluginManager()
{
    QSettings s;

    QStringList plugins = s.value("plugins").toStringList();
    plugins = plugins.toSet().toList(); /* Remove duplicates */

    foreach(QString plugin, plugins) {
        cross::DynamicLibrary *l;
        try {
             l = new cross::DynamicLibrary(plugin.toAscii().constData());
        } catch (const std::exception &e) {
            qDebug() << "Error when loading plugin " << plugin <<  ": " << e.what();
            continue;
        }

        libraries.push_back(l);
        PluginInstanceFunction f = (PluginInstanceFunction) l->GetFunction("createPluginClass");

        if (!f) {
            delete l;
            libraries.pop_back();
            continue;
        }

        ServerPlugin *s = f();

        if (!s) {
            delete l;
            libraries.pop_back();
            continue;
        }

        this->plugins.push_back(s);
    }
}

PluginManager::~PluginManager()
{
    foreach(ServerPlugin *s, plugins) {
        delete s;
    }
    plugins.clear();
    foreach(cross::DynamicLibrary *l, libraries) {
        delete l;
    }
    libraries.clear();
}

void PluginManager::battleStarting(PlayerInterface *p1, PlayerInterface *p2, const ChallengeInfo &c)
{
    foreach(ServerPlugin *s, plugins) {
        s->battleStarting(p1, p2, c);
    }
}

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>

namespace cross {
    class DynamicLibrary;
};

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();
private:
    QHash<QString, cross::DynamicLibrary *> libraries;
};

#endif // PLUGINMANAGER_H

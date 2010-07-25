#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore>
class PlayerInterface;
class ServerPlugin;
class ChallengeInfo;

namespace cross {
    class DynamicLibrary;
};

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    void battleStarting(PlayerInterface *p1, PlayerInterface *p2, const ChallengeInfo &c);
private:
    QVector<cross::DynamicLibrary *> libraries;
    QVector<ServerPlugin *> plugins;
};

#endif // PLUGINMANAGER_H

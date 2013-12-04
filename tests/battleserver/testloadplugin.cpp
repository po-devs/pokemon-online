#include <Shared/networkcommands.h>

#include "testloadplugin.h"

#define XSUFFIX(x) SUFFIX(x)
#define SUFFIX(x) #x
#define PLUGIN_SUFFIX XSUFFIX(EXE_SUFFIX)

void TestLoadPlugin::onBattleServerConnected()
{
#ifdef Q_OS_UNIX
    analyzer->notify(LoadPlugin, true, "serverplugins/libusagestats" PLUGIN_SUFFIX ".so");
#elif defined(Q_OS_WIN)
    analyzer->notify(LoadPlugin, true, "serverplugins/usagestats" PLUGIN_SUFFIX ".dll");
#else
#warning "Test not defined for current platform"
    accept();
    return;
#endif

    QTimer::singleShot(500, this, SLOT(checkPluginLoaded()));
}

void TestLoadPlugin::checkPluginLoaded()
{
    if (!getFileContent("config_battleServer").contains("usagestats")) {
        qDebug() << "Usage stats plugin not loaded";
        reject();
    }

    analyzer->notify(LoadPlugin, false, "Usage Statistics");
    QTimer::singleShot(500, this, SLOT(checkPluginUnloaded()));
}

void TestLoadPlugin::checkPluginUnloaded()
{
    if (getFileContent("config_battleServer").contains("usagestats")) {
        qDebug() << "Usage stats plugin not unloaded";
        reject();
    }

    accept();
}

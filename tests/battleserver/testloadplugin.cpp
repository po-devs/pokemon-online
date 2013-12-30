#include <Shared/networkcommands.h>
#include <Utilities/exesuffix.h>

#include "testloadplugin.h"

void TestLoadPlugin::onBattleServerConnected()
{
#ifdef Q_OS_UNIX
    analyzer->notify(LoadPlugin, true, "battleserverplugins/libusagestats" OS_LIB_SUFFIX);
#elif defined(Q_OS_WIN)
    analyzer->notify(LoadPlugin, true, "battleserverplugins/usagestats" OS_LIB_SUFFIX);
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

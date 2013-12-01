#ifndef TESTLOADPLUGIN_H
#define TESTLOADPLUGIN_H

#include "battleservertest.h"

class TestLoadPlugin : public BattleServerTest
{
    Q_OBJECT
public:
    void onBattleServerConnected();

public slots:
    void checkPluginLoaded();
    void checkPluginUnloaded();
};

#endif // TESTLOADPLUGIN_H

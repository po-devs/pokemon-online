#ifndef TESTDISCONNECTION_H
#define TESTDISCONNECTION_H

#include "testplayer.h"

class TestDisconnection : public TestPlayer
{
public:
    void onPlayerConnected();
    void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    void playerLogout(int id);
    void onPm(int id, const QString &message);
};

#endif // TESTDISCONNECTION_H

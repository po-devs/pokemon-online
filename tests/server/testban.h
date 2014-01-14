#ifndef TESTBAN_H
#define TESTBAN_H

#include "testplayer.h"

class TestBan : public TestPlayer
{
public:
    void run();
    void onPlayerConnected();
    void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    void onPlayerDisconnected();
    void onMessage(const QString&);
};

#endif // TESTBAN_H

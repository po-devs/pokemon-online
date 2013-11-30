#ifndef TESTVARIATION_H
#define TESTVARIATION_H

#include "testplayer.h"

class TestVariation : public TestPlayer
{
public:
    void run();

    void onPlayerConnected();
    void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    void onBattleMessage(int battle, const QByteArray &message);
};

#endif // TESTVARIATION_H

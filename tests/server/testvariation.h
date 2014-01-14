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
    void onBattleStarted(int, const Battle &b, const TeamBattle &t, const BattleConfiguration &conf);
private:
    Team team;
};

#endif // TESTVARIATION_H

#ifndef TESTRECONNECT_H
#define TESTRECONNECT_H

#include "testplayer.h"

class TestReconnect : public TestPlayer
{
public slots:
    void onPlayerConnected();
    void onPlayerDisconnected();
    void loggedIn(const PlayerInfo &info, const QStringList& tiers);
    void onReconnectSuccess();
    void onChannelMessage(const QString &message, int, bool);
};

#endif // TESTRECONNECT_H

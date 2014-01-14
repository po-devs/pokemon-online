#ifndef TESTSHUTDOWN_H
#define TESTSHUTDOWN_H

#include "testplayer.h"

class TestShutdown : public TestPlayer
{
public:
    void onPlayerConnected();
    void onPlayerDisconnected();
};

#endif // TESTSHUTDOWN_H

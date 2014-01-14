#ifndef TESTREGISTER_H
#define TESTREGISTER_H

#include "testplayer.h"
class TestRegister : public TestPlayer
{
public:
    void onPlayerConnected();
    void onPassRequired(const QByteArray &);
};

#endif // TESTREGISTER_H

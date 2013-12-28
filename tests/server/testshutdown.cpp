#include "testshutdown.h"

void TestShutdown::onPlayerConnected()
{
    setTimeout();

    sender()->login(TeamHolder("Shinigami"), false, false);
    sender()->sendChanMessage(0, "eval: sys.shutDown()");
}

void TestShutdown::onPlayerDisconnected()
{
    accept();
}

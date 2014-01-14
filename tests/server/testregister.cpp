#include "testregister.h"

void TestRegister::onPlayerConnected()
{
    sender()->login(TeamHolder("i am not registered"), false);
    sender()->notify(NetworkCli::Register);

    setTimeout();
}

void TestRegister::onPassRequired(const QByteArray &)
{
    accept();
}

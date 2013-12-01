#include "testregister.h"

void TestRegister::onPlayerConnected()
{
    sender()->login(TeamHolder("i am not registered"), false);
    sender()->notify(NetworkCli::Register);

    QTimer::singleShot(10000, this, SLOT(reject()));
}

void TestRegister::onPassRequired(const QByteArray &)
{
    accept();
}

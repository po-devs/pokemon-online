#include <QCoreApplication>
#include "pokemontestrunner.h"
#include "testchat.h"
#include "testdisconnection.h"
#include "testvariation.h"
#include "testregister.h"
#include "testban.h"
#include "testsession.h"
#include "testreconnect.h"
#include "testcolor.h"
#include "testshutdown.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PokemonTestRunner runner;
    runner.setName("server");
    runner.addTest(new TestChat());
    runner.addTest(new TestDisconnection());
    runner.addTest(new TestVariation());
    runner.addTest(new TestRegister());
    runner.addTest(new TestBan());
    runner.addTest(new TestSession());
    runner.addTest(new TestReconnect());
    runner.addTest(new TestColor());
    /* Always last test */
    runner.addTest(new TestShutdown());

    runner.start();

    return a.exec();
}

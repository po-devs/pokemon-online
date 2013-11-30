#include <QCoreApplication>
#include "testrunner.h"
#include "testchat.h"
#include "testdisconnection.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestRunner runner;
    runner.setName("server");
    runner.addTest(new TestChat());
    runner.addTest(new TestDisconnection());
    runner.start();

    return a.exec();
}

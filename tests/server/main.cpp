#include <QCoreApplication>
#include "testrunner.h"
#include "testchat.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestRunner runner;
    runner.setName("server");
    runner.addTest(new TestChat());
    runner.start();

    return a.exec();
}

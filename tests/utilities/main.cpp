#include <QCoreApplication>
#include "testrunner.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestRunner runner;
    runner.setName("utilities");
    runner.start();

    return a.exec();
}

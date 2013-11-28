#include <QCoreApplication>
#include "testrunner.h"
#include "testfunctions.h"
#include "testinsensitivemap.h"
#include "testrankingtree.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestRunner runner;
    runner.setName("utilities");
    runner.addTest(new TestInsensitiveMap());
    runner.addTest(new TestFunctions());
    runner.addTest(new TestRankingTree());
    runner.start();

    return a.exec();
}

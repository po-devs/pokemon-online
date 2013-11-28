#include <QCoreApplication>
#include <QTimer>
#include <iostream>

#include "test.h"
#include "testrunner.h"
using namespace std;

TestRunner::TestRunner(QObject *parent) :
    QObject(parent)
{
    name = "";
}

TestRunner::~TestRunner()
{
    qDeleteAll(tests);
    tests.clear();
}

void TestRunner::start()
{
    QTimer::singleShot(1000, this, SLOT(run()));
}

void TestRunner::addTest(Test *test)
{
    tests.append(test);
}

void TestRunner::run()
{
    cout << "Running tests for " << name << endl;

    foreach(Test* test, tests) {
        test->run();
        cout << ".";
    }

    cout << endl;
    cout << tests.count() << " tests passed" << endl;

    qApp->quit();
}

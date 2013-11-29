#include <QCoreApplication>
#include <QTimer>
#include <iostream>

#include "test.h"
#include "testrunner.h"
using namespace std;

TestRunner::TestRunner(QObject *parent) :
    QObject(parent), count(0)
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
    QTimer::singleShot(100, this, SLOT(run()));
}

void TestRunner::addTest(Test *test)
{
    tests.append(test);
}

void TestRunner::run()
{
    count = tests.size();

    cout << "Running tests for " << name << endl;

    runNext();
}

void TestRunner::printDot()
{
    cout << ".";
}

void TestRunner::runNext()
{
    if (tests.size() == 0) {
        cout << endl;
        cout << count << " tests passed" << endl;

        qApp->quit();
        return;
    }


    Test *test = tests.takeFirst();

    connect(test, SIGNAL(finished()), SLOT(printDot()));
    connect(test, SIGNAL(finished()), SLOT(runNext()));

    test->run();
}

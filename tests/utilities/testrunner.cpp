#include <QCoreApplication>
#include <QTimer>
#include <iostream>

#include "test.h"
#include "testrunner.h"
#include "testinsensitivemap.h"
#include "testfunctions.h"
#include "testrankingtree.h"

using namespace std;

TestRunner::TestRunner(QObject *parent) :
    QObject(parent)
{
    name = "";
    tests.append(new TestInsensitiveMap());
    tests.append(new TestFunctions());
    tests.append(new TestRankingTree());
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

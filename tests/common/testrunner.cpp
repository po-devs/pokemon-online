#include <QCoreApplication>
#include <QTimer>
#include <iostream>

#include "test.h"
#include "testrunner.h"
using namespace std;

TestRunner::TestRunner(QObject *parent) :
    QObject(parent), passed(0), errors(0), count(0)
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

void TestRunner::printFail()
{
    cout << "E";

    errors++;
}

void TestRunner::printDot()
{
    cout << ".";

    passed++;
}

void TestRunner::clearTest()
{
    assert(dynamic_cast<Test*>(sender()));

    /* Free memory from the test and prevent any more results from being received */
    disconnect(sender());
    sender()->deleteLater();
}

void TestRunner::runNext()
{
    if (tests.size() != count - errors - passed) {
        cout << endl;
        cout << "The last test finished without returning success or failure" << endl;

        ::exit(1);
    }
    if (tests.size() == 0) {
        cout << endl;
        cout << QString("%1, %2").arg(tr("%n test(s) passed", 0, passed))
                .arg(tr("%n test(s) failed", 0, errors)).toStdString() << endl;

        if (errors > 0) {
            ::exit(1);
        }

        qApp->quit();
        return;
    }

    Test *test = tests.takeFirst();

    connect(test, SIGNAL(success()), SLOT(printDot()));
    connect(test, SIGNAL(failure()), SLOT(printFail()));

    connect(test, SIGNAL(finished()), SLOT(clearTest()));
    connect(test, SIGNAL(finished()), SLOT(runNext()));

    test->run();
}

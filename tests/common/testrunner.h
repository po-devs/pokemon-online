#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include <QObject>
#include <QList>
#include "test.h"

class TestRunner : public QObject
{
    Q_OBJECT
public:
    explicit TestRunner(QObject *parent = 0);
    ~TestRunner();

    void start();
    void addTest(Test *test);

    void setName(const char *name){this->name = name;}
signals:
    
public slots:
    void run();
    void runNext();
    void printDot();
    void printFail();
    void clearTest();
private:
    QList<Test*> tests;
    const char *name;

    int passed;
    int errors;
    int count;
};

#endif // TESTRUNNER_H

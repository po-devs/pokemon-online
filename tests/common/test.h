#ifndef TEST_H
#define TEST_H

#include <cassert>
#include <QObject>

/* Usage:
 *
 * Override the run() method, run your tests (using assert, or exit
 * when result's not good), then emit finished() signal.
 */
class Test : public QObject
{
    Q_OBJECT
public:
    Test();
    virtual ~Test(){}

    virtual void run() = 0; //emit finished() in your sub implementation when run is over.
signals:
    void success();
    void failure();
    /* Don't call manually. Call success() or failure() */
    void finished();
};

#endif // TEST_H

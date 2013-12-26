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

    virtual void start();

    /* Method to override in your test. Use reject() or accept() to
     * conclude the test.
     *
     * By default, if you don't use accept() or reject(), the test is automatically
     * accepted when the run() function is ended.
    */
    virtual void run() = 0;
    /* If the time runs out, test fails */
    void setTimeout(int time=10);

public slots:
    void accept();
    void reject();
signals:
    void success();
    void failure();
    /* Don't call manually. Call success() or failure() */
    void finished();

private:
    bool _finished;
};

#endif // TEST_H

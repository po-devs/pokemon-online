#include <QTimer>
#include "test.h"

Test::Test() : _finished(false)
{
}

void Test::start()
{
    run();
    if (!_finished) {
        accept();
    }
}

void Test::setTimeout(int time)
{
    QTimer::singleShot(time*1000, this, SLOT(reject()));
}

void Test::accept()
{
    if (_finished) {
        return;
    }
    _finished = true;

    emit success();
    emit finished();
}

void Test::reject()
{
    if (_finished) {
        return;
    }
    _finished = true;

    emit failure();
    emit finished();
}

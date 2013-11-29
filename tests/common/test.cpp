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

void Test::accept()
{
    _finished = true;

    emit success();
    emit finished();
}

void Test::reject()
{
    _finished = true;

    emit failure();
    emit finished();
}

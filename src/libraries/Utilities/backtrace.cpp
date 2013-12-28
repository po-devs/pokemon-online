#include <qglobal.h>
#include "backtrace.h"

#ifdef Q_OS_LINUX

#include <unistd.h>
#include <execinfo.h>
#include <cstdio>

void dump_backtrace()
{
    const int size = 50;
    void * buffer[size];

    backtrace(buffer, size);
    backtrace_symbols_fd(buffer, size, STDERR_FILENO);
    fflush(stderr);
}

#else

void dump_backtrace() {
}

#endif

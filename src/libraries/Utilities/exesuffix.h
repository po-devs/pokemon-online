#ifndef EXESUFFIX_H
#define EXESUFFIX_H

#include <Qt>

#define XSUFFIX(x) #x
#define SUFFIX XSUFFIX(EXE_SUFFIX)

#ifdef Q_OS_LINUX
#define OS_EXE_SUFFIX (SUFFIX "")
#define OS_LIB_SUFFIX (SUFFIX ".so")
#elif defined(Q_OS_WIN)
#define OS_EXE_SUFFIX (SUFFIX ".exe")
#define OS_LIB_SUFFIX (SUFFIX ".dll")
#else
#error "unkown OS"
#endif

#endif // EXESUFFIX_H

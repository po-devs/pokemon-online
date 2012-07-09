#ifndef CONFIG_H
#define CONFIG_H

#include <qglobal.h>

#define VERSION QString("2.0.02a")

static const quint16 PROTOCOL_VERSION = 0;
static const quint16 PROTOCOL_SUBVERSION = 0;
static const quint16 CLIENT_VERSION_NUMBER = 2002;
static const int UPDATE_ID = 0;

#ifdef Q_OS_LINUX
#define OS "linux"
#elif defined(Q_OS_WIN)
#define OS "windows"
#elif defined(Q_OS_MAC)
#define OS "mac"
#endif

#endif // CONFIG_H

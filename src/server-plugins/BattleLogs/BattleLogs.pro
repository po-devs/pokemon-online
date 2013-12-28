#-------------------------------------------------
#
# Project created by QtCreator 2011-03-27T06:07:36
#
#-------------------------------------------------

TARGET = battlelogs
TEMPLATE = lib
DESTDIR = $$PWD/../../../bin/serverplugins
DEFINES += BATTLELOGS_LIBRARY
QT += gui core declarative

SOURCES += battlelogs.cpp \
    battleserverlog.cpp

HEADERS += battlelogs.h\
        BattleLogs_global.h \
    ../BattleServer/plugininterface.h \
    ../BattleServer/battleinterface.h \
    battleserverlog.h \
    ../Shared/battlecommands.h \
    ../Utilities/coreclasses.h

include(../../Shared/Common.pri)

LIBS += $$battlemanager

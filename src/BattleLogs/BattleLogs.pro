#-------------------------------------------------
#
# Project created by QtCreator 2011-03-27T06:07:36
#
#-------------------------------------------------

TARGET = battlelogs
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/serverplugins
DEFINES += BATTLELOGS_LIBRARY
QT += gui core declarative

SOURCES += battlelogs.cpp \
    battleserverlog.cpp

HEADERS += battlelogs.h\
        BattleLogs_global.h \
    ../Server/plugininterface.h \
    ../Server/battleinterface.h \
    battleserverlog.h \
    ../Shared/battlecommands.h \
    ../Utilities/coreclasses.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

LIBS += $$battlemanager

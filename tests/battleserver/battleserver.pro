#-------------------------------------------------
#
# Project created by QtCreator 2013-11-28T23:39:29
#
#-------------------------------------------------

QT += network

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
EXTRAS = test

DESTDIR = $$PWD/../../bin

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

include(../../src/Shared/Common.pri)

LIBS += $$battlemanager

TARGET = test-battleserver

SOURCES += main.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp \
    ../common/pokemontestrunner.cpp \
    battleservertest.cpp \
    ../../src/Server/battleanalyzer.cpp \
    testunrated.cpp \
    testteamcount.cpp \
    testdoubles.cpp \
    testimposter.cpp \
    testloadplugin.cpp

HEADERS += \
    ../common/testrunner.h \
    ../common/test.h \
    ../common/pokemontestrunner.h \
    battleservertest.h \
    ../../src/Server/battleanalyzer.h \
    testunrated.h \
    testteamcount.h \
    testdoubles.h \
    testimposter.h \
    testloadplugin.h

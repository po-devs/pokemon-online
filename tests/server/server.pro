#-------------------------------------------------
#
# Project created by QtCreator 2013-11-30T13:18:10
#
#-------------------------------------------------

QT       += core network

CONFIG   += console
CONFIG   -= app_bundle

EXTRAS = test

TEMPLATE = app

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

include(../../src/Shared/Common.pri)

LIBS += $$battlemanager

TARGET = test-server

SOURCES += main.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp \
    testplayer.cpp \
    ../../src/Teambuilder/analyze.cpp \
    testchat.cpp \
    testdisconnection.cpp \
    testvariation.cpp \
    ../common/pokemontestrunner.cpp \
    testregister.cpp \
    testban.cpp \
    testsession.cpp \
    testreconnect.cpp \
    testcolor.cpp \
    testshutdown.cpp

HEADERS += \
    ../common/test.h \
    ../common/testrunner.h \
    testplayer.h \
    ../../src/Teambuilder/analyze.h \
    testchat.h \
    testdisconnection.h \
    testvariation.h \
    ../common/pokemontestrunner.h \
    testregister.h \
    testban.h \
    testsession.h \
    testreconnect.h \
    testcolor.h \
    testshutdown.h

OTHER_FILES += \
    ../data/server/scripts.js

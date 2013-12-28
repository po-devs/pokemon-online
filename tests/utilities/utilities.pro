#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T13:51:13
#
#-------------------------------------------------

CONFIG   += console
CONFIG   -= app_bundle

EXTRAS = test

TEMPLATE = app

SOURCES += main.cpp \
    testinsensitivemap.cpp \
    testfunctions.cpp \
    testrankingtree.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

include(../../src/Shared/Common.pri)

LIBS += $$utilities

TARGET = test-utilities

HEADERS += \
    testinsensitivemap.h \
    testfunctions.h \
    testrankingtree.h \
    ../common/test.h \
    ../common/testrunner.h


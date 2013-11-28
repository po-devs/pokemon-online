#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T13:51:13
#
#-------------------------------------------------

CONFIG   += console
CONFIG   -= app_bundle

DESTDIR = $$PWD/../../bin
TEMPLATE = app


SOURCES += main.cpp \
    test.cpp \
    testinsensitivemap.cpp \
    testrunner.cpp \
    testfunctions.cpp \
    testrankingtree.cpp

LIBS += -L$$PWD/../../bin
INCLUDEPATH += ../../src/

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../../src/Shared/Common.pri)

TARGET = test-utilities

HEADERS += \
    test.h \
    testinsensitivemap.h \
    testrunner.h \
    testfunctions.h \
    testrankingtree.h


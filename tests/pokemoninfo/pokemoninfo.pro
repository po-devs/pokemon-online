#-------------------------------------------------
#
# Project created by QtCreator 2013-11-28T21:19:43
#
#-------------------------------------------------

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DESTDIR = $$PWD/../../bin

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../../src/Shared/Common.pri)

LIBS += $$pokemoninfo

unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}

TARGET = test-pokemoninfo

SOURCES += main.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp \
    testimportexportteam.cpp \
    pokemontestrunner.cpp \
    testiteminfo.cpp

HEADERS += \
    ../common/test.h \
    ../common/testrunner.h \
    testimportexportteam.h \
    pokemontestrunner.h \
    testiteminfo.h

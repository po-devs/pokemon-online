#-------------------------------------------------
#
# Project created by QtCreator 2011-01-24T20:46:54
#
#-------------------------------------------------
TARGET = EventCombinations
DESTDIR = $$PWD/../../bin
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += main.cpp

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

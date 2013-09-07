#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T18:38:46
#
#-------------------------------------------------

QT +=
TARGET = StatsExtracter
DESTDIR = $$PWD/../../bin

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
HEADERS = ../PokemonInfo/pokemoninfo.h \
          ../Utilities/coreclasses.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

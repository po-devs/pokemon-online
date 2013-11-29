#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T18:38:46
#
#-------------------------------------------------

TARGET = StatsExtracter
DESTDIR = $$PWD/../../bin

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
HEADERS = ../PokemonInfo/pokemoninfo.h \
          ../Utilities/coreclasses.h

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T18:38:46
#
#-------------------------------------------------

TARGET = StatsExtracter

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

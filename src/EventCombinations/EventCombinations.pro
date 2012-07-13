#-------------------------------------------------
#
# Project created by QtCreator 2011-01-24T20:46:54
#
#-------------------------------------------------
TARGET = EventCombinations
DESTDIR = ../../bin
LIBS += -L../../bin -lutilities -lpokemonlib
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += main.cpp

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

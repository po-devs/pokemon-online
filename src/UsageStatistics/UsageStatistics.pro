# -------------------------------------------------
# Project created by QtCreator 2010-07-22T23:36:49
# -------------------------------------------------
TARGET = usagestats
DESTDIR = ../../bin/serverplugins
TEMPLATE = lib
DEFINES += POKEMONONLINESTATSPLUGIN_LIBRARY
DEFINES += SERVER_SIDE
LIBS += -L../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities
SOURCES += usagestats.cpp
HEADERS += usagestats.h \
    usagestats_global.h \
    ../PokemonInfo/battlestructs.h \
    ../Server/plugininterface.h \
    ../Server/playerinterface.h \
    ../Server/serverinterface.h \
    ../Server/battleinterface.h

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

include(../Shared/Common.pri)

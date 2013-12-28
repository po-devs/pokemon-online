# -------------------------------------------------
# Project created by QtCreator 2010-07-22T23:36:49
# -------------------------------------------------
TARGET = usagestats
EXTRAS=serverplugin
TEMPLATE = lib
DEFINES += POKEMONONLINESTATSPLUGIN_LIBRARY
DEFINES += SERVER_SIDE
SOURCES += usagestats.cpp
HEADERS += usagestats.h \
    usagestats_global.h \
    ../PokemonInfo/battlestructs.h \
    ../BattleServer/plugininterface.h \
    ../BattleServer/battleinterface.h

include(../../Shared/Common.pri)

LIBS += $$pokemoninfo

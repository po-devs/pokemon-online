# -------------------------------------------------
# Project created by QtCreator 2010-07-23T02:42:41
# -------------------------------------------------
QT += network \
    sql \
    script
TARGET = maindll
TEMPLATE = lib
DESTDIR = ../../bin
DEFINES += MAINDLL_LIBRARY
DEFINES += SERVER_SIDE
DEFINES -= UNICODE
SOURCES += ../Server/waitingobject.cpp \
    ../Server/tiermachine.cpp \
    ../Server/tier.cpp \
    ../Server/sqlconfig.cpp \
    ../Server/sql.cpp \
    ../Server/serverwidget.cpp \
    ../Server/serverconfig.cpp \
    ../Server/server.cpp \
    ../Server/security.cpp \
    ../Server/scriptengine.cpp \
    ../Server/playerswindow.cpp \
    ../Server/player.cpp \
    ../Server/network.cpp \
    ../Server/moves.cpp \
    ../Server/mechanics.cpp \
    ../Server/mainwindow.cpp \
    ../Server/loadinsertthread.cpp \
    ../Server/items.cpp \
    ../Server/consolereader.cpp \
    ../Server/challenge.cpp \
    ../Server/berries.cpp \
    ../Server/battlingoptions.cpp \
    ../Server/battle.cpp \
    ../Server/antidos.cpp \
    ../Server/analyze.cpp \
    ../Server/abilities.cpp \
    ../PokemonInfo/pokemonstructs.cpp \
    ../PokemonInfo/pokemoninfo.cpp \
    ../PokemonInfo/networkstructs.cpp \
    ../PokemonInfo/movesetchecker.cpp \
    ../PokemonInfo/battlestructs.cpp \
    ../Utilities/otherwidgets.cpp \
    ../Utilities/mtrand.cpp \
    ../Utilities/functions.cpp \
    ../Utilities/contextswitch.cpp \
    ../Utilities/coro.c \
    ../Server/pluginmanager.cpp \
    ../Utilities/CrossDynamicLib.cpp
HEADERS += maindll.h \
    ../Server/waitingobject.h \
    ../Server/tiermachine.h \
    ../Server/tier.h \
    ../Server/sqlconfig.h \
    ../Server/sql.h \
    ../Server/serverwidget.h \
    ../Server/serverconfig.h \
    ../Server/server.h \
    ../Server/security.h \
    ../Server/scriptengine.h \
    ../Server/playerswindow.h \
    ../Server/player.h \
    ../Server/network.h \
    ../Server/moves.h \
    ../Server/memoryholder.h \
    ../Server/mechanics.h \
    ../Server/mainwindow.h \
    ../Server/loadinsertthread.h \
    ../Server/items.h \
    ../Server/consolereader.h \
    ../Server/challenge.h \
    ../Server/berries.h \
    ../Server/battlingoptions.h \
    ../Server/battle.h \
    ../Server/antidos.h \
    ../Server/analyze.h \
    ../Server/abilities.h \
    ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/networkstructs.h \
    ../PokemonInfo/movesetchecker.h \
    ../PokemonInfo/battlestructs.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/mtrand.h \
    ../Utilities/functions.h \
    ../Utilities/coro.h \
    ../Utilities/contextswitch.h \
    ../Server/pluginmanager.h \
    ../Utilities/CrossDynamicLib.h

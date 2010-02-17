# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network script
TARGET = Server
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    network.cpp \
    analyze.cpp \
    server.cpp \
    ../PokemonInfo/pokemoninfo.cpp \
    ../PokemonInfo/pokemonstructs.cpp \
    ../PokemonInfo/battlestructs.cpp \
    ../PokemonInfo/networkstructs.cpp \
    player.cpp \
    battle.cpp \
    moves.cpp \
    ../Utilities/functions.cpp \
    items.cpp \
    mechanics.cpp \
    abilities.cpp \
    security.cpp \
    ../Utilities/otherwidgets.cpp \
    ../Utilities/md5.c \
    playerswindow.cpp \
    antidos.cpp \
    serverconfig.cpp \
    berries.cpp \
    scriptengine.cpp
HEADERS += mainwindow.h \
    network.h \
    analyze.h \
    server.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    player.h \
    battle.h \
    moves.h \
    ../Utilities/functions.h \
    mechanics.h \
    items.h \
    abilities.h \
    security.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/md5.h \
    ../Utilities/thread_safe_containers.h \
    playerswindow.h \
    antidos.h \
    serverconfig.h \
    berries.h \
    scriptengine.h
LIBS += -L../../bin \
    -lzip
DEFINES = MULTI_THREADED_ACCESS \
    SERVER_SIDE

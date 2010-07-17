# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network \
    script \
    sql
TARGET = Server
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    network.cpp \
    analyze.cpp \
    server.cpp \
    serverwidget.cpp \
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
    scriptengine.cpp \
    challenge.cpp \
    ../PokemonInfo/movesetchecker.cpp \
    tier.cpp \
    ../Utilities/mtrand.cpp \
    battlingoptions.cpp \
    sql.cpp \
    sqlconfig.cpp \
    tiermachine.cpp \
    loadinsertthread.cpp \
    waitingobject.cpp \
    consolereader.cpp \
    ../Utilities/coro.c \
    ../Utilities/contextswitch.cpp
HEADERS += mainwindow.h \
    network.h \
    analyze.h \
    server.h \
    serverwidget.h \
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
    playerswindow.h \
    antidos.h \
    serverconfig.h \
    berries.h \
    scriptengine.h \
    challenge.h \
    ../PokemonInfo/movesetchecker.h \
    ../Shared/config.h \
    tier.h \
    ../Utilities/rankingtree.h \
    ../Utilities/rankingtree.h \
    ../Utilities/mtrand.h \
    battlingoptions.h \
    sql.h \
    waitingobject.h \
    sqlconfig.h \
    tiermachine.h \
    loadinsertthread.h \
    memoryholder.h \
    consolereader.h \
    ../Utilities/coro.h \
    ../Utilities/contextswitch.h
DEFINES = SERVER_SIDE
RESOURCES += 

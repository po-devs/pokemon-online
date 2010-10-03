# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network \
    script \
    xml \
    sql

TARGET = Server
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    consolereader.cpp \
    challenge.cpp \
    berries.cpp \
    battlingoptions.cpp \
    battle.cpp \
    antidos.cpp \
    player.cpp \
    network.cpp \
    mechanics.cpp \
    mainwindow.cpp \
    loadinsertthread.cpp \
    items.cpp \
    analyze.cpp \
    abilities.cpp \
    security.cpp \
    scriptengine.cpp \
    pluginmanager.cpp \
    playerswindow.cpp \
    server.cpp \
    sqlconfig.cpp \
    sql.cpp \
    serverwidget.cpp \
    serverconfig.cpp \
    waitingobject.cpp \
    tiermachine.cpp \
    tier.cpp \
    moves.cpp \
    channel.cpp \
    tiertree.cpp \
    tierwindow.cpp \
    confighelper.cpp
DEFINES = SERVER_SIDE
LIBS += -L../../bin \
    -lutilities \
    -lpokemonlib
HEADERS += player.h \
    network.h \
    moves.h \
    memoryholder.h \
    mechanics.h \
    mainwindow.h \
    loadinsertthread.h \
    items.h \
    consolereader.h \
    challenge.h \
    berries.h \
    battlingoptions.h \
    battle.h \
    antidos.h \
    analyze.h \
    abilities.h \
    security.h \
    scriptengine.h \
    pluginmanager.h \
    plugininterface.h \
    playerswindow.h \
    server.h \
    sqlconfig.h \
    sql.h \
    serverwidget.h \
    serverconfig.h \
    waitingobject.h \
    tiermachine.h \
    tier.h \
    playerinterface.h \
    ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/networkstructs.h \
    ../PokemonInfo/movesetchecker.h \
    ../PokemonInfo/battlestructs.h \
    ../Shared/config.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/mtrand.h \
    ../Utilities/functions.h \
    ../Utilities/CrossDynamicLib.h \
    ../Utilities/coro.h \
    ../Utilities/contextswitch.h \
    channel.h \
    tiertree.h \
    tierwindow.h \
    confighelper.h \
    tiernode.h \
    miscmoves.h

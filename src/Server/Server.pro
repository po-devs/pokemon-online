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
    battle.cpp \
    antidos.cpp \
    player.cpp \
    network.cpp \
    mechanics.cpp \
    loadinsertthread.cpp \
    items.cpp \
    analyze.cpp \
    abilities.cpp \
    security.cpp \
    scriptengine.cpp \
    pluginmanager.cpp \
    server.cpp \
    sql.cpp \
    waitingobject.cpp \
    tier.cpp \
    moves.cpp \
    channel.cpp \
    tiertree.cpp \
    tiermachine.cpp \
    sessiondatafactory.cpp \
    battlepluginstruct.cpp \
    battlecounters.cpp \
    moves/moves1.cpp
!CONFIG(nogui):SOURCES += mainwindow.cpp \
    playerswindow.cpp \
    sqlconfig.cpp \
    serverwidget.cpp \
    battlingoptions.cpp \
    tierwindow.cpp \
    serverconfig.cpp \
    confighelper.cpp
LIBS += -L../../bin \
    -lutilities \
    -lpokemonlib
HEADERS += player.h \
    network.h \
    moves.h \
    memoryholder.h \
    mechanics.h \
    loadinsertthread.h \
    items.h \
    consolereader.h \
    challenge.h \
    berries.h \
    battle.h \
    antidos.h \
    analyze.h \
    abilities.h \
    security.h \
    scriptengine.h \
    pluginmanager.h \
    plugininterface.h \
    server.h \
    sql.h \
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
    channel.h \
    tiertree.h \
    tiernode.h \
    ../Utilities/CrossDynamicLib.h \
    ../Utilities/coro.h \
    ../Utilities/contextswitch.h \
    ../Utilities/mtrand.h \
    miscmoves.h \
    sessiondatafactory.h \
    battleinterface.h \
    battlepluginstruct.h \
    miscabilities.h \
    serverinterface.h \
    sfmlsocket.h \
    ../Shared/networkcommands.h \
    battlecounters.h \
    battlecounterindex.h \
    battlefunctions.h
!CONFIG(nogui):HEADERS += mainwindow.h \
    battlingoptions.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/functions.h \
    playerswindow.h \
    serverwidget.h \
    serverconfig.h \
    sqlconfig.h \
    tierwindow.h \
    confighelper.h
CONFIG(nogui) { 
    QT -= gui
    DEFINES += PO_NO_GUI
}
CONFIG(sfml) { 
    DEFINES += SFML_SOCKETS
    SOURCES += sfmlsocket.cpp
    LIBS += -L/usr/local/lib \
        -lboost_system-mt
}
CONFIG(nowelcome):DEFINES += PO_NO_WELCOME
CONFIG(safeonlyscript):DEFINES += PO_SCRIPT_SAFE_ONLY
CONFIG(nosysteminscript):DEFINES += PO_SCRIPT_NO_SYSTEM




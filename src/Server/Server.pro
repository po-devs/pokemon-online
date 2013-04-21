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
    battlebase.cpp \
    battlerby.cpp \
    rbymoves.cpp \
    mechanicsbase.cpp \
    modswindow.cpp \
    relaymanager.cpp
!CONFIG(nogui):SOURCES += mainwindow.cpp \
    playerswindow.cpp \
    sqlconfig.cpp \
    serverwidget.cpp \
    battlingoptions.cpp \
    tierwindow.cpp \
    serverconfig.cpp
LIBS += -L../../bin \
    -lpo-utilities \
    -lpo-pokemoninfo
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
    battlefunctions.h \
    ../Shared/battlecommands.h \
    ../Utilities/coreclasses.h \
    playerstructs.h \
    networkutilities.h \
    battlebase.h \
    battlerby.h \
    rbymoves.h \
    mechanicsbase.h \
    modswindow.h \
    relaymanager.h \
    ../PokemonInfo/enums.h
!CONFIG(nogui):HEADERS += mainwindow.h \
    battlingoptions.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/functions.h \
    playerswindow.h \
    serverwidget.h \
    serverconfig.h \
    sqlconfig.h \
    tierwindow.h \
    ../Utilities/confighelper.h
CONFIG(nogui) { 
    QT -= gui
    DEFINES += PO_NO_GUI
}
CONFIG(sfml) { 
    DEFINES += SFML_SOCKETS
    SOURCES += sfmlsocket.cpp
    LIBS += -L/usr/local/lib \
        -lboost_system
}
CONFIG(nowelcome):DEFINES += PO_NO_WELCOME
CONFIG(safeonlyscript):DEFINES += PO_SCRIPT_SAFE_ONLY
CONFIG(nosysteminscript):DEFINES += PO_SCRIPT_NO_SYSTEM

macx {
   LIBS += -framework CoreFoundation
   # Todo: get a real icon
   #ICON = pokemononline.icns
   QMAKE_INFO_PLIST = Info.plist
   QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
   LINKLIBS = libpo-utilities.1.0.0.dylib libpo-pokemoninfo.1.0.0.dylib libpo-battlemanager.1.0.0.dylib
   QMAKE_POST_LINK = mkdir -p $${DESTDIR}/$${TARGET}.app/Contents/Frameworks;
   for(L, LINKLIBS) {
       QMAKE_POST_LINK += cp -f $${DESTDIR}/$${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/;
       QMAKE_POST_LINK += ln -s $${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/$$replace(L, 1.0.0, 1);
   }
   QMAKE_POST_LINK += macdeployqt $${DESTDIR}/$${TARGET}.app -verbose=3

}

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

include(../Shared/Common.pri)

FORMS += \
    modswindow.ui

CONFIG(debian_package) {
    DEFINES += PO_DATA_REPO=\\\"/usr/share/games/pokemon-online/\\\"
    DEFINES += PO_HOME_DIR=\\\"~/.po-server/\\\"
}
!CONFIG(debian_package) {
    DEFINES += PO_DATA_REPO=\\\"./\\\"
    DEFINES += PO_HOME_DIR=\\\"./\\\"
}

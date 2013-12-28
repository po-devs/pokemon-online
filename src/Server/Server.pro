# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network \
    script \
    xml \
    sql

TARGET = Server
TEMPLATE = app
SOURCES += main.cpp \
    consolereader.cpp \
    challenge.cpp \
    player.cpp \
    analyze.cpp \
    security.cpp \
    scriptengine.cpp \
    pluginmanager.cpp \
    server.cpp \
    waitingobject.cpp \
    tier.cpp \
    channel.cpp \
    tiertree.cpp \
    tiermachine.cpp \
    sessiondatafactory.cpp \
    modswindow.cpp \
    relaymanager.cpp \
    server.tpp \
    scriptengineagent.cpp \
    battlecommunicator.cpp \
    registrycommunicator.cpp \
    battleanalyzer.cpp \
    sql.cpp \
    sqlconfig.cpp
!CONFIG(nogui):SOURCES += mainwindow.cpp \
    playerswindow.cpp \
    serverwidget.cpp \
    battlingoptions.cpp \
    tierwindow.cpp \
    serverconfig.cpp

HEADERS += player.h \
    memoryholder.h \
    loadinsertthread.h \
    consolereader.h \
    challenge.h \
    analyze.h \
    security.h \
    scriptengine.h \
    pluginmanager.h \
    plugininterface.h \
    server.h \
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
    ../Utilities/mtrand.h \
    sessiondatafactory.h \
    serverinterface.h \
    ../Shared/networkcommands.h \
    ../Shared/battlecommands.h \
    ../Utilities/coreclasses.h \
    playerstructs.h \
    networkutilities.h \
    modswindow.h \
    relaymanager.h \
    ../PokemonInfo/enums.h \
    scriptengineagent.h \
    battlecommunicator.h \
    registrycommunicator.h \
    battleanalyzer.h \
    sql.h \
    sqlconfig.h
!CONFIG(nogui):HEADERS += mainwindow.h \
    battlingoptions.h \
    ../Utilities/otherwidgets.h \
    ../Utilities/functions.h \
    playerswindow.h \
    serverwidget.h \
    serverconfig.h \
    tierwindow.h \
    ../Utilities/confighelper.h
CONFIG(nogui) { 
    QT -= gui
    DEFINES += PO_NO_GUI
}

CONFIG(nowelcome):DEFINES += PO_NO_WELCOME
CONFIG(safeonlyscript):DEFINES += PO_SCRIPT_SAFE_ONLY
CONFIG(nosysteminscript):DEFINES += PO_SCRIPT_NO_SYSTEM

include(../Shared/Common.pri)

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

CONFIG(boost_asio) {
    DEFINES += BOOST_SOCKETS
    LIBS += -L/usr/local/lib \
        -lboost_system
}

LIBS += $$pokemoninfo

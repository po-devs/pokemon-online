#-------------------------------------------------
#
# Project created by QtCreator 2013-11-21T00:02:26
#
#-------------------------------------------------

QT       += core network

TARGET = BattleServer
CONFIG   += console
CONFIG   -= app_bundle

TARGET = BattleServer
DESTDIR = $$PWD/../../bin

TEMPLATE = app


SOURCES += main.cpp \
    rbymoves.cpp \
    mechanicsbase.cpp \
    mechanics.cpp \
    berries.cpp \
    battlerby.cpp \
    battlepluginstruct.cpp \
    battlecounters.cpp \
    battlebase.cpp \
    battle.cpp \
    abilities.cpp \
    battleserver.cpp \
    items.cpp \
    pluginmanager.cpp \
    moves.cpp \
    serverconnection.cpp \
    analyzer.cpp

HEADERS += \
    rbymoves.h \
    miscmoves.h \
    miscabilities.h \
    mechanicsbase.h \
    mechanics.h \
    berries.h \
    battlerby.h \
    battlepluginstruct.h \
    battleinterface.h \
    battlefunctions.h \
    battlecounters.h \
    battlecounterindex.h \
    battlebase.h \
    battle.h \
    abilities.h \
    battleserver.h \
    moves.h \
    items.h \
    plugininterface.h \
    pluginmanager.h \
    serverconnection.h \
    analyzer.h

unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

CONFIG(debian_package) {
    DEFINES += PO_DATA_REPO=\\\"/usr/share/games/pokemon-online/\\\"
    DEFINES += PO_HOME_DIR=\\\"~/.po-server/\\\"
}
!CONFIG(debian_package) {
    DEFINES += PO_DATA_REPO=\\\"./\\\"
    DEFINES += PO_HOME_DIR=\\\"./\\\"
}

LIBS += $$pokemoninfo

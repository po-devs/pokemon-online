# -------------------------------------------------
# Project created by QtCreator 2010-07-22T23:36:49
# -------------------------------------------------
QT += sql
TARGET = sqlusagestats
DESTDIR = ../../bin/serverplugins
TEMPLATE = lib
DEFINES += SQLUSAGEPLUGIN_LIBRARY
DEFINES += SERVER_SIDE
LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities
SOURCES += sqlusagestats.cpp \
    sqlstatsbattleplugin.cpp
HEADERS += sqlusagestats.h \
    sqlusagestats_global.h \
    ../PokemonInfo/battlestructs.h \
    ../Server/plugininterface.h \
    ../Server/playerinterface.h \
    ../Server/serverinterface.h \
    ../Server/battleinterface.h \
    sqlstatsbattleplugin.h

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

include(../Shared/Common.pri)

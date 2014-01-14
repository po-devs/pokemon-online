#-------------------------------------------------
#
# Project created by QtCreator 2012-11-28T21:46:22
#
#-------------------------------------------------

QT       += core network xml gui

TARGET = RelayStation
CONFIG   += console

TEMPLATE = app

SOURCES += main.cpp \
    relaystation.cpp \
    dualwielder.cpp \
    pokemontojson.cpp \
    battletojson.cpp \
    registrystation.cpp

HEADERS += \
    relaystation.h \
    dualwielder.h \
    pokemontojson.h \
    battletojson.h \
    battletojsonflow.h \
    registrystation.h

include(../Shared/Common.pri)

LIBS += $$battlemanager $$websocket $$json

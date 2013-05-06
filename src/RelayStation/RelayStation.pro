#-------------------------------------------------
#
# Project created by QtCreator 2012-11-28T21:46:22
#
#-------------------------------------------------

QT       += core network xml gui

TARGET = RelayStation
DESTDIR = $$PWD/../../bin
CONFIG   += console

TEMPLATE = app

LIBS += -L$$PWD/../../bin \
    -lpo-utilities \
    -lpo-pokemoninfo \
    -lpo-battlemanager \
    -lqtwebsocket \
    -lqjson

SOURCES += main.cpp \
    relaystation.cpp \
    ../Teambuilder/network.cpp \
    dualwielder.cpp \
    pokemontojson.cpp \
    battletojson.cpp

HEADERS += \
    relaystation.h \
    ../Teambuilder/network.h \
    dualwielder.h \
    pokemontojson.h \
    battletojson.h \
    battletojsonflow.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

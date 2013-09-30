# -------------------------------------------------
# Project created by QtCreator 2010-03-01T12:52:17
# -------------------------------------------------
TARGET = MoveMachine
DESTDIR = $$PWD/../../bin
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += mainwindow.h \
    ../Utilities/otherwidgets.h \
    ../PokemonInfo/movesetchecker.h
FORMS += mainwindow.ui

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

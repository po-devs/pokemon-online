# -------------------------------------------------
# Project created by QtCreator 2010-03-01T12:52:17
# -------------------------------------------------
TARGET = MoveMachine
DESTDIR = ../../bin
LIBS += -L../../bin -lpo-utilities -lpo-pokemoninfo
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += mainwindow.h \
    ../Utilities/otherwidgets.h \
    ../PokemonInfo/movesetchecker.h
FORMS += mainwindow.ui

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

# -------------------------------------------------
# Project created by QtCreator 2010-03-01T12:52:17
# -------------------------------------------------
TARGET = MoveMachine
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += mainwindow.h
FORMS += mainwindow.ui

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

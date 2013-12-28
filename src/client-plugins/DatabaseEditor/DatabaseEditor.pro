#-------------------------------------------------
#
# Project created by QtCreator 2013-12-28T19:31:11
#
#-------------------------------------------------

QT       += widgets

TARGET = DatabaseEditor
TEMPLATE = lib
DESTDIR = $$PWD/../../../bin/myplugins

DEFINES += DATABASEEDITOR_LIBRARY

SOURCES += databaseeditor.cpp \
    pokemoneditordialog.cpp

HEADERS += databaseeditor.h\
        databaseeditor_global.h \
    ../../Teambuilder/plugininterface.h \
    ../../Teambuilder/engineinterface.h \
    ../../PokemonInfo/teamholderinterface.h \
    pokemoneditordialog.h

include(../../Shared/Common.pri)

LIBS += $$pokemoninfo

FORMS += \
    pokemoneditordialog.ui

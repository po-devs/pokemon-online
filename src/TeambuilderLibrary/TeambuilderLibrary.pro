#-------------------------------------------------
#
# Project created by QtCreator 2013-12-28T21:20:10
#
#-------------------------------------------------

QT       += widgets

TARGET = po-teambuilder
TEMPLATE = lib

SOURCES += \
    theme.cpp \
    poketextedit.cpp \
    pokeselection.cpp \
    advancedsearch.cpp \
    basestatswidget.cpp \
    pokechoice.cpp

HEADERS += \
    theme.h \
    themeaccessor.h \
    poketextedit.h \
    pokeselection.h \
    modelenum.h \
    advancedsearch.h \
    basestatswidget.h \
    pokechoice.h

include(../Shared/Common.pri)

FORMS += \
    pokeselection.ui \
    advancedsearch.ui \
    basestatswidget.ui

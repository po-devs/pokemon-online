#-------------------------------------------------
#
# Project created by QtCreator 2013-11-28T21:19:43
#
#-------------------------------------------------

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

EXTRAS = test

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

include(../../src/Shared/Common.pri)

LIBS += $$pokemoninfo

TARGET = test-pokemoninfo

SOURCES += main.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp \
    testimportexportteam.cpp \
    ../common/pokemontestrunner.cpp \
    testiteminfo.cpp

HEADERS += \
    ../common/test.h \
    ../common/testrunner.h \
    testimportexportteam.h \
    ../common/pokemontestrunner.h \
    testiteminfo.h

#-------------------------------------------------
#
# Project created by QtCreator 2010-09-18T22:40:53
#
#-------------------------------------------------

QT       += core xml

DESTDIR = ../../bin

TARGET = veekun_data_extracter
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities
SOURCES += main.cpp

QMAKE_CXXFLAGS += "-std=c++0x"

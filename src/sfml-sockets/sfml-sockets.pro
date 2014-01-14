#-------------------------------------------------
#
# Project created by QtCreator 2011-04-11T00:24:11
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = sfml-sockets
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    sfmlsocket.cpp \
    server.cpp

HEADERS += \
    sfmlsocket.h \
    server.h

LIBS += -lsfml-network \
    -lsfml-system

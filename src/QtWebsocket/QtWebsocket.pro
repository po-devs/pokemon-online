#-------------------------------------------------
#
# Project created by QtCreator 2012-03-05T10:38:43
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = QtWebsocket
TEMPLATE = lib
CONFIG += staticlib

#DEFINES += QTWEBSOCKET_LIBRARY

SOURCES += QWsServer.cpp \
    QWsSocket.cpp

HEADERS += QWsServer.h \
    QWsSocket.h


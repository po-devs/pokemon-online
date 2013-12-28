#-------------------------------------------------
#
# Project created by QtCreator 2012-03-05T10:38:43
#
#-------------------------------------------------
#git://gitorious.org/qtwebsocket/qtwebsocket.git

QT       += network

QT       -= gui

TARGET = qtwebsocket
TEMPLATE = lib

#DEFINES += QTWEBSOCKET_LIBRARY

SOURCES += QWsServer.cpp \
    QWsSocket.cpp

HEADERS += QWsServer.h \
    QWsSocket.h

include(../Shared/Common.pri)

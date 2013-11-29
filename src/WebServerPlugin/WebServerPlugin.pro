#-------------------------------------------------
#
# Project created by QtCreator 2012-11-25T18:09:35
#
#-------------------------------------------------

QT       += network

TARGET = WebServerPlugin
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/serverplugins

DEFINES += WEBSERVERPLUGIN_LIBRARY

SOURCES += webserverplugin.cpp \
    webserverconfig.cpp

HEADERS += webserverplugin.h\
        WebServerPlugin_global.h \
    webserverconfig.h

include(../Shared/Common.pri)

LIBS += $$utilities $$websocket $$json

FORMS += webserverconfig.ui

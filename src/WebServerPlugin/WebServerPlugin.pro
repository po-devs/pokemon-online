#-------------------------------------------------
#
# Project created by QtCreator 2012-11-25T18:09:35
#
#-------------------------------------------------

QT       += network

TARGET = WebServerPlugin
TEMPLATE = lib
DESTDIR = ../../bin/serverplugins

DEFINES += WEBSERVERPLUGIN_LIBRARY

LIBS += -L../../bin \
    -lqtwebsocket \
    -lpo-utilities \
    -lqjson


SOURCES += webserverplugin.cpp \
    webserverconfig.cpp

HEADERS += webserverplugin.h\
        WebServerPlugin_global.h \
    webserverconfig.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-std=c++11"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x"
}

include(../Shared/Common.pri)

FORMS += \
    webserverconfig.ui

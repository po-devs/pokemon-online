#-------------------------------------------------
#
# Project created by QtCreator 2012-06-25T15:39:55
#
#-------------------------------------------------

QT += script network declarative

TARGET = ClientScripting
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins

DEFINES += CLIENTSCRIPTING_LIBRARY

SOURCES += clientscripting.cpp \
    scriptwindow.cpp \
    scriptengine.cpp \
    scriptutils.cpp \
    battlescripting.cpp

HEADERS += clientscripting.h\
        ClientScripting_global.h \
    scriptwindow.h \
    scriptengine.h \
    scriptutils.h \
    battlescripting.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5 
  QT += widgets multimedia
  QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../Shared/Common.pri)

LIBS += $$battlemanager

FORMS += \
    scriptwindow.ui

#-------------------------------------------------
#
# Project created by QtCreator 2012-06-25T15:39:55
#
#-------------------------------------------------

QT += script network declarative

TARGET = ClientScripting
TEMPLATE = lib
EXTRAS=clientplugin

DEFINES += CLIENTSCRIPTING_LIBRARY

SOURCES += clientscripting.cpp \
    scriptwindow.cpp \
    scriptengine.cpp \
    scriptutils.cpp \
    battlescripting.cpp \
    scriptengineagent.cpp

HEADERS += clientscripting.h\
        ClientScripting_global.h \
    scriptwindow.h \
    scriptengine.h \
    scriptutils.h \
    battlescripting.h \
    scriptengineagent.h

include(../../Shared/Common.pri)

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  QT += multimedia
}

LIBS += $$battlemanager

FORMS += \
    scriptwindow.ui

#-------------------------------------------------
#
# Project created by QtCreator 2012-06-27T18:54:35
#
#-------------------------------------------------

TARGET = SettingsPlugin
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins

DEFINES += SETTINGSPLUGIN_LIBRARY

SOURCES += settingsplugin.cpp \
    settingsdialog.cpp

HEADERS += settingsplugin.h\
        SettingsPlugin_global.h \
    settingsdialog.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets multimedia
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

FORMS += settingsdialog.ui

include(../Shared/Common.pri)

LIBS += $$utilities

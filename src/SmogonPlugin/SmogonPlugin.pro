#-------------------------------------------------
#
# Project created by QtCreator 2011-08-06T15:08:40
#
#-------------------------------------------------

TARGET = SmogonPlugin
TEMPLATE = lib 
DESTDIR = $$PWD/../../bin/myplugins

QT += xml network 

DEFINES += SMOGONPLUGIN_LIBRARY

SOURCES += smogonplugin.cpp \
           smogonscraper.cpp \
            pokemontab.cpp \
            pokemonteamtabs.cpp \
    smogonbuild.cpp \
    teambuildersmogonplugin.cpp \
    smogonsinglepokedialog.cpp


HEADERS += smogonplugin.h\
        SmogonPlugin_global.h \
    ../Teambuilder/plugininterface.h \
    ../Teambuilder/engineinterface.h \
    smogonscraper.h \
    pokemontab.h \
    pokemonteamtabs.h \
    smogonbuild.h \
    teambuildersmogonplugin.h \
    smogonsinglepokedialog.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}


FORMS += smogonsinglepokedialog.ui

include(../Shared/Common.pri)

LIBS += $$battlemanager

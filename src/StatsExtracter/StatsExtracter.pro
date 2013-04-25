#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T18:38:46
#
#-------------------------------------------------

QT += gui
TARGET = StatsExtracter
DESTDIR = ../../bin

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
LIBS += -L../../bin -lpo-pokemoninfo -lpo-utilities

SOURCES += main.cpp

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

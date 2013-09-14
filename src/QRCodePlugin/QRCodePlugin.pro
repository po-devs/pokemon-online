#-------------------------------------------------
#
# Project created by QtCreator 2011-08-06T15:08:40
#
#-------------------------------------------------

TARGET = QRCodePlugin
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins

QT += xml

DEFINES += QRCODEPLUGIN_LIBRARY

SOURCES += qrcodeplugin.cpp

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x"
}

HEADERS += qrcodeplugin.h\
        QRCodePlugin_global.h \
    ../Teambuilder/plugininterface.h \
    ../Teambuilder/engineinterface.h

windows: {
    #on windows, qrencode is probably in that folder
    LIBS += -L$$PWD/../../bin/myplugins
}

include(../Shared/Common.pri)

LIBS += $$pokemoninfo

windows: {
    LIBS += -lzlib1 -l../lib/windows/qrcodelib
}

!windows: {
    LIBS += -lz -lqrencode
}

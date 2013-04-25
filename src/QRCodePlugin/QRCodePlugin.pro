#-------------------------------------------------
#
# Project created by QtCreator 2011-08-06T15:08:40
#
#-------------------------------------------------

TARGET = QRCodePlugin
TEMPLATE = lib
DESTDIR = ../../bin/myplugins

QT += xml

DEFINES += QRCODEPLUGIN_LIBRARY

SOURCES += qrcodeplugin.cpp

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-std=c++11"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x"
}

HEADERS += qrcodeplugin.h\
        QRCodePlugin_global.h \
    ../Teambuilder/plugininterface.h \
    ../Teambuilder/engineinterface.h

windows: {
    #on windows, qrencode is probably in that folder
    LIBS += -L../../bin/myplugins
}

LIBS += -L../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities

windows: {
    LIBS += -lzlib1 -l../lib/windows/qrcodelib
}

!windows: {
    LIBS += -lz -lqrencode
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}

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

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

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
    LIBS += -lzlib1 -lqrcodelib
}

!windows: {
    LIBS += -lz -lqrencode
}

symbian {
    #Symbian specific definitions
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA9E7289
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = QRCodePlugin.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}

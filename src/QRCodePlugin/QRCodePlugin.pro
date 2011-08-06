#-------------------------------------------------
#
# Project created by QtCreator 2011-08-06T15:08:40
#
#-------------------------------------------------

TARGET = QRCodePlugin
TEMPLATE = lib
DESTDIR = ../../bin/myplugins

DEFINES += QRCODEPLUGIN_LIBRARY

SOURCES += qrcodeplugin.cpp

HEADERS += qrcodeplugin.h\
        QRCodePlugin_global.h

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

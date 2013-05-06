#-------------------------------------------------
#
# Project created by QtCreator 2012-07-28T19:19:39
#
#-------------------------------------------------

TARGET = ChessPlugin
TEMPLATE = lib
QT += svg
DESTDIR =  $$PWD/../../bin

DEFINES += CHESSPLUGIN_LIBRARY

SOURCES += chessplugin.cpp

HEADERS += chessplugin.h\
        ChessPlugin_global.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xED17FCB3
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ChessPlugin.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

# Chess plugin depends on cutechess
# https://github.com/coyotte508/cutechess
include(../../../cutechess/projects/gui/src/src.pri)
include(../../../cutechess/projects/gui/ui/ui.pri)
include(../../../cutechess/projects/gui/res/res.pri)
include(../../../cutechess/projects/lib/lib.pri)
DEFINES += CUTECHESS_VERSION=1

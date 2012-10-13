#-------------------------------------------------
#
# Project created by QtCreator 2011-08-06T15:08:40
#
#-------------------------------------------------

TARGET = SmogonPlugin
TEMPLATE = lib 
DESTDIR = ../../bin/myplugins

QT += xml 

DEFINES += SMOGONPLUGIN_LIBRARY

SOURCES += smogonplugin.cpp \
    smogonscraper.cpp

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

HEADERS += smogonplugin.h\
        SmogonPlugin_global.h \
    ../Teambuilder/plugininterface.h \
    ../Teambuilder/engineinterface.h \
    smogonscraper.h

windows: {
    LIBS += -L../../bin/myplugins
}

LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities

symbian {
    #Symbian specific definitions
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA9E7289
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1 
    addFiles.sources = SmogonPlugin.dll
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


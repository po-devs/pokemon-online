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

windows: {
    LIBS += -L$$PWD/../../bin/myplugins
}

LIBS += -L$$PWD/../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities \
    -lpo-battlemanager

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}


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

FORMS += \
    smogonsinglepokedialog.ui


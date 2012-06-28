#-------------------------------------------------
#
# Project created by QtCreator 2012-06-27T18:54:35
#
#-------------------------------------------------

TARGET = SettingsPlugin
TEMPLATE = lib
DESTDIR = ../../bin/myplugins
LIBS += -L../../bin \
    -lutilities

DEFINES += SETTINGSPLUGIN_LIBRARY

SOURCES += settingsplugin.cpp \
    settingsdialog.cpp

HEADERS += settingsplugin.h\
        SettingsPlugin_global.h \
    settingsdialog.h

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xED0FD9B2
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = SettingsPlugin.dll
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

FORMS += \
    settingsdialog.ui

#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T14:50:56
#
#-------------------------------------------------

TARGET = CSSChanger
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins

DEFINES += CSSCHANGER_LIBRARY

SOURCES += csschanger.cpp \
    csswidget.cpp \
    colorchoicewidget.cpp \
    massreplacewidget.cpp

HEADERS += csschanger.h\
        CSSChanger_global.h \
    csswidget.h \
    ui_dialog.h \
    ui_colorchoice.h \
    colorchoicewidget.h \
    massreplacewidget.h \
    ui_massreplace.h \
    data.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
}


symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE9A936F8
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = CSSChanger.dll
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
    dialog.ui \
    colorchoice.ui \
    massreplace.ui

include(../Shared/Common.pri)

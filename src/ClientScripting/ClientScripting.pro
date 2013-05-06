#-------------------------------------------------
#
# Project created by QtCreator 2012-06-25T15:39:55
#
#-------------------------------------------------

QT += script network declarative

TARGET = ClientScripting
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins
LIBS += -L$$PWD/../../bin \
    -lpo-utilities \
    -lpo-pokemoninfo \
    -lpo-battlemanager

DEFINES += CLIENTSCRIPTING_LIBRARY

SOURCES += clientscripting.cpp \
    scriptwindow.cpp \
    scriptengine.cpp \
    scriptutils.cpp \
    battlescripting.cpp

HEADERS += clientscripting.h\
        ClientScripting_global.h \
    scriptwindow.h \
    scriptengine.h \
    scriptutils.h \
    battlescripting.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5 
  QT += widgets multimedia
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}


symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEF055325
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ClientScripting.dll
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

include(../Shared/Common.pri)

FORMS += \
    scriptwindow.ui

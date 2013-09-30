#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T14:50:56
#
#-------------------------------------------------

TARGET = ThemeManager
TEMPLATE = lib
DESTDIR = $$PWD/../../bin/myplugins

QT += network xml

DEFINES += THEMEMANAGER_LIBRARY

SOURCES += thememanager.cpp \
    thememanagerwidget.cpp \
    themewidget.cpp \
    clickablelabel.cpp \
    imageviewerlabel.cpp

HEADERS += thememanager.h\
        ThemeManager_global.h \
    thememanagerwidget.h \
    themewidget.h \
    clickablelabel.h \
    imageviewerlabel.h

FORMS += \
    thememanagerwidget.ui \
    themewidget.ui

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  macx: {
    QMAKE_CXXFLAGS += "-U__STRICT_ANSI__"
    CONFIG += c++11
  }
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}


macx:QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/

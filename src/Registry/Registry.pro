# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network
QT -= gui
TARGET = Registry
DESTDIR = $$PWD/../../bin
TEMPLATE = app

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

SOURCES += main.cpp \
    mainwindow.cpp \
    registry.cpp \
    server.cpp \
    player.cpp \
    analyze.cpp \
    network.cpp \
    antidos.cpp
HEADERS += mainwindow.h \
    registry.h \
    server.h \
    player.h \
    analyze.h \
    network.h \
    antidos.h \
    macro.h
DEFINES = REGISTRY_SIDE

LIBS += -L$$PWD/../../bin \
    -lpo-utilities

# Build-in web server depends on pillow, you can download
# and install it from github:
# https://github.com/acossette/pillow
CONFIG(webconf) {
    HEADERS += webinterface.h
    SOURCES += webinterface.cpp
    INCLUDEPATH += /home/lamperi/pillow/pillowcore
    LIBS += /home/lamperi/pillow/lib/libpillowcore.a
    DEFINES += USE_WEBCONF
}

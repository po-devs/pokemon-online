# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network
QT -= gui
TARGET = Registry
DESTDIR = ../../bin
TEMPLATE = app
QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
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

LIBS += -L../../bin \
    -lutilities

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

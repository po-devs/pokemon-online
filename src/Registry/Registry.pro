# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network

TARGET = Registry
TEMPLATE = app

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

# Build-in web server depends on pillow, you can download
# and install it from github:
# https://github.com/acossette/pillow
CONFIG(webconf) {
    HEADERS += webinterface.h
    SOURCES += webinterface.cpp
    INCLUDEPATH += ../../lib/pillow/pillowcore
    LIBS += -L../../lib/pillow/lib/ -lpillowcore
    DEFINES += USE_WEBCONF
}

include(../Shared/Common.pri)

LIBS += $$utilities

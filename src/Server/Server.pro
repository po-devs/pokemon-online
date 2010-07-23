# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network \
    script \
    sql
TARGET = Server
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp
HEADERS += mainwindow.h \
    server.h \
    serverwidget.h \
    consolereader.h \
    plugininterface.h \
    ../Shared/defines.h
DEFINES = SERVER_SIDE
LIBS += -L../../bin \
    -lmaindll
RESOURCES += 

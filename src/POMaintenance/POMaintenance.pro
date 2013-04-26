#-------------------------------------------------
#
# Project created by QtCreator 2012-07-09T07:40:43
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = pomaintenance
TEMPLATE = app

# We do not want this app as bundle on Mac
# Note: currently POMaintenance is unused on Mac
# but in case of future migration
macx {
    CONFIG -= app_bundle
}

DESTDIR = ../../bin/

SOURCES += main.cpp\
        mainwindow.cpp \
    core.cpp

HEADERS  += mainwindow.h \
    core.h

FORMS    += mainwindow.ui

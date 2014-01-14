#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T14:50:56
#
#-------------------------------------------------

TARGET = CSSChanger
TEMPLATE = lib
EXTRAS=clientplugin

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

FORMS += \
    dialog.ui \
    colorchoice.ui \
    massreplace.ui

include(../../Shared/Common.pri)

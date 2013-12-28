TARGET = DesignerPlugin
TEMPLATE = lib
DESTDIR = $$PWD/../../../bin/myplugins

DEFINES += DESIGNERPLUGIN_LIBRARY

SOURCES += \
    designerplugin.cpp \
    designerwidget.cpp

HEADERS += \
    designerplugin.h \
    DesignerPlugin_global.h \
    designerwidget.h

include(../../Shared/Common.pri)

LIBS += $$teambuilder

FORMS += \
    designerwidget.ui

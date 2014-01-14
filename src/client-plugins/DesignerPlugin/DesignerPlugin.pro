TARGET = DesignerPlugin
TEMPLATE = lib
EXTRAS=clientplugin

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

#-------------------------------------------------
#
# Project created by QtCreator 2012-11-30T00:18:53
#
#-------------------------------------------------
# Download here: git://github.com/flavio/qjson.git

QT       -= gui

DESTDIR = $$PWD/../../bin
TARGET = qjson
TEMPLATE = lib

DEFINES += QJSON_MAKEDLL

SOURCES += \
    serializerrunnable.cpp \
    serializer.cpp \
    qobjecthelper.cpp \
    parserrunnable.cpp \
    parser.cpp \
    json_scanner.cpp \
    json_parser.cc

OTHER_FILES += \
    json_parser.yy

HEADERS += \
    stack.hh \
    serializerrunnable.h \
    serializer.h \
    qobjecthelper.h \
    qjson_export.h \
    qjson_debug.h \
    qjson.h \
    position.hh \
    parserrunnable.h \
    parser_p.h \
    parser.h \
    location.hh \
    json_scanner.h \
    json_parser.hh

include(../Shared/Common.pri)

TARGET = utilities
TEMPLATE = lib
DESTDIR = ../../bin
DEFINES += SERVER_SIDE
DEFINES -= UNICODE
SOURCES += otherwidgets.cpp \
    mtrand.cpp \
    functions.cpp \
    CrossDynamicLib.cpp \
    contextswitch.cpp \
    coro.c
HEADERS += otherwidgets.h \
    mtrand.h \
    functions.h \
    CrossDynamicLib.h \
    coro.h \
    contextswitch.h
OTHER_FILES += 

include(../Shared/Common.pri)

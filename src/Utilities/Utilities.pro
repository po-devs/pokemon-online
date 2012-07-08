TARGET = utilities
TEMPLATE = lib
DESTDIR = ../../bin
DEFINES -= UNICODE
SOURCES += otherwidgets.cpp \
    mtrand.cpp \
    functions.cpp \
    CrossDynamicLib.cpp \
    contextswitch.cpp \
    coro.c \
    coreclasses.cpp \
    qimagebuttonlr.cpp \
    confighelper.cpp \
    qtableplus.cpp \
    qclicklabel.cpp \
    ziputils.cpp
HEADERS += otherwidgets.h \
    mtrand.h \
    functions.h \
    CrossDynamicLib.h \
    coro.h \
    contextswitch.h \
    coreclasses.h \
    qimagebuttonlr.h \
    confighelper.h \
    qtableplus.h \
    qclicklabel.h \
    ziputils.h

OTHER_FILES += 
QMAKE_CXXFLAGS += "-std=c++0x"

windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

include(../Shared/Common.pri)

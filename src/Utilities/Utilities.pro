TARGET = po-utilities

TEMPLATE = lib
DESTDIR = $$PWD/../../bin
DEFINES -= UNICODE
SOURCES += otherwidgets.cpp \
    mtrand.cpp \
    functions.cpp \
    CrossDynamicLib.cpp \
    contextswitch.cpp \
    coreclasses.cpp \
    qimagebuttonlr.cpp \
    confighelper.cpp \
    qtableplus.cpp \
    qclicklabel.cpp \
    ziputils.cpp \
    qclosedockwidget.cpp \
    backtrace.cpp \
    qverticalscrollarea.cpp \
    qscrolldowntextbrowser.cpp
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
    ziputils.h \
    qclosedockwidget.h \
    backtrace.h \
    qverticalscrollarea.h \
    qscrolldowntextbrowser.h

windows: {
HEADERS += coro/taskimpl.h \
    coro/power-ucontext.h \
    coro/Coro.h \
    coro/asm.S \
    coro/amd64-ucontext.h \
    coro/386-ucontext.h\
    coro/Common.h\
    coro/Base.h


SOURCES += coro/Coro.c \
    coro/context.c \
    coro/Common.c
DEFINES += CORO2
}

!windows: {
SOURCES += coro.c
}

OTHER_FILES += 

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  SOURCES += wavreader.cpp
  HEADERS += wavreader.h
  DEFINES += QT5
  QT += widgets multimedia
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x"
}

windows: { LIBS += -L$$PWD/../../bin -lzip-2 }
!windows: { LIBS += -lzip }

include(../Shared/Common.pri)

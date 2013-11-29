TARGET = po-utilities

QT += network

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
    qscrolldowntextbrowser.cpp \
    pluginmanager.cpp \
    pluginmanagerwidget.cpp \
    antidos.cpp \
    antidoswindow.cpp \
    network.cpp \
    baseanalyzer.cpp
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
    qscrolldowntextbrowser.h \
    pluginmanager.h \
    plugininterface.h \
    pluginmanagerwidget.h \
    antidos.h \
    antidoswindow.h \
    asiosocket.h \
    network.h \
    rankingtree.h \
    baseanalyzer.h

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
CONFIG(nocoro) {
    DEFINES += CORO_PTHREAD
}
}

CONFIG(boost_asio) {
    DEFINES += BOOST_SOCKETS
    SOURCES += asiosocket.cpp
    LIBS += -L/usr/local/lib \
        -lboost_system
}

OTHER_FILES += 

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  SOURCES += wavreader.cpp
  HEADERS += wavreader.h
  QT += multimedia
}

windows: { LIBS += -L$$PWD/../../bin -lzip-2 }
!windows: { LIBS += -lzip }

include(../Shared/Common.pri)

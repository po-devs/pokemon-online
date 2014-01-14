TARGET = po-utilities

QT += network gui

TEMPLATE = lib

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
    antidos.cpp \
    antidoswindow.cpp \
    baseanalyzer.cpp \
    keypresseater.cpp \
    pluginmanagerdialog.cpp
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
    antidos.h \
    antidoswindow.h \
    asiosocket.h \
    network.h \
    rankingtree.h \
    baseanalyzer.h \
    keypresseater.h \
    exesuffix.h \
    pluginmanagerdialog.h

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

include(../../Shared/Common.pri)

windows: { LIBS += -L$$bin -lzip-2 }
!windows: { LIBS += -lzip }

FORMS += \
    pluginmanagerdialog.ui

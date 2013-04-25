TARGET = po-pokemoninfo
TEMPLATE = lib
DESTDIR = ../../bin
QT += xml
SOURCES += pokemonstructs.cpp \
    pokemoninfo.cpp \
    networkstructs.cpp \
    movesetchecker.cpp \
    battlestructs.cpp \
    teamsaver.cpp \
    pokemon.cpp
HEADERS += pokemonstructs.h \
    pokemoninfo.h \
    networkstructs.h \
    movesetchecker.h \
    battlestructs.h \
    teamsaver.h \
    enums.h \
    ../Shared/config.h \
    geninfo.h \
    pokemon.h



contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  macx: {
    QMAKE_CXXFLAGS += "-std=c++11 -U__STRICT_ANSI__"
  }
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

LIBS += -L../../bin \
    -lpo-utilities

windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

OTHER_FILES += 

include(../Shared/Common.pri)

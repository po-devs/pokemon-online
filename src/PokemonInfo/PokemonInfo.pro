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

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

LIBS += -L../../bin \
    -lpo-utilities

windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

OTHER_FILES += 

include(../Shared/Common.pri)

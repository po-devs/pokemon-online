TARGET = po-pokemoninfo
TEMPLATE = lib
DESTDIR = $$PWD/../../bin
QT += xml
SOURCES += pokemonstructs.cpp \
    pokemoninfo.cpp \
    networkstructs.cpp \
    movesetchecker.cpp \
    battlestructs.cpp \
    teamsaver.cpp \
    pokemon.cpp \
    teamholder.cpp
HEADERS += pokemonstructs.h \
    pokemoninfo.h \
    networkstructs.h \
    movesetchecker.h \
    battlestructs.h \
    teamsaver.h \
    enums.h \
    ../Shared/config.h \
    geninfo.h \
    pokemon.h \
    teamholder.h \
    teamholderinterface.h

include(../Shared/Common.pri)

LIBS += $$utilities


windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

OTHER_FILES += 

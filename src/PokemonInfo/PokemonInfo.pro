TARGET = pokemonlib
TEMPLATE = lib
DESTDIR = ../../bin
DEFINES += SERVER_SIDE
SOURCES += pokemonstructs.cpp \
    pokemoninfo.cpp \
    networkstructs.cpp \
    movesetchecker.cpp \
    battlestructs.cpp
HEADERS += pokemonstructs.h \
    pokemoninfo.h \
    networkstructs.h \
    movesetchecker.h \
    battlestructs.h
LIBS += -L../../bin \
    -lutilities

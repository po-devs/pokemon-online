TARGET = "ChainBreeding"
DESTDIR = ../../bin
HEADERS += ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/movesetchecker.h
SOURCES += main.cpp
LIBS += -L../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

include(../Shared/Common.pri)


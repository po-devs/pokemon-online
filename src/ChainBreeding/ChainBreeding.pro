TARGET = "Chain Breeding"
DESTDIR = ../../bin
HEADERS += ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/movesetchecker.h
SOURCES += main.cpp
LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities

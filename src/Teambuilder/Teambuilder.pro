# -------------------------------------------------
# Project created by QtCreator 2009-10-06T21:30:23
# -------------------------------------------------
TARGET = Teambuilder
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    teambuilder.cpp
HEADERS += teambuilder.h \
    ../PokemonInfo/pokemoninfo.h
FORMS += 
LIBS += -L../../bin \
    -lPokemonInfo \
    -lzip

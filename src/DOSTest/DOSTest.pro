# -------------------------------------------------
# Project created by QtCreator 2010-03-27T17:34:35
# -------------------------------------------------
QT += network \
    xml
TARGET = DOSTest
DESTDIR = ../../bin
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    ../PokemonInfo/pokemonstructs.cpp \
    ../PokemonInfo/pokemoninfo.cpp \
    ../PokemonInfo/networkstructs.cpp \
    ../PokemonInfo/movesetchecker.cpp \
    ../PokemonInfo/battlestructs.cpp \
    network.cpp \
    analyze.cpp \
    arg.cpp
HEADERS += ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/networkstructs.h \
    ../PokemonInfo/movesetchecker.h \
    ../PokemonInfo/battlestructs.h \
    arg.h \
    network.h \
    analyze.h
DEFINES = CLIENT_SIDE
LIBS += -L../../bin \
    -lpo-pokemoninfo \
    -lpo-utilities \
    -lzip

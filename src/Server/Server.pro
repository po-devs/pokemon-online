# -------------------------------------------------
# Project created by QtCreator 2009-11-05T22:07:35
# -------------------------------------------------
QT += network
TARGET = Server
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    network.cpp \
    analyze.cpp \
    server.cpp \
    ../PokemonInfo/pokemoninfo.cpp \
    ../PokemonInfo/pokemonstructs.cpp \
    ../PokemonInfo/battlestructs.cpp \
    ../PokemonInfo/networkstructs.cpp \
    player.cpp \
    battle.cpp
HEADERS += mainwindow.h \
    network.h \
    analyze.h \
    server.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    player.h \
    battle.h
LIBS += -L../../bin \
    -lzip
CONFIG():QMAKE_CXXFLAGS += -std=c++0x

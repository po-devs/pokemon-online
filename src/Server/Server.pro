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
    ../PokemonInfo/pokemonstructs.cpp
HEADERS += mainwindow.h \
    network.h \
    analyze.h \
    server.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/pokemonstructs.h
LIBS += -L../../bin \
	-lzip

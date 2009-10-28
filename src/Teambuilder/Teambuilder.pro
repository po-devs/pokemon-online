# -------------------------------------------------
# Project created by QtCreator 2009-10-06T21:30:23
# -------------------------------------------------
TARGET = Teambuilder
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    teambuilder.cpp \
    advanced.cpp \
    menu.cpp \
    otherwidgets.cpp \
    mainwindow.cpp \
    ../PokemonInfo/pokemoninfo.cpp
HEADERS += teambuilder.h \
    ../PokemonInfo/pokemoninfo.h \
    advanced.h \
    menu.h \
    otherwidgets.h \
    mainwindow.h
FORMS += 
LIBS += -L../../bin \
    -lPokemonInfo \
    -lzip

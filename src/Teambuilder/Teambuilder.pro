QT += network \
    xml
TARGET = Teambuilder
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    teambuilder.cpp \
    advanced.cpp \
    menu.cpp \
    mainwindow.cpp \
    ../PokemonInfo/pokemoninfo.cpp \
    ../PokemonInfo/pokemonstructs.cpp \
    ../Utilities/otherwidgets.cpp \
    network.cpp \
    ../Utilities/dockinterface.cpp \
    client.cpp \
    analyze.cpp \
    serverchoice.cpp \
    ../PokemonInfo/battlestructs.cpp \
    ../PokemonInfo/networkstructs.cpp \
    challenge.cpp \
    battlewindow.cpp \
    ../Utilities/functions.cpp \
    ../Utilities/pokeListe.cpp \
    ../Utilities/pokeButton.cpp \
    ../Utilities/md5.c \
    pmwindow.cpp \
    controlpanel.cpp \
    basebattlewindow.cpp
HEADERS += teambuilder.h \
    ../PokemonInfo/pokemoninfo.h \
    advanced.h \
    menu.h \
    mainwindow.h \
    ../PokemonInfo/pokemonstructs.h \
    ../Utilities/otherwidgets.h \
    network.h \
    ../Utilities/dockinterface.h \
    client.h \
    analyze.h \
    serverchoice.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    challenge.h \
    battlewindow.h \
    ../Utilities/functions.h \
    ../Utilities/pokeListe.h \
    ../Utilities/pokeButton.h \
    ../Utilities/md5.h \
    pmwindow.h \
    controlpanel.h \
    basebattlewindow.h
LIBS += -L../../bin \
    -lzip
DEFINES = CLIENT_SIDE
FORMS += controlpanel.ui
TRANSLATIONS = translation_fr.ts \
    translation_pt-br.ts

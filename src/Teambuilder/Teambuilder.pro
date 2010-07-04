QT += network \
    xml
TARGET = Pokemon-Online
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
    ../Utilities/md5.c \
    pmwindow.cpp \
    controlpanel.cpp \
    basebattlewindow.cpp \
    ../PokemonInfo/movesetchecker.cpp \
    box.cpp \
    ranking.cpp \
    pokedex.cpp
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
    ../Utilities/md5.h \
    pmwindow.h \
    controlpanel.h \
    basebattlewindow.h \
    ../PokemonInfo/movesetchecker.h \
    ../Shared/config.h \
    box.h \
    ranking.h \
    pokedex.h
LIBS += -L../../bin \
    -lzip
DEFINES = CLIENT_SIDE
FORMS += controlpanel.ui
TRANSLATIONS =  \
    translation_de.ts \
    translation_es.ts \
    translation_fi.ts \
    translation_fr.ts \
    translation_he.ts \
    translation_it.ts \
    translation_jp.ts \
    translation_ko.ts \
    translation_pt-br.ts \
    translation_ru.ts \
    translation_sv.ts \
    translation_zh-cn.ts
RC_FILE = myapp.rc
RESOURCES += 

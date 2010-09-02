QT += network \
    xml \
    phonon
TARGET = Pokemon-Online
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    teambuilder.cpp \
    advanced.cpp \
    menu.cpp \
    mainwindow.cpp \
    network.cpp \
    dockinterface.cpp \
    client.cpp \
    analyze.cpp \
    serverchoice.cpp \
    challenge.cpp \
    battlewindow.cpp \
    pmwindow.cpp \
    controlpanel.cpp \
    basebattlewindow.cpp \
    box.cpp \
    ranking.cpp \
    pokedex.cpp \
    pluginmanager.cpp \
    channel.cpp \
    tierstruct.cpp
HEADERS += teambuilder.h \
    ../PokemonInfo/pokemoninfo.h \
    advanced.h \
    menu.h \
    mainwindow.h \
    ../PokemonInfo/pokemonstructs.h \
    ../Utilities/otherwidgets.h \
    network.h \
    dockinterface.h \
    client.h \
    analyze.h \
    serverchoice.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    challenge.h \
    battlewindow.h \
    ../Utilities/functions.h \
    pmwindow.h \
    controlpanel.h \
    basebattlewindow.h \
    ../PokemonInfo/movesetchecker.h \
    ../Shared/config.h \
    box.h \
    ranking.h \
    pokedex.h \
    pluginmanager.h \
    plugininterface.h \
    centralwidget.h \
    channel.h \
    tierstruct.h
LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities
FORMS += controlpanel.ui
TRANSLATIONS = translation_de.ts \
    translation_es.ts \
    translation_fi.ts \
    translation_fr.ts \
    translation_he.ts \
    translation_it.ts \
    translation_jp.ts \
    translation_ko.ts \
    translation_pl.ts \
    translation_pt-br.ts \
    translation_ru.ts \
    translation_sv.ts \
    translation_zh-cn.ts
RC_FILE = myapp.rc
RESOURCES += 
macx:LIBS += -framework \
    CoreFoundation

QT += network \
    xml \
    phonon \
    declarative \
    opengl
TARGET = Pokemon-Online
DESTDIR = ../../bin
TEMPLATE = app
SOURCES += main.cpp \
    menu.cpp \
    mainwindow.cpp \
    network.cpp \
    client.cpp \
    analyze.cpp \
    serverchoice.cpp \
    battlewindow.cpp \
    controlpanel.cpp \
    basebattlewindow.cpp \
    ranking.cpp \
    pluginmanager.cpp \
    channel.cpp \
    tierstruct.cpp \
    theme.cpp \
    rearrangewindow.cpp \
    poketablemodel.cpp \
    pokemovesmodel.cpp \
    Teambuilder/pokedex.cpp \
    Teambuilder/box.cpp \
    Teambuilder/trainerbody.cpp \
    Teambuilder/avatarbox.cpp \
    Teambuilder/pokeballed.cpp \
    Teambuilder/evmanager.cpp \
    Teambuilder/pokebody.cpp \
    Teambuilder/advanced.cpp \
    Teambuilder/pokechoice.cpp \
    Teambuilder/teambody.cpp \
    Teambuilder/dockinterface.cpp \
    Teambuilder/pokebodywidget.cpp \
    logmanager.cpp \
    poketextedit.cpp \
    trainermenu.cpp \
    pokebuttonsholder.cpp \
    pokebutton.cpp \
    teamholder.cpp \
    teambuilder.cpp \
    Teambuilder/teambuilder_old.cpp \
    teammenu.cpp \
    pokeedit.cpp \
    evbox.cpp \
    pokelevelsettings.cpp \
    spectatorwindow.cpp \
    replayviewer.cpp \
    soundconfigwindow.cpp \
    password_wallet.cpp \
    pmsystem.cpp \
    ivbox.cpp \
    teamimporter.cpp \
    challengedialog.cpp \
    tierratingbutton.cpp \
    findbattledialog.cpp \
    teamline.cpp
HEADERS +=  ../PokemonInfo/pokemoninfo.h \
    menu.h \
    mainwindow.h \
    ../PokemonInfo/pokemonstructs.h \
    ../Utilities/otherwidgets.h \
    network.h \
    client.h \
    analyze.h \
    serverchoice.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    battlewindow.h \
    ../Utilities/functions.h \
    controlpanel.h \
    basebattlewindow.h \
    ../PokemonInfo/movesetchecker.h \
    ../Shared/config.h \
    ranking.h \
    pluginmanager.h \
    plugininterface.h \
    centralwidget.h \
    channel.h \
    tierstruct.h \
    theme.h \
    rearrangewindow.h \
    ../Shared/networkcommands.h \
    poketablemodel.h \
    modelenum.h \
    pokemovesmodel.h \
    Teambuilder/pokedex.h \
    Teambuilder/box.h \
    Teambuilder/trainerbody.h \
    Teambuilder/avatarbox.h \
    Teambuilder/pokeballed.h \
    Teambuilder/teambody.h \
    Teambuilder/pokebody.h \
    Teambuilder/pokechoice.h \
    Teambuilder/evmanager.h \
    Teambuilder/advanced.h \
    Teambuilder/dockinterface.h \
    Teambuilder/pokebodywidget.h \
    engineinterface.h \
    logmanager.h \
    poketextedit.h\
    password_wallet.h\
    ../BattleManager/battlescene.h \
    ../BattleManager/battleinput.h \
    ../BattleManager/battledatatypes.h \
    ../BattleManager/battledata.h \
    ../BattleManager/battleclientlog.h \
    basebattlewindowinterface.h \
    themeaccessor.h \
    ../Utilities/coreclasses.h \
    teamholder.h \
    trainermenu.h \
    pokebuttonsholder.h \
    teamholderinterface.h \
    pokebutton.h \
    teambuilder.h \
    Teambuilder/teambuilder_old.h \
    teammenu.h \
    teambuilderwidget.h \
    pokeedit.h \
    evbox.h \
    pokelevelsettings.h \
    soundconfigwindow.h \
    replayviewer.h \
    password_wallet.h \
    pmsystem.h \
    ivbox.h \
    teamimporter.h \
    challengedialog.h \
    tierratingbutton.h \
    findbattledialog.h \
    teamline.h
LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities \
    -lbattlelib
QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
FORMS += controlpanel.ui \
    trainermenu.ui \
    pokebuttonsholder.ui \
    pokebutton.ui \
    pokeedit.ui \
    evbox.ui \
    pokelevelsettings.ui \
    ivbox.ui \
    challengedialog.ui \
    tierratingbutton.ui \
    findbattledialog.ui \
    teamline.ui
TRANSLATIONS = translation_cz.ts \
    translation_de.ts \
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
    translation_th.ts \
    translation_sv.ts \
    translation_zh-cn.ts \
    translation_tr.ts
RC_FILE = myapp.rc
RESOURCES += 


macx {
   LIBS += -framework CoreFoundation
   ICON = pokemononline.icns
   QMAKE_INFO_PLIST = Info.plist
   QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
   QMAKE_POST_LINK = macdeployqt $${DESTDIR}/$${TARGET}.app
}


CONFIG(debian) {
    DEFINES += -DPO_DATA_REPO="/usr/shared/games/pokemon-online/"
}


CONFIG(popmsyoustartonly):DEFINES += PO_PMS_YOU_START_ONLY

include(../Shared/Common.pri)

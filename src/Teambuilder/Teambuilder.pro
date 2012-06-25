QT += network \
    xml \
    phonon \
    declarative \
    opengl

TARGET = Pokemon-Online

DESTDIR = ../../bin

TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    network.cpp \
    client.cpp \
    analyze.cpp \
    battlewindow.cpp \
    controlpanel.cpp \
    basebattlewindow.cpp \
    ranking.cpp \
    pluginmanager.cpp \
    channel.cpp \
    tierstruct.cpp \
    theme.cpp \
    rearrangewindow.cpp \
    logmanager.cpp \
    poketextedit.cpp \
    spectatorwindow.cpp \
    replayviewer.cpp \
    soundconfigwindow.cpp \
    password_wallet.cpp \
    pmsystem.cpp \
    challengedialog.cpp \
    tierratingbutton.cpp \
    findbattledialog.cpp \
    menu.cpp \
    serverchoice.cpp \
    Teambuilder/poketablemodel.cpp \
    Teambuilder/pokemovesmodel.cpp \
    Teambuilder/trainermenu.cpp \
    Teambuilder/pokebuttonsholder.cpp \
    Teambuilder/pokebutton.cpp \
    Teambuilder/teamholder.cpp \
    Teambuilder/teambuilder.cpp \
    Teambuilder/teammenu.cpp \
    Teambuilder/pokeedit.cpp \
    Teambuilder/evbox.cpp \
    Teambuilder/pokelevelsettings.cpp \
    Teambuilder/teamline.cpp \
    Teambuilder/pokeselection.cpp \
    Teambuilder/ivbox.cpp \
    Teambuilder/teamimporter.cpp \
    Teambuilder/pokechoice.cpp \
    Teambuilder/pokeboxes.cpp

HEADERS += ../PokemonInfo/pokemonstructs.h \
    ../PokemonInfo/battlestructs.h \
    ../PokemonInfo/networkstructs.h \
    ../PokemonInfo/pokemoninfo.h \
    ../PokemonInfo/movesetchecker.h \
    ../BattleManager/battlescene.h \
    ../BattleManager/battleinput.h \
    ../BattleManager/battledatatypes.h \
    ../BattleManager/battledata.h \
    ../BattleManager/battleclientlog.h \
    ../Utilities/functions.h \
    ../Utilities/coreclasses.h \
    ../Utilities/otherwidgets.h \
    ../Shared/config.h \
    ../Shared/networkcommands.h \
    mainwindow.h \
    network.h \
    client.h \
    analyze.h \
    battlewindow.h \
    controlpanel.h \
    basebattlewindow.h \
    ranking.h \
    pluginmanager.h \
    plugininterface.h \
    centralwidget.h \
    channel.h \
    tierstruct.h \
    theme.h \
    rearrangewindow.h \
    engineinterface.h \
    logmanager.h \
    poketextedit.h\
    password_wallet.h\
    basebattlewindowinterface.h \
    themeaccessor.h \
    soundconfigwindow.h \
    replayviewer.h \
    password_wallet.h \
    pmsystem.h \
    challengedialog.h \
    tierratingbutton.h \
    findbattledialog.h \
    tieractionfactory.h \
    menu.h \
    Teambuilder/pokeselection.h \
    Teambuilder/pokechoice.h \
    Teambuilder/poketablemodel.h \
    Teambuilder/modelenum.h \
    Teambuilder/pokemovesmodel.h \
    Teambuilder/teamholder.h \
    Teambuilder/trainermenu.h \
    Teambuilder/pokebuttonsholder.h \
    Teambuilder/teamholderinterface.h \
    Teambuilder/pokebutton.h \
    Teambuilder/teambuilder.h \
    Teambuilder/teammenu.h \
    Teambuilder/teambuilderwidget.h \
    Teambuilder/pokeedit.h \
    Teambuilder/evbox.h \
    Teambuilder/pokelevelsettings.h \
    Teambuilder/ivbox.h \
    Teambuilder/teamimporter.h \
    Teambuilder/teamline.h \
    serverchoice.h \
    Teambuilder/pokeboxes.h \
    clientinterface.h

LIBS += -L../../bin \
    -lpokemonlib \
    -lutilities \
    -lbattlelib

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

FORMS += Teambuilder/trainermenu.ui \
    Teambuilder/pokebuttonsholder.ui \
    Teambuilder/pokebutton.ui \
    Teambuilder/pokeedit.ui \
    Teambuilder/evbox.ui \
    Teambuilder/pokelevelsettings.ui \
    Teambuilder/ivbox.ui \
    Teambuilder/teamline.ui \
    Teambuilder/pokeselection.ui \
    controlpanel.ui \
    challengedialog.ui \
    tierratingbutton.ui \
    findbattledialog.ui \
    menu.ui \
    serverchoice.ui \
    Teambuilder/pokeboxes.ui

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
   LINKLIBS = libutilities.1.0.0.dylib libpokemonlib.1.0.0.dylib libbattlelib.1.0.0.dylib
   QMAKE_POST_LINK = mkdir -p $${DESTDIR}/$${TARGET}.app/Contents/Frameworks;
   for(L, LINKLIBS) {
       QMAKE_POST_LINK += cp -f $${DESTDIR}/$${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/;
       QMAKE_POST_LINK += ln -s $${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/$$replace(L, 1.0.0, 1);
   }
   QMAKE_POST_LINK += macdeployqt $${DESTDIR}/$${TARGET}.app
}

CONFIG(debian) {
    DEFINES += -DPO_DATA_REPO="/usr/shared/games/pokemon-online/"
}


CONFIG(popmsyoustartonly):DEFINES += PO_PMS_YOU_START_ONLY

include(../Shared/Common.pri)

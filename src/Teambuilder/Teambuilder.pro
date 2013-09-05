QT += network \
    xml \
    declarative \
    opengl \
    script

TARGET = Pokemon-Online

DESTDIR = $$PWD/../../bin

TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    network.cpp \
    client.cpp \
    clientsetupscripts.cpp \
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
    Teambuilder/pokeboxes.cpp \
    Teambuilder/basestatswidget.cpp \
    Teambuilder/advancedsearch.cpp \
    Teambuilder/pokebox.cpp \
    Teambuilder/pokeboxitem.cpp \
    serverchoicemodel.cpp \
    mainwidget.cpp \
    loadwindow.cpp \
    loadline.cpp \
    downloadmanager.cpp \
    Teambuilder/teambuilderwidget.cpp

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
    clientinterface.h \
    Teambuilder/basestatswidget.h \
    Teambuilder/advancedsearch.h \
    Teambuilder/pokebox.h \
    Teambuilder/pokeboxitem.h \
    serverchoicemodel.h \
    mainwidget.h \
    loadwindow.h \
    loadline.h \
    spectatorwindow.h \
    downloadmanager.h \
    teambuilderinterface.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets multimedia concurrent
  QMAKE_CXXFLAGS += "-std=c++11"
} else {
  QT += phonon
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}


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
    Teambuilder/pokeboxes.ui \
    Teambuilder/basestatswidget.ui \
    Teambuilder/advancedsearch.ui \
    mainwidget.ui \
    loadwindow.ui \
    loadline.ui

RC_FILE = myapp.rc

RESOURCES += 

macx {
   LIBS += -framework CoreFoundation
   ICON = pokemononline.icns
   QMAKE_INFO_PLIST = Info.plist
   
   HEADERS += mac/SparkleAutoUpdater.h \
              mac/FullScreenSupport.h \
              mac/CocoaInitializer.h
   OBJECTIVE_SOURCES += mac/SparkleAutoUpdater.mm \
                        mac/FullScreenSupport.mm \
                        mac/CocoaInitializer.mm
   LIBS += -framework Sparkle -framework AppKit

   QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
   LINKLIBS = libpo-utilities.1.0.0.dylib libpo-pokemoninfo.1.0.0.dylib libpo-battlemanager.1.0.0.dylib
   QMAKE_POST_LINK = mkdir -p $${DESTDIR}/$${TARGET}.app/Contents/Frameworks;
   for(L, LINKLIBS) {
       QMAKE_POST_LINK += cp -f $${DESTDIR}/$${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/;
       QMAKE_POST_LINK += ln -s $${L} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/$$replace(L, 1.0.0, 1);
   }
   QMAKE_POST_LINK += macdeployqt $${DESTDIR}/$${TARGET}.app
}

CONFIG(debian_package) {
    DEFINES += PO_DATA_REPO=\\\"/usr/share/games/pokemon-online/\\\"
}
unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}


CONFIG(popmsyoustartonly):DEFINES += PO_PMS_YOU_START_ONLY

include(../Shared/Common.pri)

LIBS += $$battlemanager

windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

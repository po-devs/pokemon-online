QT += network \
    xml \
    declarative \
    opengl \
    script

TARGET = Pokemon-Online
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
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
    rearrangewindow.cpp \
    logmanager.cpp \
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
    Teambuilder/trainermenu.cpp \
    Teambuilder/pokebuttonsholder.cpp \
    Teambuilder/pokebutton.cpp \
    Teambuilder/teambuilder.cpp \
    Teambuilder/teammenu.cpp \
    Teambuilder/pokeedit.cpp \
    Teambuilder/evbox.cpp \
    Teambuilder/pokelevelsettings.cpp \
    Teambuilder/teamline.cpp \
    Teambuilder/ivbox.cpp \
    Teambuilder/teamimporter.cpp \
    Teambuilder/pokeboxes.cpp \
    Teambuilder/pokebox.cpp \
    Teambuilder/pokeboxitem.cpp \
    serverchoicemodel.cpp \
    mainwidget.cpp \
    loadwindow.cpp \
    loadline.cpp \
    downloadmanager.cpp \
    Teambuilder/teambuilderwidget.cpp \
    Teambuilder/pokedex.cpp \
    Teambuilder/pokedexpokeselection.cpp \
    Teambuilder/avatardialog.cpp \
    damagecalc.cpp

HEADERS += mainwindow.h \
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
    rearrangewindow.h \
    engineinterface.h \
    logmanager.h \
    password_wallet.h\
    basebattlewindowinterface.h \
    soundconfigwindow.h \
    replayviewer.h \
    password_wallet.h \
    pmsystem.h \
    challengedialog.h \
    tierratingbutton.h \
    findbattledialog.h \
    tieractionfactory.h \
    menu.h \
    Teambuilder/trainermenu.h \
    Teambuilder/pokebuttonsholder.h \
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
    Teambuilder/pokebox.h \
    Teambuilder/pokeboxitem.h \
    serverchoicemodel.h \
    mainwidget.h \
    loadwindow.h \
    loadline.h \
    spectatorwindow.h \
    downloadmanager.h \
    teambuilderinterface.h \
    Teambuilder/pokedex.h \
    Teambuilder/pokedexpokeselection.h \
    ../Shared/config.h \
    Teambuilder/avatardialog.h \
    damagecalc.h

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets multimedia concurrent
  CONFIG += c++11
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
    controlpanel.ui \
    challengedialog.ui \
    tierratingbutton.ui \
    findbattledialog.ui \
    menu.ui \
    serverchoice.ui \
    Teambuilder/pokeboxes.ui \
    mainwidget.ui \
    loadwindow.ui \
    loadline.ui \
    Teambuilder/pokedex.ui \
    Teambuilder/pokedexpokeselection.ui \
    Teambuilder/avatardialog.ui \
    damagecalc.ui

RC_FILE = myapp.rc

RESOURCES += 

include(../Shared/Common.pri)

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


CONFIG(popmsyoustartonly):DEFINES += PO_PMS_YOU_START_ONLY

LIBS += $$teambuilder

windows: { LIBS += -lzip-2 }
!windows: { LIBS += -lzip }

OTHER_FILES += \
    ../../bin/Themes/Classic/default.css \
    "../../bin/Themes/Dark Classic/default.css" \
    ../../bin/Themes/Shoddy/default.css

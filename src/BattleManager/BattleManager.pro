#-------------------------------------------------
#
# Project created by QtCreator 2011-08-29T23:44:58
#
#-------------------------------------------------

QT += declarative opengl
TARGET = battlelib
TEMPLATE = lib
DESTDIR = ../../bin
LIBS += -L../../bin \
    -lutilities \
    -lpokemonlib

DEFINES += BATTLEMANAGER_LIBRARY

SOURCES += \
    main.cpp \
    testing.cpp \
    teamdata.cpp \
    battleinput.cpp \
    battleclientlog.cpp \
    auxpokebattledata.cpp \
    battlescene.cpp \
    battledataaccessor.cpp \
    battlesceneproxy.cpp \
    pokemoninfoaccessor.cpp \
    auxpokedataproxy.cpp \
    proxydatacontainer.cpp \
    datacontainer.cpp \
    regularbattlescene.cpp \
    remove_troll_characters.cpp

HEADERS +=\
    command.h \
    commandmanager.h \
    commandextracter.h \
    commandflow.h \
    commandinvoke.h \
    battleenum.h \
    battlecommandmanager.h \
    battleextracter.h \
    battlecommandinvoker.h \
    test.h \
    battledata.h \
    teamdata.h \
    battleinput.h \
    battleclientlog.h \
    auxpokebattledata.h \
    defaulttheme.h \
    battlescene.h \
    battledataaccessor.h \
    battlesceneproxy.h \
    pokemoninfoaccessor.h \
    datacontainer.h \
    proxydatacontainer.h \
    battledatatypes.h \
    auxpokedataproxy.h \
    advancedbattledata.h \
    battlesceneflow.h \
    param.h \
    regularbattlescene.h \
    ../Shared/battlecommands.h \
    remove_troll_characters.h

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEEF4D708
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = BattleManager.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    battlescene.qml \
    BattleDataQML/Pokemon.qml \
    BattleDataQML/Team.qml \
    ../../bin/qml/battlescene.qml \
    ../../bin/qml/BattleDataQML/qmldir \
    ../../bin/qml/BattleDataQML/Team.qml \
    ../../bin/qml/BattleDataQML/Pokemon.qml \
    ../../bin/qml/BattleDataQML/FieldPokemon.qml \
    ../../bin/qml/BattleDataQML/ProgressBar.qml \
    ../../bin/qml/BattleDataQML/FrameAnimation.qml \
    ../../bin/qml/BattleDataQML/PokeballAnimation.qml \
    ../../bin/qml/BattleDataQML/ColorShader.qml \
    ../../bin/qml/BattleDataQML/colors.js \
    ../../bin/qml/BattleDataQML/weather.js \
    ../../bin/qml/BattleDataQML/Weather/Rain.qml \
    ../../bin/qml/BattleDataQML/Weather/Sand.qml \
    ../../bin/qml/BattleDataQML/Weather/Hail.qml \
    ../../bin/qml/BattleDataQML/Weather/qmldir \
    ../../bin/qml/BattleDataQML/Weather/Sun.qml \
    ../../bin/qml/initial.qml \
    ../../bin/qml/BattleDataQML/CommonEffects/StatUp.qml \
    ../../bin/qml/BattleDataQML/CommonEffects/StatDown.qml \
    ../../bin/qml/BattleDataQML/effects.js \
    ../../bin/qml/BattleDataQML/moves.js \
    ../../bin/qml/BattleDataQML/utilities.js \
    ../../bin/qml/BattleDataQML/Utilities/MovingGif.qml \
    ../../bin/qml/BattleDataQML/spawner.js \
    ../../bin/qml/BattleDataQML/Moves/Move.qml \
    ../../bin/qml/BattleDataQML/Utilities/Curve.qml \
    ../../bin/qml/BattleDataQML/Moves/UTurn.qml \
    ../../bin/qml/BattleDataQML/Moves/TakeDown.qml \
    ../../bin/qml/BattleDataQML/Moves/RapidSpin.qml \
    ../../bin/qml/BattleDataQML/Moves/QuickAttack.qml \
    ../../bin/qml/BattleDataQML/Moves/HiddenPebbles.qml \
    ../../bin/qml/BattleDataQML/Moves/Earthquake.qml \
    ../../bin/qml/BattleDataQML/Moves/Substitute.qml \
    ../../bin/qml/BattleDataQML/Moves/Bonemerang.qml \
    ../../bin/qml/BattleDataQML/Moves/BoneRush.qml

include(../Shared/Common.pri)









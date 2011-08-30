#-------------------------------------------------
#
# Project created by QtCreator 2011-08-29T23:44:58
#
#-------------------------------------------------

TARGET = BattleManager
TEMPLATE = lib

DEFINES += BATTLEMANAGER_LIBRARY

SOURCES += battlemanager.cpp

HEADERS += battlemanager.h\
        BattleManager_global.h \
    battlepokemon.h \
    abstractbattlepokemon.h \
    battleteam.h \
    battledata.h \
    command.h \
    commandmanager.h \
    commandextracter.h

QMAKE_CXXFLAGS += "-std=c++0x"

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

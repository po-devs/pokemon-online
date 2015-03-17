TEMPLATE = app
QT += gui quick

TARGET = Pokemon-Online
CONFIG += c++11

SOURCES =   main.cpp \
            serverchoicemodel.cpp \
            ../Teambuilder/analyze.cpp \
            analyzeraccess.cpp \
    playerinfolistmodel.cpp

HEADERS =   serverchoicemodel.h \
            ../Teambuilder/analyze.h \
            ../libraries/TeambuilderLibrary/poketablemodel.h \
            ../libraries/PokemonInfo/teamholder.h \
            ../libraries/PokemonInfo/pokemoninfo.h \
            ../libraries/PokemonInfo/movesetchecker.h \
            ../libraries/PokemonInfo/battlestructs.h \
            ../libraries/PokemonInfo/networkstructs.h \
            analyzeraccess.h \
    playerinfolistmodel.h

INCLUDEPATH = "../libraries"

RESOURCES += \
    qml.qrc

include(../Shared/Common.pri)

LIBS += $$teambuilder

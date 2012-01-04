TARGET = pokemonlib
TEMPLATE = lib
DESTDIR = ../../bin
QT += xml
SOURCES += pokemonstructs.cpp \
    pokemoninfo.cpp \
    networkstructs.cpp \
    movesetchecker.cpp \
    battlestructs.cpp \
    teamsaver.cpp
HEADERS += pokemonstructs.h \
    pokemoninfo.h \
    networkstructs.h \
    movesetchecker.h \
    battlestructs.h \
    teamsaver.h \
    enums.h
LIBS += -L../../bin \
    -lutilities \
    -lzip
OTHER_FILES += 

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}

QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"

TARGET = po-pokemoninfo
TEMPLATE = lib
QT += xml
SOURCES += pokemonstructs.cpp \
    pokemoninfo.cpp \
    networkstructs.cpp \
    movesetchecker.cpp \
    battlestructs.cpp \
    teamsaver.cpp \
    pokemon.cpp \
    teamholder.cpp
HEADERS += pokemonstructs.h \
    pokemoninfo.h \
    networkstructs.h \
    movesetchecker.h \
    battlestructs.h \
    teamsaver.h \
    enums.h \
    ../Shared/config.h \
    geninfo.h \
    pokemon.h \
    teamholder.h \
    teamholderinterface.h

include(../../Shared/Common.pri)

LIBS += $$utilities

CUSTOM_INCLUDE_PATH =
CUSTOM_LIB_PATH = 
exists ($$LIBZIP_PATH) {
    CUSTOM_LIB_PATH += -L$$LIBZIP_PATH/lib/.libs
    INCLUDEPATH += $$LIBZIP_PATH/lib
}

windows: { LIBS += -L$$bin -lzip-2 }
!windows: { LIBS += $$CUSTOM_LIB_PATH -lzip }

OTHER_FILES += 

contains(EXTRAS, test) {
   dirprefix = tests/
} else {
   dirprefix =
}

INCLUDEPATH += $$PWD/ $$PWD/../ $$PWD/../libraries

# Common shadow build directory for all builds
CONFIG(shadow)|!equals($${_PRO_FILE_PWD_}, $${OUT_PWD}) {
   CONFIG(debug, debug|release) {
      OBJECTS_DIR=$$PWD/../../build/debug/$$dirprefix$$basename(_PRO_FILE_PWD_)
   }
   CONFIG(release, debug|release) {
      OBJECTS_DIR=$$PWD/../../build/release/$$dirprefix$$basename(_PRO_FILE_PWD_)
   }
   message("Shadow build enabled. Obj dir" $$OBJECTS_DIR)
} else {
   message("No shadow build")
}

bin = $$PWD/../../bin

DESTDIR=$$bin
contains(EXTRAS, clientplugin) {
    DESTDIR=$$bin/clientplugins
}
contains(EXTRAS, serverplugin) {
    DESTDIR=$$bin/serverplugins
}
contains(EXTRAS, battleserverplugin) {
    DESTDIR=$$bin/battleserverplugins
}

#adds debug suffix to libraries when compiled
CONFIG(debug, debug|release) {
    exesuffix=_debug
} else {
    exesuffix=
}

TARGET = $$join(TARGET,,,$$exesuffix)
utilities = -L$$bin -lpo-utilities$$exesuffix
pokemoninfo = $$utilities -lpo-pokemoninfo$$exesuffix
battlemanager = $$pokemoninfo -lpo-battlemanager$$exesuffix
teambuilder = $$battlemanager -lpo-teambuilder$$exesuffix
websocket = -lqtwebsocket$$exesuffix
json = -lqjson$$exesuffix
DEFINES += EXE_SUFFIX="$$exesuffix"

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}
